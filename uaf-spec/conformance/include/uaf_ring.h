/* uaf_ring.h -- UAF-SPEC-001 v2.1 ring conventions. NORMATIVE.
 *
 * [R-10.3-001] One convention for every ring in this specification, taken
 * from NVMe: the PRODUCER advances `tail`, the CONSUMER advances `head`.
 * v2.0 used the opposite convention in Section 9.2 from the one in Section
 * 9.3, inside the same backend.
 *
 * [R-10.3-002] Every ring depth MUST be a power of two, and index wrap MUST
 * be a mask. v2.0 wrapped a send ring with `% UAF_CQ_DEPTH_DEFAULT` (1024)
 * while the ring was allocated at UAF_SQ_DEPTH_DEFAULT (256) entries, which
 * writes up to 49152 bytes past the end for a 64-byte work request.
 *
 * [R-10.3-003] The ring index modulus MUST be the depth of the ring being
 * indexed. A constant naming a different queue MUST NOT appear.
 *
 * [R-10.3-004] Rings are SINGLE-PRODUCER, SINGLE-CONSUMER. Two threads
 * posting to one queue pair is undefined unless UAF_CAP_MT_POST is set.
 * v2.0's samples were single-producer and the spec never said so.
 */
#ifndef UAF_RING_H
#define UAF_RING_H

#include <stdint.h>
#include <stdatomic.h>
#include "uaf_config.h"
#include "uaf_types.h"

struct uaf_ring {
    _Atomic uint32_t head;   /* consumer index */
    _Atomic uint32_t tail;   /* producer index */
    uint32_t         depth;  /* power of two */
    uint32_t         mask;   /* depth - 1 */
    uint32_t         esize;  /* entry size in bytes */
    uint32_t         reserved0;
    void            *base;
};

int      uaf_ring_init(struct uaf_ring *r, void *base, uint32_t depth,
                       uint32_t esize);
uint32_t uaf_ring_count(const struct uaf_ring *r);
int      uaf_ring_full(const struct uaf_ring *r);
int      uaf_ring_empty(const struct uaf_ring *r);

/* Reserves the next producer slot. Returns the slot address, or NULL when
 * full. The caller writes the entry, then calls uaf_ring_commit(). */
void    *uaf_ring_produce(struct uaf_ring *r, uint32_t *slot_index);
void     uaf_ring_commit(struct uaf_ring *r);

/* Returns the next consumer slot, or NULL when empty. */
void    *uaf_ring_consume(struct uaf_ring *r);
void     uaf_ring_release(struct uaf_ring *r);

/* ---- Device-visible ordering (Section 9.4) ----------------------------
 * [R-9.4-001] A doorbell write MUST be preceded by a barrier that orders
 * prior descriptor stores against a store to device memory. An
 * atomic_thread_fence(memory_order_seq_cst) is NOT such a barrier: on
 * AArch64 it compiles to `dmb ish`, which orders inner-shareable cacheable
 * accesses and says nothing about Device-nGnRE. v2.0's "doorbell ordering
 * fix" used exactly that fence.
 *
 * [R-9.4-002] A submission ring that a device reads MUST be either
 * device-coherent (CXL.cache, or a hardware-coherent interconnect) or
 * explicitly cleaned before the doorbell. A cacheable ring plus a bare
 * doorbell is a stale-descriptor bug. */
static inline void uaf_dma_wmb(void)
{
#if defined(__aarch64__)
    __asm__ __volatile__("dsb st" ::: "memory");
#elif defined(__arm__)
    __asm__ __volatile__("dsb st" ::: "memory");
#elif defined(__x86_64__) || defined(__i386__)
    __asm__ __volatile__("sfence" ::: "memory");
#else
    atomic_thread_fence(memory_order_seq_cst);
#endif
}

static inline void uaf_mmio_write32(volatile void *addr, uint32_t v)
{
    uaf_dma_wmb();
    *(volatile uint32_t *)addr = v;
}

#endif /* UAF_RING_H */
