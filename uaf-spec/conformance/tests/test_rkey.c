/* Conformance C-5: one key algorithm, bound and rate-limited. Section 11.2. */
#include "uaf_test.h"
#include "uaf_rkey.h"
#include "uaf_config.h"

int main(void)
{
    struct uaf_key_table t;
    CHECK_EQ_I(uaf_key_table_init(&t, 64), UAF_OK);

    uint32_t lk = 0, rk = 0;
    CASE("register yields distinct, non-zero, unguessable keys");
    CHECK_EQ_I(uaf_key_register(&t, 0x100000u, 4096u,
                                UAF_MR_REMOTE_WRITE | UAF_MR_REMOTE_READ,
                                7u, &lk, &rk), UAF_OK);
    CHECK(lk != 0u); CHECK(rk != 0u); CHECK(lk != rk);
    /* v2.0's UAF-S rkey was fd ^ 0xA5A55A5A, so a key was derivable from a
     * small integer. Nothing here may be a function of the address. */
    CHECK(rk != (uint32_t)0x100000u);
    CHECK(rk != ((uint32_t)0x100000u ^ 0xA5A55A5Au));

    CASE("distinct registrations get distinct keys");
    uint32_t seen[32]; int nseen = 0;
    for (int i = 0; i < 32; i++) {
        uint32_t l2 = 0, r2 = 0;
        CHECK_EQ_I(uaf_key_register(&t, 0x200000u + (uint64_t)i * 4096u, 4096u,
                                    UAF_MR_REMOTE_READ, 7u, &l2, &r2), UAF_OK);
        for (int j = 0; j < nseen; j++) CHECK(seen[j] != r2);
        seen[nseen++] = r2;
    }

    CASE("an unbound rkey does not validate until connect binds it");
    /* v2.1.1's samples registered with qp_num 0 and never bound, so
     * [R-11.2-003] described a state nothing ever reached. */
    {
        uint32_t l0 = 0, r0 = 0;
        CHECK_EQ_I(uaf_key_register(&t, 0x900000u, 4096u,
                                    UAF_MR_REMOTE_WRITE, 0u, &l0, &r0), UAF_OK);
        CHECK_EQ_I(uaf_key_validate(&t, r0, 11u, 0x900000u, 8u,
                                    UAF_MR_REMOTE_WRITE, 1), UAF_ERR_RKEY);
        CHECK_EQ_I(uaf_key_bind_qp(&t, r0, 11u), UAF_OK);
        CHECK_EQ_I(uaf_key_validate(&t, r0, 11u, 0x900000u, 8u,
                                    UAF_MR_REMOTE_WRITE, 1), UAF_OK);
        CHECK_EQ_I(uaf_key_bind_qp(&t, r0, 0u), UAF_ERR_INVAL);
        CHECK_EQ_I(uaf_key_bind_qp(&t, 0x1234u, 5u), UAF_ERR_RKEY);
    }

    CASE("valid access inside bounds");
    CHECK_EQ_I(uaf_key_validate(&t, rk, 7u, 0x100000u, 4096u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_OK);
    CHECK_EQ_I(uaf_key_validate(&t, rk, 7u, 0x100800u, 8u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_OK);

    CASE("out-of-bounds is MR_FAULT");
    CHECK_EQ_I(uaf_key_validate(&t, rk, 7u, 0x100000u, 4097u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_ERR_MR_FAULT);
    CHECK_EQ_I(uaf_key_validate(&t, rk, 7u, 0x0FFFFFu, 8u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_ERR_MR_FAULT);
    CHECK_EQ_I(uaf_key_validate(&t, rk, 7u, 0x100FFCu, 8u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_ERR_MR_FAULT);

    CASE("access flags are enforced");
    CHECK_EQ_I(uaf_key_validate(&t, rk, 7u, 0x100000u, 8u,
                                UAF_MR_ATOMIC, 1), UAF_ERR_PERM);

    CASE("an rkey presented on the wrong QP fails");
    CHECK_EQ_I(uaf_key_validate(&t, rk, 9u, 0x100000u, 8u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_ERR_RKEY);

    CASE("unknown rkey fails and advances the failure counter");
    uaf_key_reset_failures(&t);
    CHECK_EQ_I(uaf_key_validate(&t, 0xA5A5A5A5u, 7u, 0x100000u, 8u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_ERR_RKEY);
    CHECK(!uaf_key_should_throttle(&t));

    CASE("authenticated brute force engages throttling");
    uaf_key_reset_failures(&t);
    for (unsigned i = 0; i < UAF_RKEY_FAIL_MAX; i++)
        (void)uaf_key_validate(&t, 0xDEAD0000u + i, 7u, 0x100000u, 8u,
                               UAF_MR_REMOTE_WRITE, 1);
    CHECK(uaf_key_should_throttle(&t));

    CASE("UNauthenticated failures never engage throttling");
    /* v2.1 moved the QP to UAF_QPS_ERR after 16 failures, so any host able to
     * reach the UDP port could kill a connection with 16 datagrams -- which
     * contradicted [R-5.7-001]. An off-path attacker must not even be able to
     * drive the responder into rate-limiting itself. */
    uaf_key_reset_failures(&t);
    for (unsigned i = 0; i < UAF_RKEY_FAIL_MAX * 4u; i++)
        (void)uaf_key_validate(&t, 0xBEEF0000u + i, 7u, 0x100000u, 8u,
                               UAF_MR_REMOTE_WRITE, 0);
    CHECK(!uaf_key_should_throttle(&t));

    CASE("a success clears the failure counter");
    CHECK_EQ_I(uaf_key_validate(&t, rk, 7u, 0x100000u, 8u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_OK);
    CHECK(!uaf_key_should_throttle(&t));

    CASE("deregistration invalidates the key");
    CHECK_EQ_I(uaf_key_deregister(&t, rk), UAF_OK);
    CHECK_EQ_I(uaf_key_validate(&t, rk, 7u, 0x100000u, 8u,
                                UAF_MR_REMOTE_WRITE, 1), UAF_ERR_RKEY);
    CHECK_EQ_I(uaf_key_deregister(&t, rk), UAF_ERR_RKEY);

    CASE("without a link authenticator, throttling is per source address");
    /* [R-11.2-002] permits a trusted fabric to run with no authenticator, in
     * which case [R-11.2-004]'s counter never advances and the 32-bit key space
     * is scannable in seconds again. */
    {
        struct uaf_src_throttle st;
        uaf_src_reset(&st);
        uint8_t a1[16] = {0}, a2[16] = {0};
        a1[15] = 1u; a2[15] = 2u;
        for (unsigned i = 0; i < UAF_RKEY_FAIL_MAX; i++)
            uaf_src_note_failure(&st, a1);
        CHECK(uaf_src_should_throttle(&st, a1));
        CHECK(!uaf_src_should_throttle(&st, a2));   /* other peers unaffected */
        uaf_src_note_failure(&st, a2);
        CHECK(!uaf_src_should_throttle(&st, a2));
        uaf_src_reset(&st);
        CHECK(!uaf_src_should_throttle(&st, a1));
    }

    uaf_key_table_fini(&t);
    TEST_MAIN_END("test_rkey");
}
