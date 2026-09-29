#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include "uaf_dst.h"

int uaf_dst_nvme_opcode(uint32_t op)
{
    switch (op) {
    case UAF_WR_STORAGE_READ:  return UAF_NVME_OPC_READ;
    case UAF_WR_STORAGE_WRITE: return UAF_NVME_OPC_WRITE;
    case UAF_WR_STORAGE_FLUSH: return UAF_NVME_OPC_FLUSH;  /* [R-6.2-001] */
    default:                   return UAF_ERR_INVAL;
    }
}

int uaf_dst_nlb_to_cdw12(uint32_t nlb, uint16_t *out)
{
    if (!out) return UAF_ERR_INVAL;
    if (nlb == 0u) return UAF_ERR_INVAL;              /* [R-6.2-002] */
    if (nlb > UAF_DST_NLB_MAX) return UAF_ERR_INVAL;
    *out = (uint16_t)(nlb - 1u);
    return UAF_OK;
}

int uaf_dst_dptr_kind(uint64_t iova, uint64_t len, uint32_t page_size,
                      int sgl_supported, enum uaf_dst_dptr *kind)
{
    if (!kind || len == 0u || !UAF_IS_POW2(page_size)) return UAF_ERR_INVAL;
    uint64_t page_mask = (uint64_t)page_size - 1u;
    uint64_t first_end = (iova | page_mask) + 1u;      /* end of page 0 */
    if (len <= first_end - iova)      *kind = UAF_DPTR_PRP1;
    else if (len <= first_end - iova + page_size) *kind = UAF_DPTR_PRP1_PRP2;
    else *kind = sgl_supported ? UAF_DPTR_SGL : UAF_DPTR_PRP_LIST;
    return UAF_OK;
}

int uaf_cid_init(struct uaf_cid_table *t, uint32_t depth)
{
    if (!t || !UAF_IS_POW2(depth) || depth > 65536u) return UAF_ERR_INVAL;
    t->wr_id  = calloc(depth, sizeof(uint64_t));
    t->in_use = calloc(depth, sizeof(uint8_t));
    if (!t->wr_id || !t->in_use) {
        free(t->wr_id); free(t->in_use);
        t->wr_id = NULL; t->in_use = NULL;
        return UAF_ERR_NOMEM;
    }
    t->depth = depth;
    t->next  = 0u;
    return UAF_OK;
}

void uaf_cid_fini(struct uaf_cid_table *t)
{
    if (!t) return;
    free(t->wr_id); free(t->in_use);
    memset(t, 0, sizeof(*t));
}

int uaf_cid_alloc(struct uaf_cid_table *t, uint64_t wr_id, uint16_t *cid)
{
    if (!t || !t->in_use || !cid) return UAF_ERR_INVAL;
    uint32_t mask = t->depth - 1u;
    for (uint32_t n = 0; n < t->depth; n++) {
        uint32_t i = (t->next + n) & mask;
        if (!t->in_use[i]) {
            t->in_use[i] = 1u;
            t->wr_id[i]  = wr_id;
            t->next      = (i + 1u) & mask;
            *cid         = (uint16_t)i;
            return UAF_OK;
        }
    }
    return UAF_ERR_BUSY;
}

int uaf_cid_release(struct uaf_cid_table *t, uint16_t cid, uint64_t *wr_id)
{
    if (!t || !t->in_use) return UAF_ERR_INVAL;
    if ((uint32_t)cid >= t->depth) return UAF_ERR_PROTO;
    if (!t->in_use[cid]) return UAF_ERR_PROTO;
    if (wr_id) *wr_id = t->wr_id[cid];
    t->in_use[cid] = 0u;
    return UAF_OK;
}

int uaf_cr_init(struct uaf_cr_state *cr, struct uaf_storage_cqe *base,
                uint32_t depth, struct uaf_cid_table *cids)
{
    if (!cr || !base || !UAF_IS_POW2(depth)) return UAF_ERR_INVAL;
    /* [R-6.3-002] The ring base MUST be UAF_CQE_ALIGN-aligned. */
    if ((uintptr_t)base & (UAF_CQE_ALIGN - 1u)) return UAF_ERR_INVAL;
    memset(base, 0, (size_t)depth * sizeof(*base));
    cr->base = base;
    cr->cids = cids;
    cr->depth = depth;
    cr->head = 0u;
    cr->expected_phase = 1u;          /* [R-6.3-001] */
    memset(cr->reserved0, 0, sizeof(cr->reserved0));
    return UAF_OK;
}

