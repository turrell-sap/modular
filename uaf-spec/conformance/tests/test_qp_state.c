/* Conformance C-4: QP state machine and attribute masks. Section 10.1. */
#include "uaf_test.h"
#include "uaf_qp_state.h"

int main(void)
{
    CASE("legal transitions");
    CHECK(uaf_qp_transition_legal(UAF_QPS_RESET, UAF_QPS_INIT));
    CHECK(uaf_qp_transition_legal(UAF_QPS_INIT,  UAF_QPS_RTR));
    CHECK(uaf_qp_transition_legal(UAF_QPS_RTR,   UAF_QPS_RTS));
    CHECK(uaf_qp_transition_legal(UAF_QPS_RTS,   UAF_QPS_SQD));
    CHECK(uaf_qp_transition_legal(UAF_QPS_SQD,   UAF_QPS_RTS));
    CHECK(uaf_qp_transition_legal(UAF_QPS_ERR,   UAF_QPS_RESET));

    CASE("any state may fault to ERR");
    const enum uaf_qp_state all[] = { UAF_QPS_RESET, UAF_QPS_INIT, UAF_QPS_RTR,
                                      UAF_QPS_RTS, UAF_QPS_SQD, UAF_QPS_SQE };
    for (unsigned i = 0; i < sizeof(all)/sizeof(all[0]); i++)
        CHECK(uaf_qp_transition_legal(all[i], UAF_QPS_ERR));

    CASE("illegal transitions are rejected");
    CHECK(!uaf_qp_transition_legal(UAF_QPS_RESET, UAF_QPS_RTR));
    CHECK(!uaf_qp_transition_legal(UAF_QPS_RESET, UAF_QPS_RTS));
    CHECK(!uaf_qp_transition_legal(UAF_QPS_INIT,  UAF_QPS_RTS));
    CHECK(!uaf_qp_transition_legal(UAF_QPS_RTR,   UAF_QPS_SQD));
    CHECK(!uaf_qp_transition_legal(UAF_QPS_ERR,   UAF_QPS_RTS));

    CASE("SQE is reachable only by fault and leaves only via RESET or ERR");
    CHECK(uaf_qp_transition_legal(UAF_QPS_SQE, UAF_QPS_RESET));
    CHECK(uaf_qp_transition_legal(UAF_QPS_SQE, UAF_QPS_ERR));
    CHECK(!uaf_qp_transition_legal(UAF_QPS_SQE, UAF_QPS_RTS));

    CASE("RTR requires rq_psn, and an address vector matching the profile");
    uint32_t m_ibv  = uaf_qp_required_mask(UAF_QPS_RTR, UAF_PROFILE_IBV);
    uint32_t m_uafr = uaf_qp_required_mask(UAF_QPS_RTR, UAF_PROFILE_UAFR);
    CHECK(m_ibv  & UAF_QP_RQ_PSN);
    CHECK(m_uafr & UAF_QP_RQ_PSN);
    CHECK(m_ibv  & UAF_QP_AV_GID);
    CHECK(m_ibv  & UAF_QP_AV_LID);
    /* A UDP or CXL backend has no GID and no LID; v2.0 demanded both. */
    CHECK(!(m_uafr & UAF_QP_AV_GID));
    CHECK(!(m_uafr & UAF_QP_AV_LID));
    CHECK(m_uafr & UAF_QP_AV_UDP);

    CASE("modify rejects a missing required attribute");
    CHECK_EQ_I(uaf_qp_modify_check(UAF_QPS_INIT, UAF_QPS_RTR,
                                   UAF_QP_STATE, UAF_QPS_RTR,
                                   UAF_PROFILE_UAFR), UAF_ERR_INVAL);
    CHECK_EQ_I(uaf_qp_modify_check(UAF_QPS_INIT, UAF_QPS_RTR,
                                   m_uafr, UAF_QPS_RTR,
                                   UAF_PROFILE_UAFR), UAF_OK);

    CASE("modify rejects attr->qp_state disagreeing with the argument");
    CHECK_EQ_I(uaf_qp_modify_check(UAF_QPS_INIT, UAF_QPS_RTR,
                                   m_uafr, UAF_QPS_RTS,
                                   UAF_PROFILE_UAFR), UAF_ERR_INVAL);

    CASE("illegal transition reports QP_STATE, not INVAL");
    CHECK_EQ_I(uaf_qp_modify_check(UAF_QPS_RESET, UAF_QPS_RTS,
                                   0xFFFFFFFFu, UAF_QPS_RTS,
                                   UAF_PROFILE_UAFR), UAF_ERR_QP_STATE);

    CASE("in-flight work keeps its originating status");
    CHECK_EQ_I(uaf_qp_flush_status(UAF_ERR_TIMEOUT, 1), UAF_ERR_TIMEOUT);
    CHECK_EQ_I(uaf_qp_flush_status(UAF_ERR_CRC, 1),     UAF_ERR_CRC);
    CHECK_EQ_I(uaf_qp_flush_status(UAF_ERR_TIMEOUT, 0), UAF_ERR_QP_STATE);

    TEST_MAIN_END("test_qp_state");
}
