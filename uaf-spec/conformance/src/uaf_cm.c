/* Connection-manager helpers: packet sizing and the simultaneous-open
 * tie-break of Section 5.5. */
#include <string.h>
#include "uaf_wire.h"

uint32_t uaf_cm_expected_len(uint8_t opcode)
{
    switch (opcode) {
    case UAF_OP_CM_REQ: return UAF_CM_REQ_LEN;   /* record + authenticator */
    case UAF_OP_CM_REP: return UAF_CM_REP_LEN;
    case UAF_OP_CM_RTU: return UAF_CM_RTU_LEN;   /* authenticator only     */
    case UAF_OP_CM_REJ: return UAF_CM_REJ_LEN;
    default:            return 0u;
    }
}

void uaf_eid_serialize(const struct uaf_endpoint_id *e,
                       uint8_t out[UAF_EID_SIZE])
{
    memcpy(out, e->addr, 16);
    uaf_put_be16(out + 16, e->udp_port);
    uaf_put_be32(out + 18, e->qp_num);
}

int uaf_eid_compare(const struct uaf_endpoint_id *a,
                    const struct uaf_endpoint_id *b)
{
    uint8_t ba[UAF_EID_SIZE], bb[UAF_EID_SIZE];
    uaf_eid_serialize(a, ba);
    uaf_eid_serialize(b, bb);
    return memcmp(ba, bb, UAF_EID_SIZE);
}

/* [R-5.5-009] Both peers know both identities, so both evaluate the same
 * comparison and reach complementary conclusions. The lower identity stays
 * active. Identical identities cannot occur: a queue pair cannot connect to
 * itself, and the comparison returning 0 is reported as a protocol error by
 * the caller. */
int uaf_cm_is_active(const struct uaf_endpoint_id *local,
                     const struct uaf_endpoint_id *remote)
{
    return uaf_eid_compare(local, remote) < 0 ? 1 : 0;
}

size_t uaf_cm_mac_input(uint8_t opcode, const uint8_t hdr[64],
                        const uint8_t *record, const uint8_t nonce[8],
                        const uint8_t timestamp[8],
                        uint8_t out[UAF_CM_MAC_INPUT_MAX])
{
    uint32_t body = uaf_cm_expected_len(opcode);
    if (body == 0u || !hdr || !nonce || !timestamp || !out) return 0u;

    size_t n = 0;
    memcpy(out, hdr, UAF_HDR_CRC_COVER);          /* header bytes 0..47 */
    n += UAF_HDR_CRC_COVER;

    if (body == UAF_CM_REQ_LEN) {                 /* REQ and REP carry a record */
        if (!record) return 0u;
        memcpy(out + n, record, UAF_CM_RECORD_SIZE);
        n += UAF_CM_RECORD_SIZE;
    }
    /* RTU and REJ carry no record: header || nonce || timestamp. */
    memcpy(out + n, nonce, 8);     n += 8;
    memcpy(out + n, timestamp, 8); n += 8;
    return n;
}
