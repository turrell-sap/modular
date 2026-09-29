/* uaf_wire.h -- UAF-SPEC-001 v2.1 UAF-RMT wire format (Profile B).
 * NORMATIVE.
 *
 * The wire format is a byte array with fixed offsets and big-endian fields,
 * not a C struct. No packed struct appears here: a packed struct's layout is
 * a compiler promise, and the wire needs a specification promise.
 *
 * Applies to UAF_PROFILE_UAFR only. A UAF_PROFILE_IBV implementation puts
 * IB transport headers (BTH+RETH) on the wire and never emits this header;
 * see UAF-SPEC-001 Section 3.2.
 */
#ifndef UAF_WIRE_H
#define UAF_WIRE_H

#include <stdint.h>
#include <stddef.h>
#include "uaf_config.h"
#include "uaf_types.h"

/* ---- Header field offsets (Section 5.2) --------------------------------
 *  off  size  field         notes
 *    0     4  magic         0x55414652 "UAFR"
 *    4     1  version       2
 *    5     1  opcode        enum uaf_wr_opcode or enum uaf_wire_opcode
 *    6     2  flags         UAF_WIRE_FLAG_*
 *    8     4  qp_id         destination QP number
 *   12     4  psn           packet sequence number
 *   16     4  msg_id        ties segments of one message together
 *   20     4  rkey          32-bit remote key
 *   24     8  remote_va     8-byte aligned offset in the header
 *   32     4  msg_len       TOTAL message length in bytes
 *   36     4  seg_off       byte offset of this segment within the message
 *   40     4  seg_len       payload bytes carried by THIS packet
 *   44     4  imm_data
 *   48     4  hdr_crc32c    CRC32C over bytes 0..47
 *   52     4  data_crc32c   CRC32C over the seg_len payload bytes
 *   56     8  reserved      MUST be zero
 *
 * v2.0 carried a single `length` with no message identity, no total length
 * and no segment offset, so a multi-packet transfer could not be framed and
 * a responder could not know when to raise one completion. It also spent
 * 4 bytes on a reserved lkey beside 20 further reserved bytes, and left the
 * payload outside every integrity check. */
#define UAF_H_MAGIC        0
#define UAF_H_VERSION      4
#define UAF_H_OPCODE       5
#define UAF_H_FLAGS        6
#define UAF_H_QP_ID        8
#define UAF_H_PSN         12
#define UAF_H_MSG_ID      16
#define UAF_H_RKEY        20
#define UAF_H_REMOTE_VA   24
#define UAF_H_MSG_LEN     32
#define UAF_H_SEG_OFF     36
#define UAF_H_SEG_LEN     40
#define UAF_H_IMM_DATA    44
#define UAF_H_HDR_CRC     48
#define UAF_H_DATA_CRC    52
#define UAF_H_RESERVED    56

/* hdr_crc32c covers everything ahead of it. */
#define UAF_HDR_CRC_COVER  48

/* ---- ACK scheduling (Section 5.6) ------------------------------------
 * [R-5.6-008] A sender SHALL set UAF_WIRE_FLAG_ACK_REQ on the LAST segment of
 * every message and at least once every UAF_ACK_REQ_INTERVAL segments.
 * [R-5.6-009] A receiver SHALL emit an ACK on any segment carrying ACK_REQ,
 * and otherwise within UAF_ACK_COALESCE_NS of receiving in-order data. */
/* ---- CM packet sizes (Section 5.5) -----------------------------------
 * v2.1 required seg_len == 96 for every CM packet while its own diagram showed
 * CM_RTU carrying an authenticator only. */
#define UAF_CM_RECORD_SIZE  64
#define UAF_CM_AUTH_SIZE    32
#define UAF_CM_REQ_LEN      (UAF_CM_RECORD_SIZE + UAF_CM_AUTH_SIZE)  /* 96 */
#define UAF_CM_REP_LEN      (UAF_CM_RECORD_SIZE + UAF_CM_AUTH_SIZE)  /* 96 */
#define UAF_CM_RTU_LEN      (UAF_CM_AUTH_SIZE)                       /* 32 */
#define UAF_CM_REJ_LEN      (UAF_CM_AUTH_SIZE)                       /* 32 */
/* Returns the required seg_len for a CM opcode, or 0 if not a CM opcode. */
uint32_t uaf_cm_expected_len(uint8_t opcode);

/* [R-5.5-006] The MAC covers header bytes 0..47, the connection record when
 * the body carries one, then the nonce and the timestamp. v2.1.1 defined the
 * input only for the 96-byte bodies, so CM_RTU and CM_REJ -- which carry no
 * record -- had no defined MAC input at all.
 *
 * Builds the byte string to be HMAC'd into `out`, returning its length, or 0
 * if `opcode` is not a CM opcode. `record` may be NULL for a 32-byte body. */
