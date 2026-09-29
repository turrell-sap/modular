#include <string.h>
#include "uaf_wire.h"

/* [R-5.3-004] UAF_WR_RECV is a LOCAL buffer post and MUST NOT appear as a
 * wire opcode. v2.0's opcode table listed it as a packet with an empty
 * payload, which gave implementers a packet to build that has no meaning. */
static int wire_opcode_valid(uint8_t op)
{
    switch (op) {
    case UAF_WR_SEND: case UAF_WR_SEND_INLINE:
    case UAF_WR_RDMA_READ: case UAF_WR_RDMA_WRITE:
    case UAF_WR_RDMA_WRITE_IMM:
    case UAF_WR_ATOMIC_CMP_SWP: case UAF_WR_ATOMIC_FETCH_ADD:
    case UAF_OP_RDMA_READ_RESP: case UAF_OP_ATOMIC_ACK:
    case UAF_OP_CM_REQ: case UAF_OP_CM_REP:
    case UAF_OP_CM_RTU: case UAF_OP_CM_REJ:
    case UAF_OP_ACK:
    case UAF_OP_NAK_SEQ: case UAF_OP_NAK_RNR: case UAF_OP_NAK_INVAL:
        return 1;
    case UAF_WR_RECV:
    default:
        return 0;
    }
}

int uaf_wire_hdr_encode(const struct uaf_wire_hdr *h, uint8_t out[64],
                        const void *payload)
{
    if (!h || !out) return UAF_ERR_INVAL;
    if (!wire_opcode_valid(h->opcode)) return UAF_ERR_INVAL;
    if (h->flags & UAF_WIRE_FLAG_RESERVED) return UAF_ERR_INVAL;
    if ((uint64_t)h->seg_off + h->seg_len > (uint64_t)h->msg_len)
        return UAF_ERR_INVAL;

    memset(out, 0, UAF_WIRE_HDR_SIZE);
    uaf_put_be32(out + UAF_H_MAGIC,     UAF_WIRE_MAGIC);
    uaf_put_u8  (out + UAF_H_VERSION,   UAF_WIRE_VERSION);
    uaf_put_u8  (out + UAF_H_OPCODE,    h->opcode);
    uaf_put_be16(out + UAF_H_FLAGS,     h->flags);
    uaf_put_be32(out + UAF_H_QP_ID,     h->qp_id);
    uaf_put_be32(out + UAF_H_PSN,       h->psn);
    uaf_put_be32(out + UAF_H_MSG_ID,    h->msg_id);
    uaf_put_be32(out + UAF_H_RKEY,      h->rkey);
    uaf_put_be64(out + UAF_H_REMOTE_VA, h->remote_va);
    uaf_put_be32(out + UAF_H_MSG_LEN,   h->msg_len);
    uaf_put_be32(out + UAF_H_SEG_OFF,   h->seg_off);
    uaf_put_be32(out + UAF_H_SEG_LEN,   h->seg_len);
    uaf_put_be32(out + UAF_H_IMM_DATA,  h->imm_data);

    if ((h->flags & UAF_WIRE_FLAG_DATA_CRC) && h->seg_len) {
        if (!payload) return UAF_ERR_INVAL;
        uaf_put_be32(out + UAF_H_DATA_CRC,
                     UAF_CRC32C(payload, h->seg_len));
    }
    uaf_put_be32(out + UAF_H_HDR_CRC, UAF_CRC32C(out, UAF_HDR_CRC_COVER));
    return UAF_OK;
}

