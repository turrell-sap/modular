/* Queue-pair state machine, Section 10.1. NORMATIVE.
 *
 * v2.0's table omitted UAF_QPS_SQE entirely, had no attribute mask, and
 * demanded a GID and a LID at RTR from backends (CXL, UDP) that have
 * neither. */
#include "uaf_qp_state.h"

int uaf_qp_transition_legal(enum uaf_qp_state from, enum uaf_qp_state to)
{
    if (to == UAF_QPS_ERR)   return 1;             /* from any state       */
    if (to == UAF_QPS_RESET) return 1;             /* from any state       */
    switch (from) {
    case UAF_QPS_RESET: return to == UAF_QPS_INIT;
    case UAF_QPS_INIT:  return to == UAF_QPS_RTR;
    case UAF_QPS_RTR:   return to == UAF_QPS_RTS;
    case UAF_QPS_RTS:   return to == UAF_QPS_SQD;
    case UAF_QPS_SQD:   return to == UAF_QPS_RTS;
    /* [R-10.1-005] SQE is entered by the implementation on a send-queue
     * error and is left only through RESET or ERR. v2.0 declared the state
     * and gave it no edges at all. */
    case UAF_QPS_SQE:   return 0;
    case UAF_QPS_ERR:   return 0;                  /* RESET handled above  */
    default:            return 0;
    }
}

uint32_t uaf_qp_required_mask(enum uaf_qp_state to, uint32_t path)
{
    switch (to) {
    case UAF_QPS_INIT:
        return UAF_QP_STATE | UAF_QP_PORT;
    case UAF_QPS_RTR: {
        /* [R-10.1-006] rq_psn is required at RTR; v2.0's table never asked
         * for it. [R-10.1-008] the address vector and the MTU requirement
         * depend on the path. */
        uint32_t m = UAF_QP_STATE | UAF_QP_RQ_PSN | UAF_QP_DEST_QPN |
                     UAF_QP_MAX_DEST_RD_AT;
        switch (path) {
        case UAF_PATH_IBV:
            m |= UAF_QP_PATH_MTU | UAF_QP_AV_GID | UAF_QP_AV_LID;
            break;
        case UAF_PATH_UDP:
            m |= UAF_QP_PATH_MTU | UAF_QP_AV_UDP;
            break;
        case UAF_PATH_CXL:
            /* No MTU: the CXL.mem path carries no packet. */
            m |= UAF_QP_AV_CXL;
            break;
        default:
            break;
        }
        return m;
    }
    case UAF_QPS_RTS:
        return UAF_QP_STATE | UAF_QP_SQ_PSN | UAF_QP_MAX_RD_ATOMIC |
               UAF_QP_RETRY | UAF_QP_RNR_RETRY | UAF_QP_TIMEOUT;
    case UAF_QPS_SQD:
    case UAF_QPS_ERR:
    case UAF_QPS_RESET:
        return UAF_QP_STATE;
    default:
        return UAF_QP_STATE;
    }
}

int uaf_qp_modify_check(enum uaf_qp_state from, enum uaf_qp_state to,
                        uint32_t attr_mask, uint32_t attr_state,
                        uint32_t path)
{
    if (!uaf_qp_transition_legal(from, to)) return UAF_ERR_QP_STATE;
    /* [R-4.9-003] attr->qp_state MUST agree with the state argument. */
    if (attr_state != (uint32_t)to) return UAF_ERR_INVAL;
    uint32_t need = uaf_qp_required_mask(to, path);
    if ((attr_mask & need) != need) return UAF_ERR_INVAL;
    return UAF_OK;
}

/* [R-10.1-007] A work request already in flight when a fault occurs
 * completes with the ORIGINATING error. Only requests flushed after the
 * transition report UAF_ERR_QP_STATE. v2.0 said every fault flushed with
 * UAF_ERR_QP_STATE, which discarded the timeout and CRC status the rest of
 * the specification takes care to produce. */
int uaf_qp_flush_status(int originating_error, int already_in_flight)
{
    return already_in_flight ? originating_error : UAF_ERR_QP_STATE;
}
