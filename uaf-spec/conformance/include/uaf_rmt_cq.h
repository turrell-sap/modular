/* uaf_rmt_cq.h -- the UAF-D intra-host RMT completion ring (Section 9.5).
 * NORMATIVE.
 *
 * [R-9.5-006] The completion ring is a separate array of struct uaf_rmt_cqe in
 * the same CXL window as the send ring, at a fixed offset recorded at queue-pair
 * creation, with the same depth as the send ring. The agent is the producer and
 * the host is the consumer.
 *
 * [R-9.5-007] Ordering is a PHASE BYTE, not an acquire on `head`. An acquire on
 * a shared index would require the host to trust an index the agent advances;
 * the phase tag makes each entry self-describing, exactly as the DST ring does
 * ([R-6.3-002], [R-6.3-005]). v2.1.1 offered both and chose neither.
 */
#ifndef UAF_RMT_CQ_H
#define UAF_RMT_CQ_H

#include "uaf_types.h"

struct uaf_rmt_cq {
    struct uaf_rmt_cqe *base;
    uint32_t            depth;          /* power of two, equal to the SQ depth */
    uint32_t            head;
    uint8_t             expected_phase; /* starts at 1 */
    uint8_t             reserved0[3];
};

int uaf_rmt_cq_init(struct uaf_rmt_cq *cq, struct uaf_rmt_cqe *base,
                    uint32_t depth);

/* Producer side (the agent, and the tests). Writes the body, releases, then
 * stores the phase byte. */
void uaf_rmt_cq_post(struct uaf_rmt_cq *cq, uint32_t slot, uint64_t wr_id,
                     int32_t status, uint32_t byte_len, uint32_t imm_data,
                     uint32_t wc_flags, uint8_t phase);

/* Consumer side (the host). Acquire-loads the phase byte, then converts each
 * accepted entry into the host-visible struct uaf_wc. Returns a count, or a
 * negative enum uaf_error, following [R-4.7-002]. */
int uaf_rmt_cq_poll(struct uaf_rmt_cq *cq, int max, struct uaf_wc *out,
                    uint32_t qp_num);

static inline uint8_t uaf_rmt_cqe_load_phase(const struct uaf_rmt_cqe *e)
{
#if defined(__GNUC__)
    return __atomic_load_n(&e->phase, __ATOMIC_ACQUIRE) & UAF_CQE_PHASE_MASK;
#else
    uint8_t p = e->phase;
    atomic_thread_fence(memory_order_acquire);
    return p & UAF_CQE_PHASE_MASK;
#endif
}

#endif /* UAF_RMT_CQ_H */

/* [R-9.5-002] The agent's window bounds check, in the overflow-safe form.
 * Returns UAF_OK, or UAF_ERR_MR_FAULT when the request leaves the window. */
static inline int uaf_cxl_window_check(uint64_t remote_addr, uint64_t sge_total,
                                       uint64_t cxl_size)
{
    return uaf_range_within(remote_addr, sge_total, cxl_size)
           ? UAF_OK : UAF_ERR_MR_FAULT;
}
