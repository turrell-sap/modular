/* Conformance C-10: an end-to-end RDMA WRITE over the Profile B UDP path.
 *
 * Two sockets on the loopback interface, the real 64-byte header, real
 * segmentation, real CRC checks, real rkey validation, and one completion on
 * the last segment. This is the test v2.0 could not have had: it had no
 * framing, no phase in its completion, no defined key check and no CM.
 */
#include "uaf_test.h"
#include "uaf_wire.h"
#include "uaf_rkey.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define MSG_BYTES   200000u
#define TEST_MTU    1500u          /* deliberately not the 4096 default */

struct responder {
    uint8_t              *target;    /* the registered region        */
    uint64_t              base;
    uint64_t              len;
    struct uaf_key_table  keys;
    uint32_t              qp_num;
    uint32_t              expect_psn;
    uint32_t              received;  /* bytes accepted for this message */
    int                   completions;
    int                   dropped_crc;
    int                   dropped_proto;
    int                   rkey_failures;
};

/* Responder side of one packet. Returns 0 if accepted, negative if dropped. */
static int rx_packet(struct responder *R, const uint8_t *pkt, size_t n)
{
    struct uaf_wire_hdr h;
    if (n < UAF_WIRE_HDR_SIZE) { R->dropped_proto++; return UAF_ERR_PROTO; }

    int rc = uaf_wire_hdr_decode(pkt, &h);
    if (rc == UAF_ERR_CRC)   { R->dropped_crc++;   return rc; }
    if (rc != UAF_OK)        { R->dropped_proto++; return rc; }
    if (n < (size_t)UAF_WIRE_HDR_SIZE + h.seg_len) {
        R->dropped_proto++; return UAF_ERR_PROTO;
    }
    const uint8_t *payload = pkt + UAF_WIRE_HDR_SIZE;
    if ((rc = uaf_wire_check_payload(&h, payload)) != UAF_OK) {
        R->dropped_crc++; return rc;
    }
    /* [R-5.6-003] out-of-order under Go-Back-N is dropped, and the expected
     * PSN is what a NAK would carry. */
    if (h.psn != R->expect_psn) { R->dropped_proto++; return UAF_ERR_PROTO; }

    uint64_t va = h.remote_va + h.seg_off;
    rc = uaf_key_validate(&R->keys, h.rkey, R->qp_num, va, h.seg_len,
                          UAF_MR_REMOTE_WRITE, 1 /* authenticated */);
    if (rc != UAF_OK) { R->rkey_failures++; return rc; }

    memcpy(R->target + (va - R->base), payload, h.seg_len);
    R->received += h.seg_len;
    R->expect_psn++;

    /* [R-5.3-003] exactly one completion, on the last segment */
    if (UAF_WIRE_IS_LAST(&h)) {
        if (R->received != h.msg_len) return UAF_ERR_PROTO;
        R->completions++;
        R->received = 0u;
    }
    return 0;
}

