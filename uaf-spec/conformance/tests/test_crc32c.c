/* Conformance C-2: CRC32C known answers. Section 5.8. */
#include "uaf_test.h"
#include "uaf_wire.h"

int main(void)
{
    /* RFC 3720 Appendix B.4 */
    CASE("RFC 3720 vectors");
    uint8_t z32[32], f32[32], inc[32];
    for (int i = 0; i < 32; i++) { z32[i] = 0x00; f32[i] = 0xFF; inc[i] = (uint8_t)i; }
    CHECK_EQ_U(UAF_CRC32C(z32, 32), 0x8A9136AAu);
    CHECK_EQ_U(UAF_CRC32C(f32, 32), 0x62A8AB43u);
    CHECK_EQ_U(UAF_CRC32C(inc, 32), 0x46DD794Eu);
    CHECK_EQ_U(UAF_CRC32C("123456789", 9), 0xE3069283u);
    CHECK_EQ_U(UAF_CRC32C("", 0), 0x00000000u);
    CHECK_EQ_U(UAF_CRC32C("a", 1), 0xC1D04330u);

    CASE("UAF golden header vector");
    /* The vector published in Section 5.8 of the specification. */
    struct uaf_wire_hdr h = {0};
    h.opcode    = UAF_WR_RDMA_WRITE;
    h.flags     = UAF_WIRE_FLAG_ACK_REQ | UAF_WIRE_FLAG_DATA_CRC;
    h.qp_id     = 0x00000101u;
    h.psn       = 0x00000001u;
    h.msg_id    = 0x00000007u;
    h.rkey      = 0xDEADBEEFu;
    h.remote_va = 0x0000100000000000ull;
    h.msg_len   = 16u;
    h.seg_off   = 0u;
    h.seg_len   = 16u;
    h.imm_data  = 0u;
    static const uint8_t payload[16] = {
        'U','A','F','-','S','P','E','C','-','0','0','1',' ','v','2','1'
    };
    uint8_t buf[64];
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, payload), UAF_OK);
    printf("  hdr_crc32c  = 0x%08X\n", uaf_get_be32(buf + UAF_H_HDR_CRC));
    printf("  data_crc32c = 0x%08X\n", uaf_get_be32(buf + UAF_H_DATA_CRC));
    printf("  header bytes:\n");
    for (int i = 0; i < 64; i += 16) {
        printf("    %02d:", i);
        for (int j = 0; j < 16; j++) printf(" %02X", buf[i + j]);
        printf("\n");
    }
    /* Self-consistency: the CRC in the buffer must verify. */
    CHECK_EQ_U(UAF_CRC32C(buf, UAF_HDR_CRC_COVER),
               uaf_get_be32(buf + UAF_H_HDR_CRC));
    CHECK_EQ_U(UAF_CRC32C(payload, 16),
               uaf_get_be32(buf + UAF_H_DATA_CRC));
    TEST_MAIN_END("test_crc32c");
}
