/* Conformance C-1: ABI layout is pinned. Section 4.1. */
#include "uaf_test.h"
#include "uaf_types.h"
#include "uaf_core.h"   /* the public header must compile under -Werror */
#include "uaf_wire.h"
#include "uaf_dst.h"
#include "uaf_rmt_cq.h"
#include <stddef.h>

int main(void)
{
    CASE("struct sizes");
    CHECK_EQ_U(sizeof(uaf_mr_t), 48);
    CHECK_EQ_U(sizeof(struct uaf_sge), 16);
    CHECK_EQ_U(sizeof(struct uaf_wr), 64);
    CHECK_EQ_U(sizeof(struct uaf_wc), 40);
    CHECK_EQ_U(sizeof(struct uaf_qp_init_attr), 40);
    CHECK_EQ_U(sizeof(struct uaf_qp_attr), 80);
    CHECK_EQ_U(sizeof(struct uaf_device_attr), 64);
    CHECK_EQ_U(sizeof(struct uaf_conn_info), 64);
    CHECK_EQ_U(sizeof(struct uaf_peer_info), 40);
    CHECK_EQ_U(sizeof(struct uaf_storage_cmd), 56);
    CHECK_EQ_U(sizeof(struct uaf_storage_cqe), 16);
    CHECK_EQ_U(sizeof(struct uaf_storage_wc), 24);
    CHECK_EQ_U(sizeof(struct uaf_rmt_cqe), 32);

    CASE("CQE is not packed and phase is the last byte");
    CHECK_EQ_U(_Alignof(struct uaf_storage_cqe), 4);
    CHECK_EQ_U(offsetof(struct uaf_storage_cqe, phase), 15);
    CHECK_EQ_U(offsetof(struct uaf_storage_cqe, cmd_id), 0);
    CHECK_EQ_U(offsetof(struct uaf_storage_cqe, status), 2);
    CHECK_EQ_U(offsetof(struct uaf_storage_cqe, bytes_transferred), 4);
    CHECK_EQ_U(offsetof(struct uaf_storage_cqe, latency_ns), 8);

    CASE("the intra-host RMT completion has a layout, with phase last");
    /* v2.1.1 told the agent to release-store "the completion" and named no
     * object: uaf_wc is 40 bytes, host-side, and has no phase byte. */
    CHECK_EQ_U(offsetof(struct uaf_rmt_cqe, wr_id), 0);
    CHECK_EQ_U(offsetof(struct uaf_rmt_cqe, phase), 31);
    CHECK_EQ_U(_Alignof(struct uaf_rmt_cqe), 8);
    CHECK_EQ_U(UAF_RMT_CQE_ALIGN, 32);

    CASE("a path discriminant exists in both the init attr and the record");
    CHECK_EQ_U(sizeof(((struct uaf_qp_init_attr *)0)->path), 4);
    CHECK_EQ_U(sizeof(((struct uaf_conn_info *)0)->path), 1);
    CHECK(UAF_PATH_IBV != UAF_PATH_UDP);
    CHECK(UAF_PATH_UDP != UAF_PATH_CXL);

    CASE("the public storage completion carries a full 64-bit wr_id");
    /* v2.1 required the library to restore the caller's wr_id while giving
     * uaf_storage_poll() only the 16-byte ring image, whose sole identifier is
     * a 16-bit cmd_id. The requirement had nowhere to land. */
    CHECK_EQ_U(sizeof(((struct uaf_storage_wc *)0)->wr_id), 8);
    CHECK_EQ_U(offsetof(struct uaf_storage_wc, wr_id), 0);

    CASE("no size_t or pointer-width field in a pinned struct");
    /* uaf_storage_cmd.buf_len was size_t in v2.0: ILP32 vs LP64 divergence. */
    CHECK_EQ_U(sizeof(((struct uaf_storage_cmd *)0)->buf_len), 8);
    CHECK_EQ_U(sizeof(((struct uaf_mr *)0)->length), 8);

    CASE("mr does not publish backend private state");
    /* v2.0 had uaf_mr.pal_priv in the application ABI. 48 bytes leaves no
     * room for it; this check fails loudly if anyone adds it back. */
    CHECK_EQ_U(offsetof(uaf_mr_t, flags) + sizeof(uint64_t), sizeof(uaf_mr_t));

    CASE("wire header offsets");
    CHECK_EQ_U(UAF_WIRE_HDR_SIZE, 64);
    CHECK_EQ_U(UAF_H_MAGIC, 0);   CHECK_EQ_U(UAF_H_VERSION, 4);
    CHECK_EQ_U(UAF_H_OPCODE, 5);  CHECK_EQ_U(UAF_H_FLAGS, 6);
    CHECK_EQ_U(UAF_H_QP_ID, 8);   CHECK_EQ_U(UAF_H_PSN, 12);
    CHECK_EQ_U(UAF_H_MSG_ID, 16); CHECK_EQ_U(UAF_H_RKEY, 20);
    CHECK_EQ_U(UAF_H_REMOTE_VA, 24);
    CHECK_EQ_U(UAF_H_REMOTE_VA % 8, 0);   /* 8-byte field, 8-byte offset */
    CHECK_EQ_U(UAF_H_MSG_LEN, 32); CHECK_EQ_U(UAF_H_SEG_OFF, 36);
    CHECK_EQ_U(UAF_H_SEG_LEN, 40); CHECK_EQ_U(UAF_H_IMM_DATA, 44);
    CHECK_EQ_U(UAF_H_HDR_CRC, 48); CHECK_EQ_U(UAF_H_DATA_CRC, 52);
    CHECK_EQ_U(UAF_H_RESERVED, 56);
    CHECK_EQ_U(UAF_H_RESERVED + 8, UAF_WIRE_HDR_SIZE);

    CASE("MTU includes the header");
    /* v2.0 defined MTU as payload only, so a maximum packet was 4160 bytes
     * and did not fit a 4 KiB path MTU. */
    CHECK_EQ_U(UAF_WIRE_MAX_SEG(4096), 4096 - 64);
    CHECK_EQ_U(UAF_WIRE_SEG_DEFAULT, 4032);

    CASE("CM packet lengths and the endpoint identity");
    CHECK_EQ_U(UAF_CM_RECORD_SIZE, 64);
    CHECK_EQ_U(UAF_CM_AUTH_SIZE, 32);
    CHECK_EQ_U(UAF_CM_REQ_LEN, 96);
    CHECK_EQ_U(UAF_CM_RTU_LEN, 32);   /* authenticator only, not 96 */
    CHECK_EQ_U(UAF_EID_SIZE, 22);

    CASE("the reserved flag mask leaves room for ECN echo");
    CHECK_EQ_U(UAF_WIRE_FLAG_ECN_ECHO, 1u << 6);
    CHECK_EQ_U(UAF_WIRE_FLAG_RESERVED, 0xFF80u);
    CHECK_EQ_U(UAF_WIRE_FLAG_ECN_ECHO & UAF_WIRE_FLAG_RESERVED, 0u);

    CASE("connection wire form is 64 bytes with no holes");
    CHECK_EQ_U(UAF_CONN_WIRE_SIZE, 64);
    CHECK_EQ_U(UAF_CW_PATH, 59);
    CHECK_EQ_U(UAF_CW_RESERVED + 4, UAF_CONN_WIRE_SIZE);

    TEST_MAIN_END("test_abi");
}
