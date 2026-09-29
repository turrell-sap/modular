#ifndef UAF_QP_STATE_H
#define UAF_QP_STATE_H
#include "uaf_types.h"
int      uaf_qp_transition_legal(enum uaf_qp_state from, enum uaf_qp_state to);
uint32_t uaf_qp_required_mask(enum uaf_qp_state to, uint32_t profile);
int      uaf_qp_modify_check(enum uaf_qp_state from, enum uaf_qp_state to,
                             uint32_t attr_mask, uint32_t attr_state,
                             uint32_t profile);
int      uaf_qp_flush_status(int originating_error, int already_in_flight);
#endif
