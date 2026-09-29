/* Conformance C-7: DST completion polling via the phase tag. Section 6.3. */
#include "uaf_test.h"
#include "uaf_dst.h"
#include <stdalign.h>

int main(void)
{
    alignas(UAF_CQE_ALIGN) static struct uaf_storage_cqe ring[8];
    struct uaf_cr_state cr;
    struct uaf_storage_cqe out[8];

    CASE("init requires a power-of-two depth and an aligned base");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 3u), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u), UAF_OK);
    CHECK_EQ_U(cr.expected_phase, 1u);

    CASE("a zeroed ring yields zero completions, not a false UAF_OK");
    /* This is the v2.0 defect: with no phase tag, a zeroed slot is
     * indistinguishable from a completion whose status is UAF_OK (0). */
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 0);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 0);   /* still empty, idempotent */

    CASE("poll returns a count, never an error, when entries are ready");
    uaf_cr_post(&cr, 0u, 0x1234u, UAF_OK, 4096u, 1500u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 1);
    CHECK_EQ_U(out[0].cmd_id, 0x1234u);
    CHECK_EQ_I(out[0].status, UAF_OK);
    CHECK_EQ_U(out[0].bytes_transferred, 4096u);
    CHECK_EQ_U(out[0].latency_ns, 1500u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 0);   /* consumed */

    CASE("max bounds the batch");
    for (uint32_t i = 1; i < 6u; i++)
        uaf_cr_post(&cr, i, (uint16_t)(0x100u + i), UAF_OK, 512u, 10u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 2, out), 2);
    CHECK_EQ_U(out[0].cmd_id, 0x101u);
    CHECK_EQ_U(out[1].cmd_id, 0x102u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 3);

    CASE("phase flips on wrap so stale entries are not re-reported");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u), UAF_OK);
    for (uint32_t i = 0; i < 8u; i++)
        uaf_cr_post(&cr, i, (uint16_t)i, UAF_OK, 64u, 1u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 8);
    CHECK_EQ_U(cr.expected_phase, 0u);          /* flipped after the wrap */
    /* The ring still holds phase-1 entries. They MUST NOT be re-reported. */
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 0);
    /* The producer now writes phase 0 for the second lap. */
    uaf_cr_post(&cr, 0u, 0xBEEFu, UAF_ERR_REMOTE, 0u, 77u, 0u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 8, out), 1);
    CHECK_EQ_U(out[0].cmd_id, 0xBEEFu);
    CHECK_EQ_I(out[0].status, UAF_ERR_REMOTE);

    CASE("a failure status survives the ring intact");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u), UAF_OK);
    uaf_cr_post(&cr, 0u, 1u, UAF_ERR_MR_FAULT, 0u, 0u, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 1, out), 1);
    CHECK_EQ_I(out[0].status, UAF_ERR_MR_FAULT);
    CHECK(out[0].status < 0);

    CASE("latency clamps instead of wrapping");
    CHECK_EQ_I(uaf_cr_init(&cr, ring, 8u), UAF_OK);
    uaf_cr_post(&cr, 0u, 2u, UAF_OK, 4096u, 0xFFFFFFFFu, 1u);
    CHECK_EQ_I(uaf_cr_poll(&cr, 1, out), 1);
    CHECK_EQ_U(out[0].latency_ns, 0xFFFFFFFFu);

    CASE("bad arguments are rejected");
    CHECK_EQ_I(uaf_cr_poll(&cr, -1, out), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_cr_poll(&cr, 1, NULL), UAF_ERR_INVAL);

    TEST_MAIN_END("test_cqe_phase");
}
