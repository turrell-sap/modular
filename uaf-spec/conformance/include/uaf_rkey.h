/* uaf_rkey.h -- UAF-SPEC-001 v2.1 memory key model. NORMATIVE.
 *
 * ONE algorithm for every backend (Section 10.2 / 11.2). v2.0 promised a
 * 32-bit CSPRNG rkey in Section 11.1 and then derived it from a file
 * descriptor XOR a build-time constant on UAF-S, and from the CXL offset,
 * length and flags on UAF-D.
 *
 * [R-11.2-001] lkey and rkey MUST come from a CSPRNG and MUST NOT be derived
 *              from an address, an offset, a length, a file descriptor or any
 *              other value an attacker can guess or observe.
 * [R-11.2-002] lkey and rkey MUST be distinct values.
 * [R-11.2-003] An rkey is bound to (domain, queue pair, base, length) and to
 *              a generation. A request presenting a valid rkey on a QP it was
 *              not issued for MUST fail with UAF_ERR_RKEY.
 * [R-11.2-004] After UAF_RKEY_FAIL_MAX consecutive validation failures a QP
 *              MUST move to UAF_QPS_ERR. A 32-bit key space is otherwise
 *              exhaustible in seconds at 400 Gbps.
 * [R-11.2-005] Deregistration MUST increment the generation so a stale rkey
 *              can never match a later registration.
 */
#ifndef UAF_RKEY_H
#define UAF_RKEY_H
#include "uaf_types.h"

struct uaf_key_ent {
    uint32_t rkey;
    uint32_t lkey;
    uint32_t qp_num;      /* 0 means any QP in the domain */
    uint32_t generation;
    uint64_t base;
    uint64_t length;
    uint64_t access;      /* enum uaf_mr_flags */
    uint8_t  valid;
    uint8_t  reserved0[7];
};

struct uaf_key_table {
    struct uaf_key_ent *ent;
    uint32_t            depth;
    uint32_t            generation;
    uint32_t            fail_count;
    uint32_t            reserved0;
};

int  uaf_key_table_init(struct uaf_key_table *t, uint32_t depth);
void uaf_key_table_fini(struct uaf_key_table *t);

/* Generates a CSPRNG lkey/rkey pair and binds it. */
int  uaf_key_register(struct uaf_key_table *t, uint64_t base, uint64_t length,
                      uint64_t access, uint32_t qp_num, uint32_t *lkey,
                      uint32_t *rkey);
int  uaf_key_deregister(struct uaf_key_table *t, uint32_t rkey);

/* Validates rkey for [va, va+len) with `want` access from qp_num.
 * Returns UAF_OK, UAF_ERR_RKEY, UAF_ERR_PERM or UAF_ERR_MR_FAULT. On
 * UAF_ERR_RKEY the table's failure counter advances; uaf_key_should_err()
 * reports when the QP must be moved to ERR. */
int  uaf_key_validate(struct uaf_key_table *t, uint32_t rkey, uint32_t qp_num,
                      uint64_t va, uint64_t len, uint64_t want);
int  uaf_key_should_err(const struct uaf_key_table *t);
void uaf_key_reset_failures(struct uaf_key_table *t);

#endif
