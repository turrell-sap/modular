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

int uaf_key_validate(struct uaf_key_table *t, uint32_t rkey, uint32_t qp_num,
                     uint64_t va, uint64_t len, uint64_t want,
                     int authenticated)
{
    if (!t || !t->ent) return UAF_ERR_INVAL;
    for (uint32_t i = 0; i < t->depth; i++) {
        const struct uaf_key_ent *e = &t->ent[i];
        if (!e->valid || e->rkey != rkey) continue;
        /* [R-11.2-003] bound to the QP it was issued for */
        if (e->qp_num != 0u && e->qp_num != qp_num) {
            if (authenticated) t->fail_count++;      /* [R-11.2-004] */
            return UAF_ERR_RKEY;
        }
        if ((e->access & want) != want) return UAF_ERR_PERM;
        if (va < e->base || len > e->length ||
            va - e->base > e->length - len) return UAF_ERR_MR_FAULT;
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