void uaf_cr_post(struct uaf_cr_state *cr, uint32_t slot, uint16_t cmd_id,
                 int16_t status, uint32_t bytes, uint32_t latency_ns,
                 uint8_t phase)
{
    struct uaf_storage_cqe *e = &cr->base[slot & (cr->depth - 1u)];
    e->cmd_id = cmd_id;
    e->status = status;
    e->bytes_transferred = bytes;
    e->latency_ns = latency_ns;
    e->reserved0 = 0u;
    e->reserved1 = 0u;
    /* phase is released last, after the body is visible ([R-6.3-002]). */
#if defined(__GNUC__)
    __atomic_store_n(&e->phase, (uint8_t)(phase & UAF_CQE_PHASE_MASK),
                     __ATOMIC_RELEASE);
#else
    atomic_thread_fence(memory_order_release);
    e->phase = (uint8_t)(phase & UAF_CQE_PHASE_MASK);
#endif
}

int uaf_cr_poll(struct uaf_cr_state *cr, int max, struct uaf_storage_wc *out)
{
    if (!cr || !cr->base || !out || max < 0) return UAF_ERR_INVAL;
    int n = 0;
    while (n < max) {
        struct uaf_storage_cqe *e = &cr->base[cr->head & (cr->depth - 1u)];
        if (uaf_cqe_load_phase(e) != cr->expected_phase) break;  /* acquire */

        uint64_t wr_id = 0u;
        if (cr->cids) {
            /* [R-6.5-001] restore the caller's full 64-bit identifier */
            int rc = uaf_cid_release(cr->cids, e->cmd_id, &wr_id);
            if (rc != UAF_OK) return rc;   /* [R-6.5-003] unknown cmd_id */
        } else {
            wr_id = e->cmd_id;
        }
        out[n].wr_id             = wr_id;
        out[n].status            = e->status;
        out[n].bytes_transferred = e->bytes_transferred;
        out[n].latency_ns        = e->latency_ns;
        out[n].reserved0         = 0u;
        n++;

        cr->head++;
        if ((cr->head & (cr->depth - 1u)) == 0u)
            cr->expected_phase ^= 1u;             /* flip on wrap */
    }
    return n;   /* 0 means empty, never an error code */
}

int uaf_dst_status_to_uaf(uint8_t sct, uint8_t sc)
{
    if (sct == UAF_NVME_SCT_GENERIC && sc == 0x00) return UAF_OK;
    switch (sct) {
    case UAF_NVME_SCT_GENERIC:
        switch (sc) {
        case 0x01: return UAF_ERR_INVAL;         /* Invalid Command Opcode   */
        case 0x02: return UAF_ERR_INVAL;         /* Invalid Field in Command */
        case 0x03: return UAF_ERR_PROTO;         /* Command ID Conflict      */
        case 0x04: return UAF_ERR_REMOTE;        /* Data Transfer Error      */
        case 0x06: return UAF_ERR_REMOTE;        /* Internal Error           */
        case 0x07: return UAF_ERR_BUSY;          /* Command Abort Requested  */
        case 0x0B: return UAF_ERR_MR_FAULT;      /* Invalid PRP Offset       */
        case 0x0D: return UAF_ERR_MR_FAULT;      /* Invalid SGL Descriptor   */
        case 0x80: return UAF_ERR_MR_FAULT;      /* LBA Out of Range         */
        case 0x81: return UAF_ERR_NOMEM;         /* Capacity Exceeded        */
        case 0x82: return UAF_ERR_NODEV;         /* Namespace Not Ready      */
        default:   return UAF_ERR_REMOTE;
        }
    case UAF_NVME_SCT_CMD_SPEC:
        return (sc == 0x1E) ? UAF_ERR_PERM : UAF_ERR_INVAL;
    case UAF_NVME_SCT_MEDIA:
        switch (sc) {
        case 0x81: return UAF_ERR_CRC;           /* Data Transfer Guard Check */
        case 0x82: return UAF_ERR_CRC;           /* Guard Check Error         */
        case 0x83: return UAF_ERR_CRC;           /* Application Tag Check     */
        case 0x84: return UAF_ERR_CRC;           /* Reference Tag Check       */
        case 0x86: return UAF_ERR_PERM;          /* Access Denied             */
        default:   return UAF_ERR_REMOTE;        /* unrecovered media error   */
        }
    case UAF_NVME_SCT_PATH:
        return UAF_ERR_TIMEOUT;
    default:
        return UAF_ERR_REMOTE;
    }
}
