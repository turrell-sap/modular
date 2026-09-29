/* Conformance C-6: ring conventions. Section 10.3. */
#include "uaf_test.h"
#include "uaf_ring.h"
#include "uaf_config.h"

int main(void)
{
    struct uaf_ring r;
    struct uaf_wr slots[256];

    CASE("non-power-of-two depth is rejected");
    CHECK_EQ_I(uaf_ring_init(&r, slots, 100u, sizeof(struct uaf_wr)),
               UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_ring_init(&r, slots, 0u, sizeof(struct uaf_wr)),
               UAF_ERR_INVAL);

    CASE("empty and full");
    CHECK_EQ_I(uaf_ring_init(&r, slots, 256u, sizeof(struct uaf_wr)), UAF_OK);
    CHECK(uaf_ring_empty(&r));
    CHECK(!uaf_ring_full(&r));
    CHECK_EQ_U(uaf_ring_count(&r), 0u);

    CASE("producer advances tail, consumer advances head");
    uint32_t idx = 0xFFFFFFFFu;
    void *s = uaf_ring_produce(&r, &idx);
    CHECK(s != NULL);
    CHECK_EQ_U(idx, 0u);
    uaf_ring_commit(&r);
    CHECK_EQ_U(uaf_ring_count(&r), 1u);
    CHECK_EQ_U(atomic_load(&r.tail), 1u);
    CHECK_EQ_U(atomic_load(&r.head), 0u);
    CHECK(uaf_ring_consume(&r) != NULL);
    uaf_ring_release(&r);
    CHECK(uaf_ring_empty(&r));

    CASE("fills to exactly depth and then refuses");
    CHECK_EQ_I(uaf_ring_init(&r, slots, 256u, sizeof(struct uaf_wr)), UAF_OK);
    for (uint32_t i = 0; i < 256u; i++) {
        void *p = uaf_ring_produce(&r, &idx);
        CHECK(p != NULL);
        CHECK(idx < 256u);
        /* every slot must land inside the array */
        CHECK((uint8_t *)p >= (uint8_t *)slots);
        CHECK((uint8_t *)p + sizeof(struct uaf_wr) <=
              (uint8_t *)slots + sizeof(slots));
        uaf_ring_commit(&r);
    }
    CHECK(uaf_ring_full(&r));
    CHECK(uaf_ring_produce(&r, &idx) == NULL);   /* BUSY, not an overwrite */

    CASE("index wrap stays inside the ring across 4x depth");
    CHECK_EQ_I(uaf_ring_init(&r, slots, 256u, sizeof(struct uaf_wr)), UAF_OK);
    for (uint32_t i = 0; i < 1024u; i++) {
        void *p = uaf_ring_produce(&r, &idx);
        CHECK(p != NULL);
        /* v2.0 wrapped a 256-entry send ring with % UAF_CQ_DEPTH_DEFAULT
         * (1024), writing up to 49152 bytes past the end. */
        CHECK(idx < 256u);
        CHECK((uint8_t *)p + sizeof(struct uaf_wr) <=
              (uint8_t *)slots + sizeof(slots));
        uaf_ring_commit(&r);
        (void)uaf_ring_consume(&r);
        uaf_ring_release(&r);
    }

    CASE("the two default depths are both powers of two and independent");
    CHECK(UAF_IS_POW2(UAF_SQ_DEPTH_DEFAULT));
    CHECK(UAF_IS_POW2(UAF_CQ_DEPTH_DEFAULT));
    CHECK(UAF_SQ_DEPTH_DEFAULT != UAF_CQ_DEPTH_DEFAULT);

    CASE("counter wrap at 2^32 does not corrupt occupancy");
    CHECK_EQ_I(uaf_ring_init(&r, slots, 256u, sizeof(struct uaf_wr)), UAF_OK);
    atomic_store(&r.head, 0xFFFFFFFEu);
    atomic_store(&r.tail, 0xFFFFFFFEu);
    CHECK(uaf_ring_empty(&r));
    for (int i = 0; i < 4; i++) { CHECK(uaf_ring_produce(&r, &idx) != NULL);
                                  uaf_ring_commit(&r); }
    CHECK_EQ_U(uaf_ring_count(&r), 4u);       /* tail wrapped past zero */

    TEST_MAIN_END("test_ring");
}