int main(void)
{
    int tx = socket(AF_INET, SOCK_DGRAM, 0);
    int rx = socket(AF_INET, SOCK_DGRAM, 0);
    if (tx < 0 || rx < 0) { printf("SKIP test_udp_loopback: no socket\n"); return 0; }

    struct sockaddr_in a; memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = 0;
    if (bind(rx, (struct sockaddr *)&a, sizeof(a)) != 0) {
        printf("SKIP test_udp_loopback: bind: %s\n", strerror(errno));
        return 0;
    }
    socklen_t al = sizeof(a);
    getsockname(rx, (struct sockaddr *)&a, &al);

    /* Grow the receive buffer so loopback does not drop our burst. */
    int rcvbuf = 8 * 1024 * 1024;
    setsockopt(rx, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    static uint8_t src[MSG_BYTES];
    static uint8_t dst[MSG_BYTES];
    for (uint32_t i = 0; i < MSG_BYTES; i++) src[i] = (uint8_t)(i * 31u + 7u);
    memset(dst, 0, sizeof(dst));

    struct responder R;
    memset(&R, 0, sizeof(R));
    R.target = dst;
    R.base   = 0x40000000ull;
    R.len    = MSG_BYTES;
    R.qp_num = 0x101u;
    R.expect_psn = 1u;
    CHECK_EQ_I(uaf_key_table_init(&R.keys, 8), UAF_OK);

    uint32_t lkey = 0, rkey = 0;
    CHECK_EQ_I(uaf_key_register(&R.keys, R.base, R.len, UAF_MR_REMOTE_WRITE,
                               0u /* unbound at registration */, &lkey, &rkey),
               UAF_OK);
    (void)lkey;
    /* [R-11.2-003] the queue-pair binding is installed at connect time. */
    CHECK_EQ_I(uaf_key_bind_qp(&R.keys, rkey, R.qp_num), UAF_OK);

    CASE("segment, send, receive, reassemble");
    uint32_t nseg = uaf_wire_seg_count(MSG_BYTES, TEST_MTU);
    CHECK(nseg > 100u);          /* a genuinely multi-packet message */
    uint8_t pkt[TEST_MTU];
    uint32_t psn = 1u, sent = 0u;

    for (uint32_t i = 0; i < nseg; i++) {
        struct uaf_wire_hdr h; memset(&h, 0, sizeof(h));
        h.opcode    = UAF_WR_RDMA_WRITE;
        h.flags     = UAF_WIRE_FLAG_DATA_CRC | UAF_WIRE_FLAG_ACK_REQ;
        h.qp_id     = R.qp_num;
        h.psn       = psn++;
        h.msg_id    = 9u;
        h.rkey      = rkey;
        h.remote_va = R.base;
        CHECK_EQ_I(uaf_wire_segment(&h, MSG_BYTES, TEST_MTU, i), UAF_OK);
        CHECK_EQ_I(uaf_wire_hdr_encode(&h, pkt, src + h.seg_off), UAF_OK);
        memcpy(pkt + UAF_WIRE_HDR_SIZE, src + h.seg_off, h.seg_len);
        size_t plen = UAF_WIRE_HDR_SIZE + h.seg_len;
        CHECK(plen <= TEST_MTU);

        ssize_t s = sendto(tx, pkt, plen, 0, (struct sockaddr *)&a, sizeof(a));
        CHECK_EQ_I(s, (ssize_t)plen);
        sent++;

        /* Drain synchronously: this is a correctness test, not a rate test. */
        uint8_t in[TEST_MTU];
        ssize_t got = recv(rx, in, sizeof(in), 0);
        CHECK(got > 0);
        CHECK_EQ_I(rx_packet(&R, in, (size_t)got), 0);
    }

    CHECK_EQ_U(sent, nseg);
    CHECK_EQ_I(R.completions, 1);            /* exactly one, on LAST */
    CHECK_EQ_I(R.dropped_crc, 0);
    CHECK_EQ_I(R.dropped_proto, 0);
    CHECK_EQ_I(R.rkey_failures, 0);
    CASE("the written region matches byte for byte");
    CHECK_EQ_I(memcmp(src, dst, MSG_BYTES), 0);

    CASE("a corrupted payload is dropped, not written");
    struct uaf_wire_hdr h; memset(&h, 0, sizeof(h));
    h.opcode = UAF_WR_RDMA_WRITE; h.flags = UAF_WIRE_FLAG_DATA_CRC;
    h.qp_id = R.qp_num; h.psn = R.expect_psn; h.msg_id = 10u;
    h.rkey = rkey; h.remote_va = R.base;
    CHECK_EQ_I(uaf_wire_segment(&h, 64u, TEST_MTU, 0), UAF_OK);
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, pkt, src), UAF_OK);
    memcpy(pkt + UAF_WIRE_HDR_SIZE, src, 64u);
    pkt[UAF_WIRE_HDR_SIZE + 5] ^= 0x01u;         /* flip a payload bit */
    uint8_t save = dst[5];
    CHECK_EQ_I(rx_packet(&R, pkt, UAF_WIRE_HDR_SIZE + 64u), UAF_ERR_CRC);
    CHECK_EQ_I(R.dropped_crc, 1);
    CHECK_EQ_U(dst[5], save);                     /* nothing was written */

    CASE("a wrong rkey is refused");
    memset(&h, 0, sizeof(h));
    h.opcode = UAF_WR_RDMA_WRITE; h.qp_id = R.qp_num; h.psn = R.expect_psn;
    h.msg_id = 11u; h.rkey = rkey ^ 0x1u; h.remote_va = R.base;
    CHECK_EQ_I(uaf_wire_segment(&h, 8u, TEST_MTU, 0), UAF_OK);
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, pkt, NULL), UAF_OK);
    CHECK_EQ_I(rx_packet(&R, pkt, UAF_WIRE_HDR_SIZE + 8u), UAF_ERR_RKEY);
    CHECK_EQ_I(R.rkey_failures, 1);

    CASE("a write past the end of the region is refused");
    memset(&h, 0, sizeof(h));
    h.opcode = UAF_WR_RDMA_WRITE; h.qp_id = R.qp_num; h.psn = R.expect_psn;
    h.msg_id = 12u; h.rkey = rkey;
    h.remote_va = R.base + R.len - 4u;        /* 8 bytes would overrun by 4 */
    CHECK_EQ_I(uaf_wire_segment(&h, 8u, TEST_MTU, 0), UAF_OK);
    CHECK_EQ_I(uaf_wire_hdr_encode(&h, pkt, NULL), UAF_OK);
    CHECK_EQ_I(rx_packet(&R, pkt, UAF_WIRE_HDR_SIZE + 8u), UAF_ERR_MR_FAULT);

    uaf_key_table_fini(&R.keys);
    close(tx); close(rx);
    TEST_MAIN_END("test_udp_loopback");
}
