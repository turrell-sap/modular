/* uaf_dst.h -- UAF-SPEC-001 v2.1 Direct Storage Transport. NORMATIVE.
 *
 * v2.0 Section 6.1 specified a 64-byte "UAF SQE" that no backend ever
 * filled; every sample built an NVMe SQE instead. That pseudo-format is
 * deleted. DST is specified as a mapping from uaf_storage_cmd onto the NVMe
 * I/O command set, which is what implementations actually emit.
 */
#ifndef UAF_DST_H
#define UAF_DST_H

#include <stdint.h>
#include "uaf_config.h"
#include "uaf_types.h"

/* ---- NVMe I/O opcodes (NVM Express Base 2.0d) ------------------------- */
#define UAF_NVME_OPC_FLUSH  0x00
#define UAF_NVME_OPC_WRITE  0x01
#define UAF_NVME_OPC_READ   0x02

/* [R-6.2-001] The UAF-to-NVMe opcode map is exhaustive and total. v2.0's
 * sample used a two-way ternary on READ, so UAF_WR_STORAGE_FLUSH fell
 * through and was issued as a WRITE. Returns a negative error for anything
 * not in the map. */
int uaf_dst_nvme_opcode(uint32_t uaf_opcode);

/* [R-6.2-002] nlb in uaf_storage_cmd is a 1-BASED block count. NVMe CDW12
 * carries a 0-BASED count. The conversion is the library's, it MUST reject
 * nlb == 0, and it MUST reject nlb > 65536. v2.0 computed
 * `(nlb - 1) & 0xFFFF` with no zero check, turning a zero-length request
 * into a 65536-block, 256 MiB transfer. */
#define UAF_DST_NLB_MAX  65536u
int uaf_dst_nlb_to_cdw12(uint32_t nlb, uint16_t *cdw12_nlb0);

/* [R-6.2-003] Data pointer rule. A single PRP entry describes at most one
 * page boundary crossing; anything larger MUST use a PRP list or an NVMe
 * SGL. v2.0 put the whole buffer in dptr_prp1 regardless of length. */
enum uaf_dst_dptr {
    UAF_DPTR_PRP1      = 1, /* fits in PRP1 alone                        */
    UAF_DPTR_PRP1_PRP2 = 2, /* two pages: PRP1 + PRP2                    */
    UAF_DPTR_PRP_LIST  = 3, /* > 2 pages: PRP2 points at a PRP list      */
    UAF_DPTR_SGL       = 4, /* controller supports SGLs and caller opted */
};
int uaf_dst_dptr_kind(uint64_t iova, uint64_t len, uint32_t page_size,
                      int sgl_supported, enum uaf_dst_dptr *kind);

/* ---- Command identifier allocation (Section 6.5) ----------------------
 * [R-6.5-001] cmd_id is 16 bits and wr_id is 64. v2.0 truncated
 * (wr_id & 0xFFFF) into the completion, so two in-flight commands whose
 * wr_ids differed above bit 16 collided and the application could not
 * recover its own value. cmd_id is now allocated by the library as an index
 * into an in-flight table, and the library restores the full wr_id before
 * handing a completion to the caller. */
struct uaf_cid_table {
    uint64_t *wr_id;     /* depth entries */
    uint8_t  *in_use;    /* depth entries */
    uint32_t  depth;     /* power of two  */
    uint32_t  next;
};

int  uaf_cid_init(struct uaf_cid_table *t, uint32_t depth);
void uaf_cid_fini(struct uaf_cid_table *t);
/* Returns UAF_OK and sets *cid, or UAF_ERR_BUSY when the table is full. */
int  uaf_cid_alloc(struct uaf_cid_table *t, uint64_t wr_id, uint16_t *cid);
/* Returns UAF_OK and sets *wr_id, or UAF_ERR_PROTO for an unknown cid. */
int  uaf_cid_release(struct uaf_cid_table *t, uint16_t cid, uint64_t *wr_id);

/* ---- Completion ring polling (Section 6.3) ---------------------------
 * [R-6.3-001] A completion ring is zero-filled at creation and the initial
 * expected phase is 1. A consumer MUST read `phase` first, compare it with
 * the expected phase, and only then read the remainder of the entry. The
 * expected phase flips on every wrap. Without this a zeroed slot is
 * indistinguishable from a completion whose status is UAF_OK (0) -- v2.0 had
 * no phase tag at all, so DST polling could not be implemented. */
struct uaf_cr_state {
    struct uaf_storage_cqe *base;   /* the device ring image */
    struct uaf_cid_table   *cids;   /* resolves cmd_id -> the caller's wr_id */
    uint32_t depth;          /* power of two */
    uint32_t head;
    uint8_t  expected_phase; /* starts at 1 */
    uint8_t  reserved0[3];
};

int uaf_cr_init(struct uaf_cr_state *cr, struct uaf_storage_cqe *base,
                uint32_t depth, struct uaf_cid_table *cids);

/* [R-6.3-003] The consumer SHALL read `phase` with ACQUIRE semantics. Reading
 * it and then reading the body in program order is not enough: on AArch64 the
 * body loads may be satisfied before the phase load. v2.1 required a release
 * store on the producer and gave the consumer no matching acquire, on the very
 * path the phase tag exists to make safe. */
static inline uint8_t uaf_cqe_load_phase(const struct uaf_storage_cqe *e)
{
#if defined(__GNUC__)
    return __atomic_load_n(&e->phase, __ATOMIC_ACQUIRE) & UAF_CQE_PHASE_MASK;
#else
    uint8_t p = e->phase;
    atomic_thread_fence(memory_order_acquire);
    return p & UAF_CQE_PHASE_MASK;
#endif
}

/* Returns the number of completions copied to out[0..max-1], 0 if none were
 * ready, or a negative enum uaf_error. Fills the PUBLIC host completion, whose
 * wr_id is the caller's full 64-bit value ([R-6.5-001]). */
int uaf_cr_poll(struct uaf_cr_state *cr, int max, struct uaf_storage_wc *out);
/* Producer side, for tests and for software backends. */
void uaf_cr_post(struct uaf_cr_state *cr, uint32_t slot, uint16_t cmd_id,
                 int16_t status, uint32_t bytes, uint32_t latency_ns,
                 uint8_t phase);

/* ---- NVMe completion status mapping (Section 6.7) ---------------------
 * [R-6.7-001] An NVMe status field SHALL be mapped to an enum uaf_error. v2.1
 * specified the opcode and NLB mapping and left completion status unmapped, so
 * a DST implementation could submit a command and had no defined way to report
 * why it failed.
 *
 * `sct` is the Status Code Type (CDW3 bits 27..25) and `sc` the Status Code
 * (CDW3 bits 24..17) of the NVMe completion. */
#define UAF_NVME_SCT_GENERIC   0x0
#define UAF_NVME_SCT_CMD_SPEC  0x1
#define UAF_NVME_SCT_MEDIA     0x2
#define UAF_NVME_SCT_PATH      0x3

int uaf_dst_status_to_uaf(uint8_t sct, uint8_t sc);

#endif /* UAF_DST_H */
