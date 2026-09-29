/* Conformance C-11: connection-manager packet sizing and simultaneous open. */
#include "uaf_test.h"
#include "uaf_wire.h"

static struct uaf_endpoint_id eid(const char *v4, uint16_t port, uint32_t qpn)
{
    struct uaf_endpoint_id e;
    memset(&e, 0, sizeof(e));
    /* IPv4-mapped IPv6: ::ffff:a.b.c.d */
    e.addr[10] = 0xFF; e.addr[11] = 0xFF;
    unsigned a, b, c, d;
    sscanf(v4, "%u.%u.%u.%u", &a, &b, &c, &d);
    e.addr[12] = (uint8_t)a; e.addr[13] = (uint8_t)b;
    e.addr[14] = (uint8_t)c; e.addr[15] = (uint8_t)d;
    e.udp_port = port;
    e.qp_num   = qpn;
    return e;
}

int main(void)
{
    CASE("CM packet lengths, including RTU which carries no record");
    /* v2.1 required seg_len == 96 for every CM packet while its diagram showed
     * CM_RTU as an authenticator only. */
    CHECK_EQ_U(uaf_cm_expected_len(UAF_OP_CM_REQ), 96u);
    CHECK_EQ_U(uaf_cm_expected_len(UAF_OP_CM_REP), 96u);
    CHECK_EQ_U(uaf_cm_expected_len(UAF_OP_CM_RTU), 32u);
    CHECK_EQ_U(uaf_cm_expected_len(UAF_OP_CM_REJ), 32u);
    CHECK_EQ_U(uaf_cm_expected_len(UAF_WR_RDMA_WRITE), 0u);

    CASE("a CM packet whose seg_len disagrees is rejected");
    uint8_t buf[64];
    struct uaf_wire_hdr h, g;
    memset(&h, 0, sizeof(h));
    h.opcode = UAF_OP_CM_RTU; h.qp_id = 1u; h.psn = 1u;
    h.msg_len = 96u; h.seg_len = 96u;             /* wrong for RTU */
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_OK);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_OK);
    CHECK(g.seg_len != uaf_cm_expected_len((uint8_t)g.opcode));
    h.msg_len = 32u; h.seg_len = 32u;             /* correct for RTU */
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, buf, NULL), UAF_OK);
    CHECK_EQ_I(uaf_wire_hdr_decode(buf, &g), UAF_OK);
    CHECK_EQ_U(g.seg_len, uaf_cm_expected_len((uint8_t)g.opcode));

    CASE("endpoint identity serialises to 22 canonical big-endian bytes");
    struct uaf_endpoint_id a = eid("10.0.0.1", 4791u, 0x00000101u);
    uint8_t s[UAF_EID_SIZE];
    uaf_eid_serialize(&a, s);
    CHECK_EQ_U(s[12], 10u); CHECK_EQ_U(s[15], 1u);
    CHECK_EQ_U(s[16], (4791u >> 8) & 0xFFu);
    CHECK_EQ_U(s[17], 4791u & 0xFFu);
    CHECK_EQ_U(s[18], 0u); CHECK_EQ_U(s[21], 0x01u);

    CASE("the tie-break is a strict total order");
    struct uaf_endpoint_id b = eid("10.0.0.2", 4791u, 0x00000101u);
    CHECK(uaf_eid_compare(&a, &b) < 0);
    CHECK(uaf_eid_compare(&b, &a) > 0);
    CHECK_EQ_I(uaf_eid_compare(&a, &a), 0);

    CASE("simultaneous open elects exactly one active side");
    /* v2.1 compared (dest_ip, dest_udp_port, qp_num). Each side's destination
     * is the other side, so the two peers compared different tuples and could
     * both stay active or both go passive. */
    CHECK_EQ_I(uaf_cm_is_active(&a, &b), 1);
    CHECK_EQ_I(uaf_cm_is_active(&b, &a), 0);
    CHECK_EQ_I(uaf_cm_is_active(&a, &b) + uaf_cm_is_active(&b, &a), 1);

    CASE("exactly one winner for every distinguishing field");
    const struct uaf_endpoint_id pairs[][2] = {
        { eid("10.0.0.1", 4791u, 1u), eid("10.0.0.1", 4791u, 2u) },  /* qp_num */
        { eid("10.0.0.1", 4790u, 1u), eid("10.0.0.1", 4791u, 1u) },  /* port   */
        { eid("10.0.0.1", 4791u, 1u), eid("192.168.0.1", 4791u, 1u) },
        { eid("10.0.0.1", 4791u, 9u), eid("10.0.0.2", 4791u, 1u) },  /* addr   */
    };
    for (unsigned i = 0; i < sizeof(pairs)/sizeof(pairs[0]); i++) {
        int one = uaf_cm_is_active(&pairs[i][0], &pairs[i][1]);
        int two = uaf_cm_is_active(&pairs[i][1], &pairs[i][0]);
        CHECK_EQ_I(one + two, 1);          /* never 0, never 2 */
    }

    CASE("the address dominates the port, and the port the queue pair");
    struct uaf_endpoint_id lo_addr_hi_qpn = eid("10.0.0.1", 65535u, 0xFFFFFFFFu);
    struct uaf_endpoint_id hi_addr_lo_qpn = eid("10.0.0.2", 0u, 0u);
    CHECK(uaf_eid_compare(&lo_addr_hi_qpn, &hi_addr_lo_qpn) < 0);

    TEST_MAIN_END("test_cm");
}
