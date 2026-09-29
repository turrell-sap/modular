/* Conformance C-9: connection exchange serialisation. Section 5.5. */
#include "uaf_test.h"
#include "uaf_wire.h"

int main(void)
{
    struct uaf_conn_info ci, out;
    uint8_t w[UAF_CONN_WIRE_SIZE];

    memset(&ci, 0, sizeof(ci));
    ci.struct_size  = sizeof(ci);
    ci.qp_num       = 0x01020304u;
    ci.psn          = 0x00ABCDEFu;
    ci.rkey         = 0xCAFEBABEu;
    ci.remote_va    = 0x0000100000002000ull;
    ci.cxl_base     = 0x0000200000000000ull;
    ci.cxl_size     = 0x0000000040000000ull;
    for (int i = 0; i < 16; i++) ci.gid[i] = (uint8_t)(0xE0 + i);
    ci.lid          = 0x1234u;
    ci.mtu          = 4096u;
    ci.sl           = 3u;
    ci.traffic_class = 7u;
    ci.wire_profile = UAF_PROFILE_UAFR;
    ci.path         = UAF_PATH_UDP;

    CASE("round trip");
    CHECK_EQ_I(uaf_conn_info_serialize(&ci, w), UAF_OK);
    CHECK_EQ_I(uaf_conn_info_deserialize(w, &out), UAF_OK);
    CHECK_EQ_U(out.qp_num, ci.qp_num);
    CHECK_EQ_U(out.psn, ci.psn);
    CHECK_EQ_U(out.rkey, ci.rkey);
    CHECK_EQ_U(out.remote_va, ci.remote_va);
    CHECK_EQ_U(out.cxl_base, ci.cxl_base);
    CHECK_EQ_U(out.cxl_size, ci.cxl_size);
    CHECK_EQ_U(out.lid, ci.lid);
    CHECK_EQ_U(out.mtu, ci.mtu);
    CHECK_EQ_U(out.sl, ci.sl);
    CHECK_EQ_U(out.traffic_class, ci.traffic_class);
    CHECK_EQ_U(out.wire_profile, ci.wire_profile);
    CHECK_EQ_U(out.path, ci.path);
    CHECK_EQ_I(memcmp(out.gid, ci.gid, 16), 0);

    CASE("the wire form is big-endian with no holes");
    CHECK_EQ_U(w[UAF_CW_QP_NUM + 0], 0x01);
    CHECK_EQ_U(w[UAF_CW_QP_NUM + 3], 0x04);
    CHECK_EQ_U(w[UAF_CW_RKEY + 0], 0xCA);
    CHECK_EQ_U(w[UAF_CW_RKEY + 3], 0xBE);
    CHECK_EQ_U(w[UAF_CW_MTU + 0], 0x10);      /* 4096 = 0x1000 */
    CHECK_EQ_U(w[UAF_CW_MTU + 1], 0x00);
    CHECK_EQ_U(w[UAF_CW_PATH], UAF_PATH_UDP);
    for (int i = 0; i < 4; i++) CHECK_EQ_U(w[UAF_CW_RESERVED + i], 0u);

    CASE("a host struct memcpy would NOT have been portable");
    /* uaf_conn_info is 64 bytes of host-order fields with interior padding;
     * the wire form is 64 bytes of big-endian fields with none. They are not
     * the same bytes, which is exactly why v2.0's implied memcpy was wrong. */
    CHECK(memcmp(w, &ci, UAF_CONN_WIRE_SIZE) != 0);

    CASE("no file descriptor can reach the wire");
    /* v2.0 carried dma_buf_fd in uaf_conn_info. A descriptor is a local
     * integer: copied to another node it is meaningless, and interpreted
     * locally it is a confused deputy. It now lives only in uaf_peer_info. */
    CHECK_EQ_U(sizeof(struct uaf_conn_info), 64u);
    struct uaf_peer_info pi;
    memset(&pi, 0, sizeof(pi));
    pi.struct_size = sizeof(pi);
    pi.dma_buf_fd = -1;
    CHECK_EQ_I(pi.dma_buf_fd, -1);   /* same-host only, via SCM_RIGHTS */

    CASE("non-zero reserved bytes are rejected");
    CHECK_EQ_I(uaf_conn_info_serialize(&ci, w), UAF_OK);
    w[UAF_CW_RESERVED + 2] = 0x01u;
    CHECK_EQ_I(uaf_conn_info_deserialize(w, &out), UAF_ERR_PROTO);

    CASE("an ambiguous or absent profile is rejected");
    CHECK_EQ_I(uaf_conn_info_serialize(&ci, w), UAF_OK);
    w[UAF_CW_PROFILE] = (uint8_t)(UAF_PROFILE_IBV | UAF_PROFILE_UAFR);
    CHECK_EQ_I(uaf_conn_info_deserialize(w, &out), UAF_ERR_PROTO);
    w[UAF_CW_PROFILE] = 0u;
    CHECK_EQ_I(uaf_conn_info_deserialize(w, &out), UAF_ERR_PROTO);

    CASE("an MTU with no room for the header is rejected on a packet path");
    ci.mtu = 64u;
    CHECK_EQ_I(uaf_conn_info_serialize(&ci, w), UAF_OK);
    CHECK_EQ_I(uaf_conn_info_deserialize(w, &out), UAF_ERR_PROTO);

    CASE("the CXL path has no MTU, so the floor does not apply to it");
    ci.path = UAF_PATH_CXL; ci.mtu = 0u;
    CHECK_EQ_I(uaf_conn_info_serialize(&ci, w), UAF_OK);
    CHECK_EQ_I(uaf_conn_info_deserialize(w, &out), UAF_OK);
    CHECK_EQ_U(out.path, UAF_PATH_CXL);

    CASE("a path inconsistent with the profile is rejected");
    ci.path = UAF_PATH_IBV;                 /* IBV path, UAFR profile */
    CHECK_EQ_I(uaf_conn_info_serialize(&ci, w), UAF_OK);
    CHECK_EQ_I(uaf_conn_info_deserialize(w, &out), UAF_ERR_PROTO);
    ci.path = 0u;                            /* undefined */
    CHECK_EQ_I(uaf_conn_info_serialize(&ci, w), UAF_OK);
    CHECK_EQ_I(uaf_conn_info_deserialize(w, &out), UAF_ERR_PROTO);
    ci.path = UAF_PATH_UDP; ci.mtu = 4096u;

    CASE("struct_size must be set by the caller");
    ci.mtu = 4096u; ci.struct_size = 0u;
    CHECK_EQ_I(uaf_conn_info_serialize(&ci, w), UAF_ERR_INVAL);

    TEST_MAIN_END("test_conn_wire");
}
