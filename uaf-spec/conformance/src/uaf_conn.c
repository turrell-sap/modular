#include <string.h>
#include "uaf_wire.h"

int uaf_conn_info_serialize(const struct uaf_conn_info *ci,
                            uint8_t out[UAF_CONN_WIRE_SIZE])
{
    if (!ci || !out) return UAF_ERR_INVAL;
    if (ci->struct_size != sizeof(*ci)) return UAF_ERR_INVAL;
    memset(out, 0, UAF_CONN_WIRE_SIZE);
    uaf_put_be32(out + UAF_CW_QP_NUM,    ci->qp_num);
    uaf_put_be32(out + UAF_CW_PSN,       ci->psn);
    uaf_put_be32(out + UAF_CW_RKEY,      ci->rkey);
    uaf_put_be16(out + UAF_CW_LID,       ci->lid);
    uaf_put_be16(out + UAF_CW_MTU,       ci->mtu);
    uaf_put_be64(out + UAF_CW_REMOTE_VA, ci->remote_va);
    uaf_put_be64(out + UAF_CW_CXL_BASE,  ci->cxl_base);
    uaf_put_be64(out + UAF_CW_CXL_SIZE,  ci->cxl_size);
    memcpy(out + UAF_CW_GID, ci->gid, 16);
    uaf_put_u8(out + UAF_CW_SL,      ci->sl);
    uaf_put_u8(out + UAF_CW_TCLASS,  ci->traffic_class);
    uaf_put_u8(out + UAF_CW_PROFILE, ci->wire_profile);
    uaf_put_u8(out + UAF_CW_PATH,    ci->path);
    return UAF_OK;
}

int uaf_conn_info_deserialize(const uint8_t in[UAF_CONN_WIRE_SIZE],
                              struct uaf_conn_info *ci)
{
    if (!in || !ci) return UAF_ERR_INVAL;
    for (int i = 0; i < 4; i++)
        if (in[UAF_CW_RESERVED + i]) return UAF_ERR_PROTO;
    memset(ci, 0, sizeof(*ci));
    ci->struct_size   = (uint32_t)sizeof(*ci);
    ci->qp_num        = uaf_get_be32(in + UAF_CW_QP_NUM);
    ci->psn           = uaf_get_be32(in + UAF_CW_PSN);
    ci->rkey          = uaf_get_be32(in + UAF_CW_RKEY);
    ci->lid           = uaf_get_be16(in + UAF_CW_LID);
    ci->mtu           = uaf_get_be16(in + UAF_CW_MTU);
    ci->remote_va     = uaf_get_be64(in + UAF_CW_REMOTE_VA);
    ci->cxl_base      = uaf_get_be64(in + UAF_CW_CXL_BASE);
    ci->cxl_size      = uaf_get_be64(in + UAF_CW_CXL_SIZE);
    memcpy(ci->gid, in + UAF_CW_GID, 16);
    ci->sl            = uaf_get_u8(in + UAF_CW_SL);
    ci->traffic_class = uaf_get_u8(in + UAF_CW_TCLASS);
    ci->wire_profile  = uaf_get_u8(in + UAF_CW_PROFILE);
    ci->path          = uaf_get_u8(in + UAF_CW_PATH);

    /* [R-5.5-003] A profile bit MUST name exactly one profile. */
    if (ci->wire_profile != UAF_PROFILE_IBV &&
        ci->wire_profile != UAF_PROFILE_UAFR) return UAF_ERR_PROTO;
    /* [R-5.5-012] The path SHALL be one of the three defined values and SHALL
     * be consistent with the profile. UAF-D declares UAF_PROFILE_UAFR for
     * node-to-node traffic and also runs an intra-host CXL path, so the profile
     * alone does not say which data plane a queue pair uses. */
    switch (ci->path) {
    case UAF_PATH_IBV:
        if (ci->wire_profile != UAF_PROFILE_IBV) return UAF_ERR_PROTO;
        break;
    case UAF_PATH_UDP:
    case UAF_PATH_CXL:
        if (ci->wire_profile != UAF_PROFILE_UAFR) return UAF_ERR_PROTO;
        break;
    default:
        return UAF_ERR_PROTO;
    }
    /* [R-5.5-004] A packet-carrying path needs room for the header. The CXL
     * path carries no packet and has no MTU, so the floor does not apply. */
    if (ci->path != UAF_PATH_CXL && ci->mtu <= UAF_WIRE_HDR_SIZE)
        return UAF_ERR_PROTO;
    return UAF_OK;
}
