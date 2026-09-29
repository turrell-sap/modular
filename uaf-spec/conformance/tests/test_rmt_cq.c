/* Conformance C-12: the UAF-D intra-host RMT completion ring, Section 9.5. */
#include "uaf_test.h"
#include "uaf_rmt_cq.h"
#include <stdalign.h>

int main(void)
{
    alignas(UAF_RMT_CQE_ALIGN) static struct uaf_rmt_cqe ring[8];
    struct uaf_rmt_cq cq;
    struct uaf_wc wc[8];
    const uint32_t QPN = 0x321u;

    CASE("init requires a power-of-two depth and an aligned base");
    CHECK_EQ_I(uaf_rmt_cq_init(&cq, ring, 6u), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_rmt_cq_init(&cq, ring, 8u), UAF_OK);
    CHECK_EQ_U(cq.expected_phase, 1u);

    CASE("the initial expected phase is 1, not 0");
    /* If a consumer started expecting 0 it would accept every zeroed entry as
     * UAF_OK. v2.1.2's [R-9.5-007] cited the producer release and the consumer
     * acquire and omitted the initial value and the wrap flip. */
    CHECK_EQ_U(cq.expected_phase, 1u);
    CHECK_EQ_U(ring[0].phase & UAF_CQE_PHASE_MASK, 0u);
    CHECK(cq.expected_phase != (ring[0].phase & UAF_CQE_PHASE_MASK));

    CASE("a zeroed ring yields zero completions");
    /* Same reason the DST ring needs a phase tag: status UAF_OK is 0, which is
     * also what an untouched slot holds. */
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 8, wc, QPN), 0);

    CASE("an accepted entry becomes a host uaf_wc");
    uaf_rmt_cq_post(&cq, 0u, 0xFEEDFACE12345678ull, UAF_OK, 4096u,
                    0xABCDu, UAF_WC_WITH_IMM, 1u);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 8, wc, QPN), 1);
    CHECK_EQ_U(wc[0].wr_id, 0xFEEDFACE12345678ull);
    CHECK_EQ_I(wc[0].status, UAF_OK);
    CHECK_EQ_U(wc[0].byte_len, 4096u);
    CHECK_EQ_U(wc[0].imm_data, 0xABCDu);
    CHECK(wc[0].wc_flags & UAF_WC_WITH_IMM);
    CHECK_EQ_U(wc[0].qp_num, QPN);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 8, wc, QPN), 0);

    CASE("poll returns a count and respects max");
    for (uint32_t i = 1; i < 6u; i++)
        uaf_rmt_cq_post(&cq, i, 0x1000ull + i, UAF_OK, 64u, 0u, 0u, 1u);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 2, wc, QPN), 2);
    CHECK_EQ_U(wc[0].wr_id, 0x1001ull);
    CHECK_EQ_U(wc[1].wr_id, 0x1002ull);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 8, wc, QPN), 3);

    CASE("phase flips on wrap and stale entries are not re-reported");
    CHECK_EQ_I(uaf_rmt_cq_init(&cq, ring, 8u), UAF_OK);
    for (uint32_t i = 0; i < 8u; i++)
        uaf_rmt_cq_post(&cq, i, 0x2000ull + i, UAF_OK, 8u, 0u, 0u, 1u);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 8, wc, QPN), 8);
    CHECK_EQ_U(cq.expected_phase, 0u);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 8, wc, QPN), 0);
    uaf_rmt_cq_post(&cq, 0u, 0x3000ull, UAF_ERR_RKEY, 0u, 0u, 0u, 0u);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 8, wc, QPN), 1);
    CHECK_EQ_U(wc[0].wr_id, 0x3000ull);
    CHECK_EQ_I(wc[0].status, UAF_ERR_RKEY);

    CASE("a failure status reaches the host intact");
    CHECK_EQ_I(uaf_rmt_cq_init(&cq, ring, 8u), UAF_OK);
    uaf_rmt_cq_post(&cq, 0u, 9u, UAF_ERR_MR_FAULT, 0u, 0u, 0u, 1u);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 1, wc, QPN), 1);
    CHECK_EQ_I(wc[0].status, UAF_ERR_MR_FAULT);
    CHECK(wc[0].status < 0);

    CASE("non-zero reserved bytes in a device-written entry are rejected");
    CHECK_EQ_I(uaf_rmt_cq_init(&cq, ring, 8u), UAF_OK);
    uaf_rmt_cq_post(&cq, 0u, 1u, UAF_OK, 8u, 0u, 0u, 1u);
    ring[0].reserved0[3] = 0x01u;
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 1, wc, QPN), UAF_ERR_PROTO);

    CASE("the atomic result convention is visible in the completion");
    /* [R-4.11-001] is profile-independent, so it applies on this path too. */
    CHECK_EQ_I(uaf_rmt_cq_init(&cq, ring, 8u), UAF_OK);
    uaf_rmt_cq_post(&cq, 0u, 42u, UAF_OK, 8u, 0u, UAF_WC_ATOMIC_ORIG, 1u);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 1, wc, QPN), 1);
    CHECK(wc[0].wc_flags & UAF_WC_ATOMIC_ORIG);
    CHECK_EQ_U(wc[0].byte_len, 8u);

    CASE("the window bounds check cannot be defeated by overflow");
    /* v2.1.3 stated this as remote_addr + L > cxl_size. For uint64_t that wraps:
     * 2^64 - 8 plus 16 is 8, which passes a naive test and reads 16 bytes from
     * an address 8 below the end of the address space. */
    const uint64_t WIN = 0x40000000ull;                      /* 1 GiB window */
    CHECK_EQ_I(uaf_cxl_window_check(0u, WIN, WIN), UAF_OK);
    CHECK_EQ_I(uaf_cxl_window_check(WIN - 8u, 8u, WIN), UAF_OK);
    CHECK_EQ_I(uaf_cxl_window_check(WIN - 8u, 9u, WIN), UAF_ERR_MR_FAULT);
    CHECK_EQ_I(uaf_cxl_window_check(WIN, 1u, WIN), UAF_ERR_MR_FAULT);
    CHECK_EQ_I(uaf_cxl_window_check(0xFFFFFFFFFFFFFFF8ull, 16u, WIN),
               UAF_ERR_MR_FAULT);                 /* the wrapping case */
    CHECK_EQ_I(uaf_cxl_window_check(0xFFFFFFFFFFFFFFFFull, 1u, WIN),
               UAF_ERR_MR_FAULT);
    CHECK_EQ_I(uaf_cxl_window_check(8u, 0xFFFFFFFFFFFFFFFFull, WIN),
               UAF_ERR_MR_FAULT);
    /* A zero-length request is within the window exactly when its offset is.
     * v2.1.4's helper had a len == 0 shortcut that accepted a zero-length
     * request past the end, disagreeing with [R-9.5-002], which rejects it on
     * the offset term alone. The helper and the inequality now agree. */
    CHECK_EQ_I(uaf_cxl_window_check(0u, 0u, 0u), UAF_OK);
    CHECK_EQ_I(uaf_cxl_window_check(WIN, 0u, WIN), UAF_OK);      /* at the end */
    CHECK_EQ_I(uaf_cxl_window_check(WIN + 1u, 0u, WIN),
               UAF_ERR_MR_FAULT);                                /* past it    */
    CHECK_EQ_I(uaf_cxl_window_check(0xFFFFFFFFFFFFFFFFull, 0u, WIN),
               UAF_ERR_MR_FAULT);
    /* Cross-check the helper against the inequality of [R-9.5-002] over a
     * spread of inputs, including the wrapping ones. */
    {
        static const uint64_t V[] = {
            0u, 1u, 8u, WIN - 1u, WIN, WIN + 1u, 0xFFFFFFFFull,
            0xFFFFFFFFFFFFFFF8ull, 0xFFFFFFFFFFFFFFFFull
        };
        for (unsigned a = 0; a < sizeof(V)/sizeof(V[0]); a++)
            for (unsigned b = 0; b < sizeof(V)/sizeof(V[0]); b++) {
                int helper = uaf_range_within(V[a], V[b], WIN);
                int rule   = !(V[a] > WIN || V[b] > WIN - (V[a] > WIN ? 0 : V[a]));
                if (V[a] > WIN) rule = 0;      /* first term short-circuits */
                CHECK_EQ_I(helper, rule);
            }
    }

    CASE("bad arguments are rejected");
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, -1, wc, QPN), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_rmt_cq_poll(&cq, 1, NULL, QPN), UAF_ERR_INVAL);

    TEST_MAIN_END("test_rmt_cq");
}
