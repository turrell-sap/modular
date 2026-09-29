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
 *              not issued for MUST fail with UAF_ERR_RKEY. The queue-pair
 *              component is installed at CONNECT time by uaf_key_bind_qp(): a
 *              queue pair does not exist when memory is registered, and an
 *              unbound rkey MUST NOT validate. v2.1.1's informative samples
 *              registered with qp_num 0 and never bound, so the requirement
 *              described a state nothing reached.
 * [R-11.2-004] After UAF_RKEY_FAIL_MAX consecutive failures -- counted only
 *              for packets that passed the link authenticator -- the responder
 *              MUST throttle validation on that QP to one attempt per
 *              UAF_RKEY_THROTTLE_NS. It MUST NOT change queue-pair state.
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

/* Installs the queue-pair component of the binding. Called from uaf_connect().
 * qp_num MUST be non-zero. */
int  uaf_key_bind_qp(struct uaf_key_table *t, uint32_t rkey, uint32_t qp_num);

/* Validates rkey for [va, va+len) with `want` access from qp_num.
 * `authenticated` states whether the packet passed the link authenticator;
 * only an authenticated failure may advance the counter, so an off-path
 * attacker cannot drive the responder into throttling either.
 * Returns UAF_OK, UAF_ERR_RKEY, UAF_ERR_PERM or UAF_ERR_MR_FAULT. */
int  uaf_key_validate(struct uaf_key_table *t, uint32_t rkey, uint32_t qp_num,
                      uint64_t va, uint64_t len, uint64_t want,
                      int authenticated);
/* True once validation on this QP must be rate-limited. Never a reason to
 * change queue-pair state. */
int  uaf_key_should_throttle(const struct uaf_key_table *t);
void uaf_key_reset_failures(struct uaf_key_table *t);

/* [R-11.2-008] Where no link authenticator is configured -- permitted on a
 * trusted fabric by [R-11.1-002] -- an rkey failure cannot be attributed to an
 * authenticated peer, so [R-11.2-004]'s counter never advances and the 32-bit
 * key space is scannable again in seconds. A responder in that configuration
 * SHALL throttle per SOURCE ADDRESS instead. Queue-pair state is still never
 * changed, so the off-path kill stays closed. */
#define UAF_SRC_THROTTLE_SLOTS 16
struct uaf_src_throttle {
    struct { uint8_t addr[16]; uint32_t fails; uint8_t used; } slot[UAF_SRC_THROTTLE_SLOTS];
    uint32_t next;
};
void uaf_src_note_failure(struct uaf_src_throttle *st, const uint8_t addr[16]);
int  uaf_src_should_throttle(const struct uaf_src_throttle *st,
                             const uint8_t addr[16]);
void uaf_src_reset(struct uaf_src_throttle *st);

#endif
