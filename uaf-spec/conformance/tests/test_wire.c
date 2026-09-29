/* Conformance C-3: wire header encode/decode, framing, error handling. */
#include "uaf_test.h"
#include "uaf_wire.h"

static struct uaf_wire_hdr base_hdr(void)
{
    struct uaf_wire_hdr h = {0};
    h.opcode = UAF_WR_RDMA_WRITE; h.qp_id = 7; h.psn = 42; h.msg_id = 3;
    h.rkey = 0x11223344u; h.remote_va = 0x2000u;
    h.msg_len = 100u; h.seg_off = 0u; h.seg_len = 100u;
    return h;
}

int main(void)
{
    uint8_t buf[64];
    struct uaf_wire_hdr h, g;

    CASE("round trip");
    h = base_hdr();
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_OK);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_OK);
    CHECK_EQ_U(g.opcode, h.opcode);   CHECK_EQ_U(g.qp_id, h.qp_id);
    CHECK_EQ_U(g.psn, h.psn);         CHECK_EQ_U(g.msg_id, h.msg_id);
    CHECK_EQ_U(g.rkey, h.rkey);       CHECK_EQ_U(g.remote_va, h.remote_va);
    CHECK_EQ_U(g.msg_len, h.msg_len); CHECK_EQ_U(g.seg_len, h.seg_len);

    CASE("fields are big-endian on the wire");
    CHECK_EQ_U(buf[0], 0x55); CHECK_EQ_U(buf[1], 0x41);
    CHECK_EQ_U(buf[2], 0x46); CHECK_EQ_U(buf[3], 0x52);   /* "UAFR" */
    CHECK_EQ_U(buf[UAF_H_RKEY + 0], 0x11);
    CHECK_EQ_U(buf[UAF_H_RKEY + 3], 0x44);

    CASE("bad magic is dropped as PROTO, not a CRC or state error");
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_OK);
    buf[0] ^= 0xFFu;
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_ERR_PROTO);

    CASE("bad version");
    uaf_wire_hdr_encode(&h, buf, NULL);
    buf[UAF_H_VERSION] = 99u;
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_ERR_PROTO);

    CASE("corrupted header is UAF_ERR_CRC");
    uaf_wire_hdr_encode(&h, buf, NULL);
    buf[UAF_H_MSG_LEN] ^= 0x01u;
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_ERR_CRC);

    CASE("non-zero reserved bytes are rejected");
    uaf_wire_hdr_encode(&h, buf, NULL);
    buf[UAF_H_RESERVED + 3] = 1u;
    /* the header CRC does not cover the reserved area, so this must be
     * caught by the explicit zero check, not by the CRC */
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_ERR_PROTO);

    CASE("reserved flag bits are rejected");
    h = base_hdr(); h.flags = 0x8000u;
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_ERR_INVAL);

    CASE("UAF_WR_RECV is not a wire opcode");
    h = base_hdr(); h.opcode = UAF_WR_RECV;
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_ERR_INVAL);

    CASE("CM and ACK/NAK opcodes are defined and encodable");
    const uint8_t ops[] = { UAF_OP_CM_REQ, UAF_OP_CM_REP, UAF_OP_CM_RTU,
                            UAF_OP_CM_REJ, UAF_OP_ACK, UAF_OP_NAK,
                            UAF_OP_RDMA_READ_RESP };
    for (unsigned i = 0; i < sizeof(ops); i++) {
        h = base_hdr(); h.opcode = ops[i];
        CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_OK);
        CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_OK);
    }

    CASE("segment that overruns its message is rejected");
    h = base_hdr(); h.msg_len = 10u; h.seg_off = 8u; h.seg_len = 8u;
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_ERR_INVAL);

    CASE("framing: FIRST and LAST are derived");
    uint32_t mtu = 4096u, seg = UAF_WIRE_MAX_SEG(mtu);
    CHECK_EQ_U(uaf_wire_seg_count(0u, mtu), 1u);
    CHECK_EQ_U(uaf_wire_seg_count(1u, mtu), 1u);
    CHECK_EQ_U(uaf_wire_seg_count(seg, mtu), 1u);
    CHECK_EQ_U(uaf_wire_seg_count(seg + 1u, mtu), 2u);
    CHECK_EQ_U(uaf_wire_seg_count(1048576u, mtu), (1048576u + seg - 1u) / seg);

    uint32_t msg = 1000000u, n = uaf_wire_seg_count(msg, mtu), total = 0u;
    for (uint32_t i = 0; i < n; i++) {
        struct uaf_wire_hdr s = base_hdr();
        CHECK_EQ_I(uaf_wire_segment(&s, msg, mtu, i), UAF_OK);
        CHECK(s.seg_len <= seg);
        CHECK_EQ_U(s.seg_off, (uint64_t)i * seg);
        CHECK_EQ_I(UAF_WIRE_IS_FIRST(&s), i == 0u);
        CHECK_EQ_I(UAF_WIRE_IS_LAST(&s), i == n - 1u);
        total += s.seg_len;
        CHECK_EQ_I(uaf_wire_hdr_encode(&s, buf, NULL), UAF_OK);
    }
    CHECK_EQ_U(total, msg);                    /* every byte accounted for */
    CHECK_EQ_I(uaf_wire_segment(&h, msg, mtu, n), UAF_ERR_INVAL);

    CASE("payload CRC detects a flipped data bit");
    uint8_t data[64]; for (int i = 0; i < 64; i++) data[i] = (uint8_t)(i * 7);
    h = base_hdr(); h.msg_len = 64u; h.seg_len = 64u;
    h.flags = UAF_WIRE_FLAG_DATA_CRC;
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, data), UAF_OK);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_OK);
    CHECK_EQ_I(uaf_wire_check_payload(&g, data), UAF_OK);
    data[13] ^= 0x01u;
    CHECK_EQ_I(uaf_wire_check_payload(&g, data), UAF_ERR_CRC);

    CASE("atomic request and response lengths are enforced");
    h = base_hdr(); h.opcode = UAF_WR_ATOMIC_CMP_SWP;
    h.remote_va = 0x2000u; h.msg_len = 16u; h.seg_len = 16u;
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_OK);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_OK);
    h.seg_len = 8u; h.msg_len = 8u;      /* wrong length for CMP_SWP */
    uaf_wire_hdr_encode(&h, buf, NULL);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_ERR_PROTO);

    h = base_hdr(); h.opcode = UAF_WR_ATOMIC_FETCH_ADD;
    h.msg_len = 8u; h.seg_len = 8u; h.remote_va = 0x2000u;
    uaf_wire_hdr_encode(&h, buf, NULL);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_OK);

    h = base_hdr(); h.opcode = UAF_OP_ATOMIC_ACK;
    h.msg_len = 8u; h.seg_len = 8u;
    uaf_wire_hdr_encode(&h, buf, NULL);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_OK);
    CHECK_EQ_U(g.seg_len, UAF_ATOMIC_ACK_LEN);

    CASE("misaligned atomic target is rejected");
    h = base_hdr(); h.opcode = UAF_WR_ATOMIC_FETCH_ADD;
    h.msg_len = 8u; h.seg_len = 8u; h.remote_va = 0x2004u;   /* not 8-aligned */
    uaf_wire_hdr_encode(&h, buf, NULL);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_ERR_INVAL);

    CASE("PSN stays inside 24 bits when bridged to Profile A");
    CHECK_EQ_U(0xFFFFFFFFu & UAF_PSN_MASK_IBV, 0x00FFFFFFu);
    /* PSN wrap is modulo 2^32 in Profile B and modulo 2^24 when bridged. */
    uint32_t psn = 0xFFFFFFFFu;
    CHECK_EQ_U((uint32_t)(psn + 1u), 0u);
    CHECK_EQ_U((0x00FFFFFFu + 1u) & UAF_PSN_MASK_IBV, 0u);

    TEST_MAIN_END("test_wire");
}
