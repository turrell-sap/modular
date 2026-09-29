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

    CASE("the MAC input is defined for a 32-byte body, not only for 96");
    /* v2.1.1 always hashed header[0..47] || record(64) || nonce || timestamp,
     * and CM_RTU and CM_REJ carry no record. */
    uint8_t hdr[64], rec[UAF_CM_RECORD_SIZE], nonce[8], ts[8];
    uint8_t mac_in[UAF_CM_MAC_INPUT_MAX];
    memset(hdr, 0, sizeof(hdr));
    for (unsigned i = 0; i < sizeof(rec); i++)   rec[i]   = (uint8_t)(i + 1u);
    for (unsigned i = 0; i < sizeof(nonce); i++) nonce[i] = (uint8_t)(0xA0 + i);
    for (unsigned i = 0; i < sizeof(ts); i++)    ts[i]    = (uint8_t)(0xB0 + i);
    uaf_put_be32(hdr + UAF_H_QP_ID, 0x0DEFACEDu);

    size_t n_req = uaf_cm_mac_input(UAF_OP_CM_REQ, hdr, rec, nonce, ts, mac_in);
    CHECK_EQ_U(n_req, 48u + 64u + 16u);          /* 128 */
    CHECK_EQ_U(mac_in[48], 1u);                  /* record starts at 48 */
    CHECK_EQ_U(mac_in[48 + 64], 0xA0u);          /* then the nonce */
    CHECK_EQ_U(mac_in[48 + 64 + 8], 0xB0u);      /* then the timestamp */

    size_t n_rtu = uaf_cm_mac_input(UAF_OP_CM_RTU, hdr, NULL, nonce, ts, mac_in);
    CHECK_EQ_U(n_rtu, 48u + 16u);                /* 64: no record */
    CHECK_EQ_U(mac_in[48], 0xA0u);               /* the nonce follows the header */
    CHECK_EQ_U(mac_in[56], 0xB0u);

    size_t n_rej = uaf_cm_mac_input(UAF_OP_CM_REJ, hdr, NULL, nonce, ts, mac_in);
    CHECK_EQ_U(n_rej, 48u + 16u);
    CHECK_EQ_U(uaf_cm_mac_input(UAF_WR_RDMA_WRITE, hdr, rec, nonce, ts, mac_in),
               0u);                              /* not a CM opcode */
    /* A REQ without its record cannot be hashed, and must not silently become
     * a short input that a REJ could also produce. */
    CHECK_EQ_U(uaf_cm_mac_input(UAF_OP_CM_REQ, hdr, NULL, nonce, ts, mac_in), 0u);
    CHECK(n_req != n_rtu);

    CASE("replay is keyed on the header qp_id, which every CM packet has");
    /* v2.1.1 keyed replay on (nonce, qp_num) where qp_num lived in the record
     * that RTU and REJ do not carry. */
    CHECK_EQ_U(uaf_cm_replay_qp(hdr), 0x0DEFACEDu);

    CASE("identities built from SOURCE fields stay complementary");
    /* Each side uses its OWN source address and port. Simulating both peers:
     * A sends from 10.0.0.1:40000, B from 10.0.0.2:4791. */
    struct uaf_endpoint_id a_self = eid("10.0.0.1", 40000u, 0x101u);
    struct uaf_endpoint_id b_self = eid("10.0.0.2", 4791u,  0x202u);
    /* A derives B's identity from the source fields of the CM_REQ it received,
     * and B derives A's the same way, so both hold the same pair. */
    int a_view = uaf_cm_is_active(&a_self, &b_self);
    int b_view = uaf_cm_is_active(&b_self, &a_self);
    CHECK_EQ_I(a_view + b_view, 1);
    /* Had each side used its DESTINATION port instead, both would have compared
     * (own addr, PEER port, own qpn) and could agree. Show the tuples differ. */
    struct uaf_endpoint_id a_wrong = eid("10.0.0.1", 4791u,  0x101u);
    struct uaf_endpoint_id b_wrong = eid("10.0.0.2", 40000u, 0x202u);
    CHECK(uaf_eid_compare(&a_self, &a_wrong) != 0);
    CHECK(uaf_eid_compare(&b_self, &b_wrong) != 0);

    CASE("Profile A uses the GID with a zero port");
    struct uaf_endpoint_id g1, g2;
    memset(&g1, 0, sizeof(g1)); memset(&g2, 0, sizeof(g2));
    g1.addr[0] = 0xFE; g1.addr[15] = 0x01; g1.qp_num = 5u;
    g2.addr[0] = 0xFE; g2.addr[15] = 0x02; g2.qp_num = 5u;
    CHECK_EQ_U(g1.udp_port, 0u);
    CHECK_EQ_I(uaf_cm_is_active(&g1, &g2) + uaf_cm_is_active(&g2, &g1), 1);

    TEST_MAIN_END("test_cm");
}
