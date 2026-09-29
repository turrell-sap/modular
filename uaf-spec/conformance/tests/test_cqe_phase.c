/* Conformance C-7: DST completion polling via the phase tag. Section 6.3. */
#include "uaf_test.h"
#include "uaf_dst.h"
#include <stdalign.h>

int main(void)
{
    alignas(UAF_CQE_ALIGN) static struct uaf_storage_cqe ring[8];
    struct uaf_cr_state cr;
    struct uaf_storage_wc out[8];

    CASE("init requires a power-of-two depth and an aligned base");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 3u, NULL), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u, NULL), UAF_OK);
    CHECK_EQ_U(cr.expected_phase, 1u);

    CASE("a zeroed ring yields zero completions, not a false UAF_OK");
    /* This is the v2.0 defect: with no phase tag, a zeroed slot is
     * indistinguishable from a completion whose status is UAF_OK (0). */
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 0);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 0);   /* still empty, idempotent */

    CASE("poll returns a count, never an error, when entries are ready");
    uaf_cr_post(&cr, 0u, 0x1234u, UAF_OK, 4096u, 1500u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 1);
    CHECK_EQ_U(out[0].wr_id, 0x1234u);   /* no cid table: wr_id == cmd_id */
    CHECK_EQ_I(out[0].status, UAF_OK);
    CHECK_EQ_U(out[0].bytes_transferred, 4096u);
    CHECK_EQ_U(out[0].latency_ns, 1500u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 0);   /* consumed */

    CASE("max bounds the batch");
    for (uint32_t i = 1; i < 6u; i++)
        uaf_cr_post(&cr, i, (uint16_t)(0x100u + i), UAF_OK, 512u, 10u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 2, out), 2);
    CHECK_EQ_U(out[0].wr_id, 0x101u);
    CHECK_EQ_U(out[1].wr_id, 0x102u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 3);

    CASE("phase flips on wrap so stale entries are not re-reported");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u, NULL), UAF_OK);
    for (uint32_t i = 0; i < 8u; i++)
        uaf_cr_post(&cr, i, (uint16_t)i, UAF_OK, 64u, 1u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 8);
    CHECK_EQ_U(cr.expected_phase, 0u);          /* flipped after the wrap */
    /* The ring still holds phase-1 entries. They MUST NOT be re-reported. */
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 0);
    /* The producer now writes phase 0 for the second lap. */
    uaf_cr_post(&cr, 0u, 0xBEEFu, UAF_ERR_REMOTE, 0u, 77u, 0u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 1);
    CHECK_EQ_U(out[0].wr_id, 0xBEEFu);
    CHECK_EQ_I(out[0].status, UAF_ERR_REMOTE);

    CASE("a failure status survives the ring intact");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u, NULL), UAF_OK);
    uaf_cr_post(&cr, 0u, 1u, UAF_ERR_MR_FAULT, 0u, 0u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 1, out), 1);
    CHECK_EQ_I(out[0].status, UAF_ERR_MR_FAULT);
    CHECK(out[0].status < 0);

    CASE("latency clamps instead of wrapping");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u, NULL), UAF_OK);
    uaf_cr_post(&cr, 0u, 2u, UAF_OK, 4096u, 0xFFFFFFFFu, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 1, out), 1);
    CHECK_EQ_U(out[0].latency_ns, 0xFFFFFFFFu);

    CASE("with a cid table, poll returns the caller's full 64-bit wr_id");
    /* This is the path an application actually sees. v2.1's poll handed back
     * the 16-byte ring image, so a 64-bit wr_id could not survive it. */
    struct uaf_cid_table cids;
    CHECK_EQ_I(uaf_cid_init(&cids, 8u), UAF_OK);
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u, &cids), UAF_OK);
    const uint64_t big_a = 0x0000000000001234ull;
    const uint64_t big_b = 0xAAAABBBB00001234ull;
    uint16_t ca = 0, cb = 0;
    CHECK_EQ_I(uaf_cid_alloc(&cids, big_a, &ca), UAF_OK);
    CHECK_EQ_I(uaf_cid_alloc(&cids, big_b, &cb), UAF_OK);
    CHECK(ca != cb);
    uaf_cr_post(&cr, 0u, ca, UAF_OK, 4096u, 10u, 1u);
    uaf_cr_post(&cr, 1u, cb, UAF_OK, 8192u, 20u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 2);
    CHECK_EQ_U(out[0].wr_id, big_a);
    CHECK_EQ_U(out[1].wr_id, big_b);
    CHECK(out[0].wr_id != out[1].wr_id);   /* v2.0 collided here */
    CHECK_EQ_U(out[1].bytes_transferred, 8192u);

    CASE("a completion naming an unknown cmd_id is a protocol error");
    uaf_cr_post(&cr, 2u, 7777u, UAF_OK, 0u, 0u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), UAF_ERR_PROTO);
    uaf_cid_fini(&cids);

    CASE("bad arguments are rejected");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u, NULL), UAF_OK);
    CHECK_EQ_I(uaf_cr_poll(&cr, -1, out), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_cr_poll(&cr, 1, NULL), UAF_ERR_INVAL);

    TEST_MAIN_END("test_cqe_phase");
}
