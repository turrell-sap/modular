/* Conformance C-8: UAF-to-NVMe mapping. Section 6.2 / 6.5. */
#include "uaf_test.h"
#include "uaf_dst.h"

int main(void)
{
    CASE("opcode map is total and FLUSH does not become a WRITE");
    CHECK_EQ_I(uaf_dst_nvme_opcode(UAF_WR_STORAGE_READ),  UAF_NVME_OPC_READ);
    CHECK_EQ_I(uaf_dst_nvme_opcode(UAF_WR_STORAGE_WRITE), UAF_NVME_OPC_WRITE);
    /* v2.0's two-way ternary sent FLUSH down the WRITE arm. */
    CHECK_EQ_I(uaf_dst_nvme_opcode(UAF_WR_STORAGE_FLUSH), UAF_NVME_OPC_FLUSH);
    CHECK(uaf_dst_nvme_opcode(UAF_WR_STORAGE_FLUSH) != UAF_NVME_OPC_WRITE);

    CASE("non-storage opcodes are rejected");
    CHECK_EQ_I(uaf_dst_nvme_opcode(UAF_WR_RDMA_WRITE), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_dst_nvme_opcode(UAF_WR_SEND),       UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_dst_nvme_opcode(0xFFu),             UAF_ERR_INVAL);

    CASE("nlb is 1-based in the API and 0-based on the wire");
    uint16_t cdw12 = 0xFFFFu;
    CHECK_EQ_I(uaf_dst_nlb_to_cdw12(1u, &cdw12), UAF_OK);
    CHECK_EQ_U(cdw12, 0u);                    /* 1 block -> NLB 0 */
    CHECK_EQ_I(uaf_dst_nlb_to_cdw12(8u, &cdw12), UAF_OK);
    CHECK_EQ_U(cdw12, 7u);
    CHECK_EQ_I(uaf_dst_nlb_to_cdw12(65536u, &cdw12), UAF_OK);
    CHECK_EQ_U(cdw12, 0xFFFFu);               /* the largest legal transfer */

    CASE("nlb == 0 is rejected, not turned into 65536 blocks");
    /* v2.0 computed (nlb - 1) & 0xFFFF with no check, so a zero-length
     * request became a 65536-block, 256 MiB transfer. */
    cdw12 = 0x1234u;
    CHECK_EQ_I(uaf_dst_nlb_to_cdw12(0u, &cdw12), UAF_ERR_INVAL);
    CHECK_EQ_U(cdw12, 0x1234u);               /* output left untouched */
    CHECK_EQ_I(uaf_dst_nlb_to_cdw12(65537u, &cdw12), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_dst_nlb_to_cdw12(0xFFFFFFFFu, &cdw12), UAF_ERR_INVAL);

    CASE("data pointer kind follows the buffer, not wishful thinking");
    enum uaf_dst_dptr k;
    /* v2.0 put every buffer in dptr_prp1 regardless of size. */
    CHECK_EQ_I(uaf_dst_dptr_kind(0x1000u, 4096u, 4096u, 0, &k), UAF_OK);
    CHECK_EQ_I(k, UAF_DPTR_PRP1);
    CHECK_EQ_I(uaf_dst_dptr_kind(0x1800u, 2048u, 4096u, 0, &k), UAF_OK);
    CHECK_EQ_I(k, UAF_DPTR_PRP1);             /* ends exactly at the page end */
    CHECK_EQ_I(uaf_dst_dptr_kind(0x1800u, 4096u, 4096u, 0, &k), UAF_OK);
    CHECK_EQ_I(k, UAF_DPTR_PRP1_PRP2);        /* straddles one boundary */
    CHECK_EQ_I(uaf_dst_dptr_kind(0x1000u, 8192u, 4096u, 0, &k), UAF_OK);
    CHECK_EQ_I(k, UAF_DPTR_PRP1_PRP2);
    CHECK_EQ_I(uaf_dst_dptr_kind(0x1000u, 12288u, 4096u, 0, &k), UAF_OK);
    CHECK_EQ_I(k, UAF_DPTR_PRP_LIST);         /* three pages, no SGL support */
    CHECK_EQ_I(uaf_dst_dptr_kind(0x1000u, 1u << 20, 4096u, 1, &k), UAF_OK);
    CHECK_EQ_I(k, UAF_DPTR_SGL);
    CHECK_EQ_I(uaf_dst_dptr_kind(0x1000u, 0u, 4096u, 0, &k), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_dst_dptr_kind(0x1000u, 4096u, 5000u, 0, &k), UAF_ERR_INVAL);

    CASE("cmd_id is allocated, and the full 64-bit wr_id survives");
    struct uaf_cid_table t;
    CHECK_EQ_I(uaf_cid_init(&t, 256u), UAF_OK);
    /* Two wr_ids that differ only above bit 16 collided in v2.0, which used
     * (wr_id & 0xFFFF) as the command id. */
    const uint64_t a = 0x0000000000001234ull;
    const uint64_t b = 0xAAAABBBB00001234ull;
    uint16_t ca = 0, cb = 0;
    CHECK_EQ_I(uaf_cid_alloc(&t, a, &ca), UAF_OK);
    CHECK_EQ_I(uaf_cid_alloc(&t, b, &cb), UAF_OK);
    CHECK(ca != cb);
    uint64_t got = 0;
    CHECK_EQ_I(uaf_cid_release(&t, cb, &got), UAF_OK);
    CHECK_EQ_U(got, b);
    CHECK_EQ_I(uaf_cid_release(&t, ca, &got), UAF_OK);
    CHECK_EQ_U(got, a);

    CASE("releasing an unknown or already-free cmd_id is a protocol error");
    CHECK_EQ_I(uaf_cid_release(&t, ca, &got), UAF_ERR_PROTO);
    CHECK_EQ_I(uaf_cid_release(&t, 60000u, &got), UAF_ERR_PROTO);

    CASE("NVMe completion status maps onto uaf_error");
    /* v2.1 mapped opcodes and NLB and left completion status unmapped, so a
     * DST implementation could submit a command and had no defined way to
     * report why it failed. */
    CHECK_EQ_I(uaf_dst_status_to_uaf(UAF_NVME_SCT_GENERIC, 0x00), UAF_OK);
    CHECK_EQ_I(uaf_dst_status_to_uaf(UAF_NVME_SCT_GENERIC, 0x02), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_dst_status_to_uaf(UAF_NVME_SCT_GENERIC, 0x80),
               UAF_ERR_MR_FAULT);                       /* LBA out of range */
    CHECK_EQ_I(uaf_dst_status_to_uaf(UAF_NVME_SCT_GENERIC, 0x0B),
               UAF_ERR_MR_FAULT);                       /* invalid PRP offset */
    CHECK_EQ_I(uaf_dst_status_to_uaf(UAF_NVME_SCT_MEDIA, 0x82), UAF_ERR_CRC);
    CHECK_EQ_I(uaf_dst_status_to_uaf(UAF_NVME_SCT_MEDIA, 0x86), UAF_ERR_PERM);
    CHECK_EQ_I(uaf_dst_status_to_uaf(UAF_NVME_SCT_PATH, 0x01), UAF_ERR_TIMEOUT);
    /* Every unmapped code must still be a failure, never a silent success. */
    for (unsigned sct = 0; sct < 8u; sct++)
        for (unsigned sc = 1; sc < 256u; sc++)
            CHECK(uaf_dst_status_to_uaf((uint8_t)sct, (uint8_t)sc) < 0);

    CASE("the table reports BUSY instead of reusing a live cmd_id");
    for (uint32_t i = 0; i < 256u; i++) {
        uint16_t c;
        CHECK_EQ_I(uaf_cid_alloc(&t, 0x5000ull + i, &c), UAF_OK);
    }
    uint16_t c_over;
    CHECK_EQ_I(uaf_cid_alloc(&t, 1u, &c_over), UAF_ERR_BUSY);
    uaf_cid_fini(&t);

    TEST_MAIN_END("test_dst_map");
}
