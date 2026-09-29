#include <string.h>
#include "uaf_ring.h"

int uaf_ring_init(struct uaf_ring *r, void *base, uint32_t depth,
                  uint32_t esize)
{
    if (!r || !base || !esize) return UAF_ERR_INVAL;
    if (!UAF_IS_POW2(depth)) return UAF_ERR_INVAL;   /* [R-10.3-002] */
    memset(r, 0, sizeof(*r));
    atomic_store_explicit(&r->head, 0u, memory_order_relaxed);
    atomic_store_explicit(&r->tail, 0u, memory_order_relaxed);
    r->depth = depth;
    r->mask  = depth - 1u;
    r->esize = esize;
    r->base  = base;
    return UAF_OK;
}

uint32_t uaf_ring_count(const struct uaf_ring *r)
{
    uint32_t t = atomic_load_explicit(&r->tail, memory_order_acquire);
    uint32_t h = atomic_load_explicit(&r->head, memory_order_acquire);
    return t - h;   /* unsigned wrap is well defined and intended */
}

int uaf_ring_full(const struct uaf_ring *r)
{ return uaf_ring_count(r) == r->depth; }

int uaf_ring_empty(const struct uaf_ring *r)
{ return uaf_ring_count(r) == 0u; }

void *uaf_ring_produce(struct uaf_ring *r, uint32_t *slot_index)
{
    if (uaf_ring_full(r)) return NULL;
    uint32_t t = atomic_load_explicit(&r->tail, memory_order_relaxed);
    uint32_t i = t & r->mask;          /* [R-10.3-003] mask of THIS ring */
    if (slot_index) *slot_index = i;
    return (uint8_t *)r->base + (size_t)i * r->esize;
}

void uaf_ring_commit(struct uaf_ring *r)
{
    uint32_t t = atomic_load_explicit(&r->tail, memory_order_relaxed);
    atomic_store_explicit(&r->tail, t + 1u, memory_order_release);
}

void *uaf_ring_consume(struct uaf_ring *r)
{
    if (uaf_ring_empty(r)) return NULL;
    uint32_t h = atomic_load_explicit(&r->head, memory_order_relaxed);
    return (uint8_t *)r->base + (size_t)(h & r->mask) * r->esize;
}

void uaf_ring_release(struct uaf_ring *r)
{
    uint32_t h = atomic_load_explicit(&r->head, memory_order_relaxed);
    atomic_store_explicit(&r->head, h + 1u, memory_order_release);
}