#define UAF_CM_MAC_INPUT_MAX (UAF_HDR_CRC_COVER + UAF_CM_RECORD_SIZE + 16)
size_t uaf_cm_mac_input(uint8_t opcode, const uint8_t hdr[64],
                        const uint8_t *record, const uint8_t nonce[8],
                        const uint8_t timestamp[8],
                        uint8_t out[UAF_CM_MAC_INPUT_MAX]);


/* ---- Endpoint identity and simultaneous open (Section 5.5) -----------
 * [R-5.5-009] The tie-break is a total order over a canonical 22-byte
 * identity, and both peers evaluate the same two identities in the same order.
 *
 * WHICH address and WHICH port. An endpoint's identity is its own SOURCE
 * address, its own SOURCE UDP port and its qp_num:
 *
 *   local  identity <- the source address and port this endpoint sends from,
 *                      plus its own qp_num
 *   remote identity <- the source address and port of the IP/UDP header of the
 *                      CM_REQ just received, plus the qp_num in that packet
 *
 * Under Profile A the address is the GID from the connection record and the
 * port field is zero. v2.1 compared destination fields, which rebuilds the very
 * asymmetry the rule exists to remove: A's destination port is B's source port.
 * v2.1.1 fixed the comparison and still did not say which fields fill the
 * image -- and the connection record zeroes the GID under Profile B and carries
 * no IP address, so there was no answer to be had inside it. */
#define UAF_EID_SIZE  22   /* addr[16] || udp_port(2) || qp_num(4), all BE */

struct uaf_endpoint_id {
    uint8_t  addr[16];   /* IPv6, or IPv4-mapped; Profile A uses the GID */
    uint16_t udp_port;   /* 0 under Profile A */
    uint32_t qp_num;
};

void uaf_eid_serialize(const struct uaf_endpoint_id *e,
                       uint8_t out[UAF_EID_SIZE]);
/* Strict total order: <0, 0 or >0, by memcmp of the serialized identities. */
int  uaf_eid_compare(const struct uaf_endpoint_id *a,
                     const struct uaf_endpoint_id *b);
/* Returns 1 if `local` continues as the active side, 0 if it becomes passive.
 * Symmetric by construction: both peers get complementary answers. */
int  uaf_cm_is_active(const struct uaf_endpoint_id *local,
                      const struct uaf_endpoint_id *remote);

/* Profile A (IB transport) carries a 24-bit PSN. A Profile B sender that
 * may be bridged to Profile A MUST keep psn within 24 bits. */
#define UAF_PSN_MASK_IBV   0x00FFFFFFU

/* Host-side decoded view of the header. Not a wire layout. */
struct uaf_wire_hdr {
    uint32_t magic;
    uint8_t  version;
    uint8_t  opcode;
    uint16_t flags;
    uint32_t qp_id;
    uint32_t psn;
    uint32_t msg_id;
    uint32_t rkey;
    uint64_t remote_va;
    uint32_t msg_len;
    uint32_t seg_off;
    uint32_t seg_len;
    uint32_t imm_data;
    uint32_t hdr_crc32c;
    uint32_t data_crc32c;
};

/* ---- CRC32C (Castagnoli, reflected, poly 0x1EDC6F41) ------------------
 * Init 0xFFFFFFFF, final XOR 0xFFFFFFFF, as in RFC 3720 Appendix B.4.
 * NOT an integrity or authentication mechanism: it detects corruption only.
 * An attacker who alters a packet simply recomputes it. See Section 11.1. */
uint32_t uaf_crc32c(uint32_t crc, const void *buf, size_t len);
#define UAF_CRC32C(buf, len) uaf_crc32c(0xFFFFFFFFU, (buf), (len))

/* ---- Header encode / decode ------------------------------------------ */

/* Serialise h into the 64 bytes at out, computing hdr_crc32c. If payload is
 * non-NULL and h->flags has UAF_WIRE_FLAG_DATA_CRC, data_crc32c is computed
 * over h->seg_len bytes of payload. Returns UAF_OK or a negative error. */
int uaf_wire_hdr_encode(const struct uaf_wire_hdr *h, uint8_t out[64],
                        const void *payload);

/* Parse the 64 bytes at in. Validates magic, version, reserved-zero, the
 * flag reserved bits, hdr_crc32c, and the framing invariants of Section 5.3.
 * Returns UAF_OK, UAF_ERR_CRC, UAF_ERR_PROTO or UAF_ERR_INVAL. */
int uaf_wire_hdr_decode(const uint8_t in[64], struct uaf_wire_hdr *h);

/* Verify a decoded header against the payload that followed it. */
int uaf_wire_check_payload(const struct uaf_wire_hdr *h, const void *payload);