int uaf_wire_hdr_decode(const uint8_t in[64], struct uaf_wire_hdr *h)
{
    if (!in || !h) return UAF_ERR_INVAL;

    /* [R-5.7-001] A bad magic or an unknown version MUST be dropped without
     * touching QP state: an off-fabric packet cannot be allowed to move a
     * connection to ERR. */
    if (uaf_get_be32(in + UAF_H_MAGIC) != UAF_WIRE_MAGIC) return UAF_ERR_PROTO;
    if (uaf_get_u8(in + UAF_H_VERSION) != UAF_WIRE_VERSION) return UAF_ERR_PROTO;

    /* [R-5.7-002] A header CRC mismatch MUST be reported as UAF_ERR_CRC and
     * the packet dropped; it MUST NOT be treated as a sequence error. */
    uint32_t want = uaf_get_be32(in + UAF_H_HDR_CRC);
    if (UAF_CRC32C(in, UAF_HDR_CRC_COVER) != want) return UAF_ERR_CRC;

    for (int i = 0; i < 8; i++)
        if (in[UAF_H_RESERVED + i]) return UAF_ERR_PROTO;

    h->magic       = UAF_WIRE_MAGIC;
    h->version     = UAF_WIRE_VERSION;
    h->opcode      = uaf_get_u8(in + UAF_H_OPCODE);
    h->flags       = uaf_get_be16(in + UAF_H_FLAGS);
    h->qp_id       = uaf_get_be32(in + UAF_H_QP_ID);
    h->psn         = uaf_get_be32(in + UAF_H_PSN);
    h->msg_id      = uaf_get_be32(in + UAF_H_MSG_ID);
    h->rkey        = uaf_get_be32(in + UAF_H_RKEY);
    h->remote_va   = uaf_get_be64(in + UAF_H_REMOTE_VA);
    h->msg_len     = uaf_get_be32(in + UAF_H_MSG_LEN);
    h->seg_off     = uaf_get_be32(in + UAF_H_SEG_OFF);
    h->seg_len     = uaf_get_be32(in + UAF_H_SEG_LEN);
    h->imm_data    = uaf_get_be32(in + UAF_H_IMM_DATA);
    h->hdr_crc32c  = want;
    h->data_crc32c = uaf_get_be32(in + UAF_H_DATA_CRC);

    if (!wire_opcode_valid((uint8_t)h->opcode))  return UAF_ERR_PROTO;
    if (h->flags & UAF_WIRE_FLAG_RESERVED)       return UAF_ERR_PROTO;
    if ((uint64_t)h->seg_off + h->seg_len > (uint64_t)h->msg_len)
        return UAF_ERR_PROTO;

    /* [R-5.4-002] Atomic payload lengths are fixed and remote_va must be
     * 8-byte aligned. */
    if (h->opcode == UAF_WR_ATOMIC_CMP_SWP &&
        h->seg_len != UAF_ATOMIC_CMP_SWP_LEN) return UAF_ERR_PROTO;
    if (h->opcode == UAF_WR_ATOMIC_FETCH_ADD &&
        h->seg_len != UAF_ATOMIC_FETCH_ADD_LEN) return UAF_ERR_PROTO;
    if (h->opcode == UAF_OP_ATOMIC_ACK &&
        h->seg_len != UAF_ATOMIC_ACK_LEN) return UAF_ERR_PROTO;
    if ((h->opcode == UAF_WR_ATOMIC_CMP_SWP ||
         h->opcode == UAF_WR_ATOMIC_FETCH_ADD) &&
        (h->remote_va & (UAF_ATOMIC_ALIGN - 1u))) return UAF_ERR_INVAL;

    return UAF_OK;
}

int uaf_wire_check_payload(const struct uaf_wire_hdr *h, const void *payload)
{
    if (!h) return UAF_ERR_INVAL;
    if (!(h->flags & UAF_WIRE_FLAG_DATA_CRC) || h->seg_len == 0) return UAF_OK;
    if (!payload) return UAF_ERR_INVAL;
    return (UAF_CRC32C(payload, h->seg_len) == h->data_crc32c)
           ? UAF_OK : UAF_ERR_CRC;
}

uint32_t uaf_wire_seg_count(uint32_t msg_len, uint32_t mtu)
{
    uint32_t seg = UAF_WIRE_MAX_SEG(mtu);
    if (seg == 0u) return 0u;
    if (msg_len == 0u) return 1u;   /* a zero-length SEND is one packet */
    return (msg_len + seg - 1u) / seg;
}

int uaf_wire_segment(struct uaf_wire_hdr *h, uint32_t msg_len, uint32_t mtu,
                     uint32_t idx)
{
    if (!h || mtu <= UAF_WIRE_HDR_SIZE) return UAF_ERR_INVAL;
    uint32_t seg = UAF_WIRE_MAX_SEG(mtu);
    if (idx >= uaf_wire_seg_count(msg_len, mtu)) return UAF_ERR_INVAL;
    uint64_t off = (uint64_t)idx * seg;
    uint32_t len = (msg_len - (uint32_t)off < seg)
                   ? (msg_len - (uint32_t)off) : seg;
    h->msg_len = msg_len;
    h->seg_off = (uint32_t)off;
    h->seg_len = len;
    return UAF_OK;
}
