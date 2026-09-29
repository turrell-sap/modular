#include <string.h>
#include <stdatomic.h>
#include "uaf_rmt_cq.h"
#include "uaf_config.h"

int uaf_rmt_cq_init(struct uaf_rmt_cq *cq, struct uaf_rmt_cqe *base,
                    uint32_t depth)
{
    if (!cq || !base || !UAF_IS_POW2(depth)) return UAF_ERR_INVAL;
    if ((uintptr_t)base & (UAF_RMT_CQE_ALIGN - 1u)) return UAF_ERR_INVAL;
    memset(base, 0, (size_t)depth * sizeof(*base));
    cq->base = base;
    cq->depth = depth;
    cq->head = 0u;
    cq->expected_phase = 1u;
    memset(cq->reserved0, 0, sizeof(cq->reserved0));
    return UAF_OK;
}

void uaf_rmt_cq_post(struct uaf_rmt_cq *cq, uint32_t slot, uint64_t wr_id,
                     int32_t status, uint32_t byte_len, uint32_t imm_data,
                     uint32_t wc_flags, uint8_t phase)
{
    struct uaf_rmt_cqe *e = &cq->base[slot & (cq->depth - 1u)];
    e->wr_id    = wr_id;
    e->status   = status;
    e->byte_len = byte_len;
    e->imm_data = imm_data;
    e->wc_flags = wc_flags;
    memset(e->reserved0, 0, sizeof(e->reserved0));
#if defined(__GNUC__)
    __atomic_store_n(&e->phase, (uint8_t)(phase & UAF_CQE_PHASE_MASK),
                     __ATOMIC_RELEASE);
#else
    atomic_thread_fence(memory_order_release);
    e->phase = (uint8_t)(phase & UAF_CQE_PHASE_MASK);
#endif
}

int uaf_rmt_cq_poll(struct uaf_rmt_cq *cq, int max, struct uaf_wc *out,
                    uint32_t qp_num)
{
    if (!cq || !cq->base || !out || max < 0) return UAF_ERR_INVAL;
    int n = 0;
    while (n < max) {
        struct uaf_rmt_cqe *e = &cq->base[cq->head & (cq->depth - 1u)];
        if (uaf_rmt_cqe_load_phase(e) != cq->expected_phase) break;

        /* [R-9.5-008] reserved bytes are checked, not ignored: they are the
         * only spare room in a device-written entry. */
        for (unsigned i = 0; i < sizeof(e->reserved0); i++)
            if (e->reserved0[i]) return UAF_ERR_PROTO;

        memset(&out[n], 0, sizeof(out[n]));
        out[n].wr_id    = e->wr_id;
        out[n].status   = e->status;
        out[n].byte_len = e->byte_len;
        out[n].imm_data = e->imm_data;
        out[n].wc_flags = e->wc_flags;
        out[n].qp_num   = qp_num;
        out[n].src_qp   = qp_num;
        n++;

        cq->head++;
        if ((cq->head & (cq->depth - 1u)) == 0u)
            cq->expected_phase ^= 1u;
    }
    return n;
}