/* ---- Framing (Section 5.3) -------------------------------------------
 * A message of msg_len bytes is cut into segments of at most
 * UAF_WIRE_MAX_SEG(mtu). Every segment of one message carries the same
 * msg_id and msg_len. FIRST and LAST are DERIVED, never signalled, so a
 * sender cannot contradict itself:
 *     FIRST := seg_off == 0
 *     LAST  := seg_off + seg_len == msg_len
 * The responder raises exactly one work completion, on LAST.
 * Gather is a local operation: a sender walks its SGE list and emits one
 * contiguous byte stream. The wire carries no SGE list. */
#define UAF_WIRE_IS_FIRST(h)  ((h)->seg_off == 0u)
#define UAF_WIRE_IS_LAST(h)   ((uint64_t)(h)->seg_off + (h)->seg_len == \
                               (uint64_t)(h)->msg_len)

/* Number of segments a msg_len message occupies at a given MTU. */
uint32_t uaf_wire_seg_count(uint32_t msg_len, uint32_t mtu);

/* Fill h for segment index `idx` of a message. Returns UAF_OK, or
 * UAF_ERR_INVAL if idx is past the end of the message. */
int uaf_wire_segment(struct uaf_wire_hdr *h, uint32_t msg_len, uint32_t mtu,
                     uint32_t idx);

/* ---- Atomics (Section 5.4) -------------------------------------------
 * v2.0 specified a 16-byte request payload and never defined the response,
 * so the fetched value had nowhere to go and atomics could not be built.
 *
 *   CMP_SWP   request : seg_len 16, payload = compare(8) || swap(8), BE
 *   FETCH_ADD request : seg_len  8, payload = addend(8), BE
 *   ATOMIC_ACK        : seg_len  8, payload = pre-operation value(8), BE
 *                       msg_id echoes the request
 *
 * remote_va MUST be 8-byte aligned; a responder MUST answer a misaligned
 * request with NAK and MUST NOT perform the operation. The requester writes
 * the returned value into SGE 0, which MUST be at least 8 bytes and locally
 * writable, and sets UAF_WC_ATOMIC_ORIG in the completion. */
#define UAF_ATOMIC_CMP_SWP_LEN   16
#define UAF_ATOMIC_FETCH_ADD_LEN  8
#define UAF_ATOMIC_ACK_LEN        8
#define UAF_ATOMIC_ALIGN          8

/* ---- Connection exchange wire form (Section 5.5) ---------------------
 * 64 bytes, big-endian, no holes. uaf_conn_info itself has interior padding
 * and host byte order and MUST NOT be written to a socket. */
#define UAF_CONN_WIRE_SIZE  64
#define UAF_CW_QP_NUM        0
#define UAF_CW_PSN           4
#define UAF_CW_RKEY          8
#define UAF_CW_LID          12
#define UAF_CW_MTU          14
#define UAF_CW_REMOTE_VA    16
#define UAF_CW_CXL_BASE     24
#define UAF_CW_CXL_SIZE     32
#define UAF_CW_GID          40
#define UAF_CW_SL           56
#define UAF_CW_TCLASS       57
#define UAF_CW_PROFILE      58
#define UAF_CW_PATH         59  /* enum uaf_path */
#define UAF_CW_RESERVED     60  /* 4 bytes, MUST be zero */

int uaf_conn_info_serialize(const struct uaf_conn_info *ci,
                            uint8_t out[UAF_CONN_WIRE_SIZE]);
int uaf_conn_info_deserialize(const uint8_t in[UAF_CONN_WIRE_SIZE],
                              struct uaf_conn_info *ci);

/* ---- Big-endian primitives ------------------------------------------- */
static inline void uaf_put_u8(uint8_t *p, uint8_t v)  { p[0] = v; }
static inline void uaf_put_be16(uint8_t *p, uint16_t v)
{ p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; }
static inline void uaf_put_be32(uint8_t *p, uint32_t v)
{ p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16);
  p[2] = (uint8_t)(v >> 8);  p[3] = (uint8_t)v; }
static inline void uaf_put_be64(uint8_t *p, uint64_t v)
{ uaf_put_be32(p, (uint32_t)(v >> 32)); uaf_put_be32(p + 4, (uint32_t)v); }
static inline uint8_t  uaf_get_u8(const uint8_t *p)  { return p[0]; }
static inline uint16_t uaf_get_be16(const uint8_t *p)
{ return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }
static inline uint32_t uaf_get_be32(const uint8_t *p)
{ return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8)  | (uint32_t)p[3]; }
static inline uint64_t uaf_get_be64(const uint8_t *p)
{ return ((uint64_t)uaf_get_be32(p) << 32) | (uint64_t)uaf_get_be32(p + 4); }
/* [R-5.5-007] Replay is keyed on (nonce, qp_id) where qp_id is taken from the
 * HEADER, not from the connection record: a 32-byte CM body has no record, and
 * v2.1.1 keyed replay on a field that was not present in half of the CM
 * packets it applied to. */
static inline uint32_t uaf_cm_replay_qp(const uint8_t hdr[64])
{ return uaf_get_be32(hdr + UAF_H_QP_ID); }

#endif /* UAF_WIRE_H */
