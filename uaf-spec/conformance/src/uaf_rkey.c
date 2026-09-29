#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include "uaf_rkey.h"
#include "uaf_config.h"

static int csprng_u32(uint32_t *out)
{
    /* [R-11.2-001] A CSPRNG, not rand(). */
    if (getrandom(out, sizeof(*out), 0) != (ssize_t)sizeof(*out))
        return UAF_ERR_PERM;
    return UAF_OK;
}

int uaf_key_table_init(struct uaf_key_table *t, uint32_t depth)
{
    if (!t || depth == 0u) return UAF_ERR_INVAL;
    t->ent = calloc(depth, sizeof(*t->ent));
    if (!t->ent) return UAF_ERR_NOMEM;
    t->depth = depth;
    t->generation = 1u;
    t->fail_count = 0u;
    t->reserved0 = 0u;
    return UAF_OK;
}

void uaf_key_table_fini(struct uaf_key_table *t)
{
    if (!t) return;
    free(t->ent);
    memset(t, 0, sizeof(*t));
}

int uaf_key_register(struct uaf_key_table *t, uint64_t base, uint64_t length,
                     uint64_t access, uint32_t qp_num, uint32_t *lkey,
                     uint32_t *rkey)
{
    if (!t || !t->ent || !lkey || !rkey || length == 0u) return UAF_ERR_INVAL;
    for (uint32_t i = 0; i < t->depth; i++) {
        if (t->ent[i].valid) continue;
        uint32_t l = 0u, r = 0u;
        int rc;
        if ((rc = csprng_u32(&l)) != UAF_OK) return rc;
        do {
            if ((rc = csprng_u32(&r)) != UAF_OK) return rc;
        } while (r == l || r == 0u);      /* [R-11.2-002] distinct, non-zero */
        t->ent[i].rkey       = r;
        t->ent[i].lkey       = l;
        t->ent[i].qp_num     = qp_num;
        t->ent[i].generation = t->generation;
        t->ent[i].base       = base;
        t->ent[i].length     = length;
        t->ent[i].access     = access;
        t->ent[i].valid      = 1u;
        *lkey = l; *rkey = r;
        return UAF_OK;
    }
    return UAF_ERR_NOMEM;
}

int uaf_key_deregister(struct uaf_key_table *t, uint32_t rkey)
{
    if (!t || !t->ent) return UAF_ERR_INVAL;
    for (uint32_t i = 0; i < t->depth; i++) {
        if (t->ent[i].valid && t->ent[i].rkey == rkey) {
            memset(&t->ent[i], 0, sizeof(t->ent[i]));
            t->generation++;             /* [R-11.2-005] */
            return UAF_OK;
        }
    }
    return UAF_ERR_RKEY;
}

int uaf_key_bind_qp(struct uaf_key_table *t, uint32_t rkey, uint32_t qp_num)
{
    if (!t || !t->ent || qp_num == 0u) return UAF_ERR_INVAL;
    for (uint32_t i = 0; i < t->depth; i++) {
        if (t->ent[i].valid && t->ent[i].rkey == rkey) {
            t->ent[i].qp_num = qp_num;
            return UAF_OK;
        }
    }
    return UAF_ERR_RKEY;
}

int uaf_key_validate(struct uaf_key_table *t, uint32_t rkey, uint32_t qp_num,
                     uint64_t va, uint64_t len, uint64_t want,
                     int authenticated)
{
    if (!t || !t->ent) return UAF_ERR_INVAL;
    for (uint32_t i = 0; i < t->depth; i++) {
        const struct uaf_key_ent *e = &t->ent[i];
        if (!e->valid || e->rkey != rkey) continue;
        /* [R-11.2-003] an unbound rkey never validates: the queue-pair
         * component is installed by uaf_key_bind_qp() at connect time. */
        if (e->qp_num == 0u || e->qp_num != qp_num) {
            if (authenticated) t->fail_count++;      /* [R-11.2-004] */
            return UAF_ERR_RKEY;
        }
        if ((e->access & want) != want) return UAF_ERR_PERM;
        if (va < e->base ||
            !uaf_range_within(va - e->base, len, e->length))
            return UAF_ERR_MR_FAULT;          /* one overflow-safe spelling */
        t->fail_count = 0u;
        return UAF_OK;
    }
    if (authenticated) t->fail_count++;  /* [R-11.2-004] */
    return UAF_ERR_RKEY;
}

int uaf_key_should_throttle(const struct uaf_key_table *t)
{ return t && t->fail_count >= UAF_RKEY_FAIL_MAX; }

void uaf_key_reset_failures(struct uaf_key_table *t)
{ if (t) t->fail_count = 0u; }

/* ---- per-source throttling, [R-11.2-008] ------------------------------- */
void uaf_src_note_failure(struct uaf_src_throttle *st, const uint8_t addr[16])
{
    if (!st || !addr) return;
    for (uint32_t i = 0; i < UAF_SRC_THROTTLE_SLOTS; i++)
        if (st->slot[i].used && memcmp(st->slot[i].addr, addr, 16) == 0) {
            st->slot[i].fails++;
            return;
        }
    uint32_t v = st->next % UAF_SRC_THROTTLE_SLOTS;   /* evict round-robin */
    memcpy(st->slot[v].addr, addr, 16);
    st->slot[v].fails = 1u;
    st->slot[v].used  = 1u;
    st->next = v + 1u;
}

int uaf_src_should_throttle(const struct uaf_src_throttle *st,
                            const uint8_t addr[16])
{
    if (!st || !addr) return 0;
    for (uint32_t i = 0; i < UAF_SRC_THROTTLE_SLOTS; i++)
        if (st->slot[i].used && memcmp(st->slot[i].addr, addr, 16) == 0)
            return st->slot[i].fails >= UAF_RKEY_FAIL_MAX;
    return 0;
}

void uaf_src_reset(struct uaf_src_throttle *st)
{ if (st) memset(st, 0, sizeof(*st)); }
