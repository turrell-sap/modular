#ifndef UAF_QP_STATE_H
#define UAF_QP_STATE_H
#include "uaf_types.h"
int      uaf_qp_transition_legal(enum uaf_qp_state from, enum uaf_qp_state to);
/* The required mask depends on the PATH, not only the profile: UAF-D declares
 * UAF_PROFILE_UAFR and its intra-host CXL path has no MTU and no UDP address
 * vector. v2.1.1's [R-9.5-003] and [R-10.1-002] disagreed about exactly this. */
uint32_t uaf_qp_required_mask(enum uaf_qp_state to, uint32_t path);
int      uaf_qp_modify_check(enum uaf_qp_state from, enum uaf_qp_state to,
                             uint32_t attr_mask, uint32_t attr_state,
                             uint32_t path);
int      uaf_qp_flush_status(int originating_error, int already_in_flight);
#endif
