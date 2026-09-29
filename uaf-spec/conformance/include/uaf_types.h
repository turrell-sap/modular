/* uaf_types.h -- UAF-SPEC-001 v2.1 core types. NORMATIVE.
 *
 * ABI rules for this header (Section 4.1):
 *   R1. Every struct that crosses the library boundary uses fixed-width
 *       integer types only. No `enum` field, no `size_t` field. An enum's
 *       size and signedness are implementation-defined in C11, and `size_t`
 *       differs between ILP32 and LP64 Snapdragon userspace; v2.0 used both.
 *   R2. Every such struct is pinned by a _Static_assert on its size, so any
 *       layout drift is a compile error rather than a field-offset bug.
 *   R3. A CONFIGURATION or QUERY struct the application fills and the library
 *       may later extend carries `struct_size` as its first field, set by the
 *       caller: uaf_mr_t, uaf_qp_init_attr, uaf_qp_attr, uaf_device_attr,
 *       uaf_conn_info, uaf_peer_info, uaf_storage_cmd. A PER-OPERATION struct
 *       on the hot path does NOT: uaf_sge, uaf_wr, uaf_wc, uaf_storage_cqe,
 *       uaf_storage_wc. Their layout is fixed for the life of an ABI major
 *       version and pinned by assertion, so a per-call size field would cost a
 *       branch on every work request and buy nothing. v2.1 applied the field
 *       unevenly and did not say which rule it was following.
 *   R4. No struct in this header is a wire format. Wire formats are byte
 *       arrays with explicit offsets; see uaf_wire.h.
 */
#ifndef UAF_TYPES_H
#define UAF_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "uaf_config.h"

typedef struct uaf_device   uaf_device_t;
typedef struct uaf_domain   uaf_domain_t;
typedef struct uaf_qp       uaf_qp_t;
typedef struct uaf_cq       uaf_cq_t;
typedef struct uaf_sq       uaf_sq_t;
typedef struct uaf_cr       uaf_cr_t;

/* ---- Enumerations (constants only; never a struct field) --------------- */

enum uaf_qp_state {
    UAF_QPS_RESET = 0,
    UAF_QPS_INIT  = 1,
    UAF_QPS_RTR   = 2,
    UAF_QPS_RTS   = 3,
    UAF_QPS_SQD   = 4,
    UAF_QPS_SQE   = 5,
    UAF_QPS_ERR   = 6,
};

enum uaf_wr_opcode {
    UAF_WR_SEND             = 0x01,
    UAF_WR_SEND_INLINE      = 0x02,
    UAF_WR_RECV             = 0x03,
    UAF_WR_RDMA_READ        = 0x04,
    UAF_WR_RDMA_WRITE       = 0x05,
    UAF_WR_RDMA_WRITE_IMM   = 0x06,
    UAF_WR_ATOMIC_CMP_SWP   = 0x07,
    UAF_WR_ATOMIC_FETCH_ADD = 0x08,
    UAF_WR_STORAGE_READ     = 0x10,
    UAF_WR_STORAGE_WRITE    = 0x11,
    UAF_WR_STORAGE_FLUSH    = 0x12,
};

/* Wire-only opcodes. v2.0 used 0x80-0x91 on the wire without defining them
 * in any enum, so two implementations had nothing to agree on. */
enum uaf_wire_opcode {
    UAF_OP_RDMA_READ_RESP   = 0x84,
    UAF_OP_ATOMIC_ACK       = 0x88, /* carries the 8-byte original value */
    UAF_OP_CM_REQ           = 0x80,
    UAF_OP_CM_REP           = 0x81,
    UAF_OP_CM_RTU           = 0x82,
    UAF_OP_CM_REJ           = 0x83,
    UAF_OP_ACK              = 0x90,
    /* v2.1 had a single NAK (0x91) whose imm_data was already committed to
     * the expected PSN, while [R-5.6-006] also required a NAK-RNR. One
     * opcode cannot encode both. */
    UAF_OP_NAK_SEQ          = 0x91, /* out of sequence; expected PSN in imm_data */
    UAF_OP_NAK_RNR          = 0x92, /* responder not ready; no receive buffer  */
    UAF_OP_NAK_INVAL        = 0x93, /* malformed request, e.g. misaligned atomic */
};

enum uaf_mr_flags {
    UAF_MR_LOCAL_WRITE  = (1ULL << 0),
    UAF_MR_REMOTE_WRITE = (1ULL << 1),
    UAF_MR_REMOTE_READ  = (1ULL << 2),
    UAF_MR_ATOMIC       = (1ULL << 3),
    UAF_MR_GPU          = (1ULL << 4),
    UAF_MR_CXL          = (1ULL << 5),
};

enum uaf_send_flags {
    UAF_SEND_SIGNALED   = (1U << 0),
    UAF_SEND_INLINE     = (1U << 1),
    UAF_SEND_SOLICITED  = (1U << 2),
    UAF_SEND_FENCE      = (1U << 3),
};

enum uaf_error {
    UAF_OK                =   0,
    UAF_ERR_INVAL         =  -1,
    UAF_ERR_NOMEM         =  -2,
    UAF_ERR_NODEV         =  -3,
    UAF_ERR_PERM          =  -4,
    UAF_ERR_BUSY          =  -5,
    UAF_ERR_TIMEOUT       =  -6,
    UAF_ERR_QP_STATE      =  -7,
    /* -8 was UAF_ERR_CQ_EMPTY in v2.0. Retired: the poll functions report an
     * empty queue as a return of 0 entries (Section 4.7). The value stays
     * reserved so no existing error code has to be renumbered. */
    UAF_ERR_RESERVED_8    =  -8,
    UAF_ERR_MR_FAULT      =  -9,
    UAF_ERR_NOT_SUPPORTED = -10,
    UAF_ERR_PROTO         = -11,
    UAF_ERR_CRC           = -12,
    UAF_ERR_REMOTE        = -13,
    UAF_ERR_RKEY          = -14, /* rkey unknown, or not bound to this QP */
    UAF_ERR_AUTH          = -15, /* CM authentication or replay check failed */
    UAF_ERR_RELOCATED     = -16, /* backend cannot register in place        */
};

/* ---- Wire profiles (Section 3.2) --------------------------------------
 * v2.0 claimed one wire format across all three backends while also
 * offloading UAF-N to ConnectX hardware. Those are mutually exclusive: the
 * NIC emits IB transport headers, not the UAF header. An implementation now
 * declares which profile or profiles it speaks. */
enum uaf_wire_profile {
    UAF_PROFILE_IBV  = (1U << 0), /* IB/RoCEv2 transport, BTH+RETH on wire */
    UAF_PROFILE_UAFR = (1U << 1), /* 64-byte UAF-RMT header over UDP       */
};

/* The data plane a queue pair uses. A profile is not enough to say: UAF-D
 * declares UAF_PROFILE_UAFR for node-to-node traffic and also carries an
 * intra-host CXL.mem path with no packet header and no MTU, and v2.1.1 gave
 * nothing in uaf_create_qp or uaf_conn_info able to select between them. The
 * required RTR attribute mask differs per path (Section 10.1). */
enum uaf_path {
    UAF_PATH_IBV = 1,  /* Profile A: IB or RoCEv2 transport   */
    UAF_PATH_UDP = 2,  /* Profile B: UAF-RMT header over UDP  */
    UAF_PATH_CXL = 3,  /* CXL.mem ring, intra-host (Section 9.5) */
};

/* ---- Device capabilities (Section 4.8) -------------------------------- */
enum uaf_device_cap {
    UAF_CAP_ATOMICS        = (1ULL << 0),
    UAF_CAP_DST            = (1ULL << 1),
    UAF_CAP_GPU_MR         = (1ULL << 2),
    UAF_CAP_CXL_MR         = (1ULL << 3),
    UAF_CAP_DMABUF_MR      = (1ULL << 4),
    UAF_CAP_CQ_EVENTS      = (1ULL << 5),
    /* Set when uaf_reg_mr() can register arbitrary caller memory in place.
     * Clear means the application MUST obtain buffers from uaf_alloc_mr().
     * See Section 4.6; this is the capability that replaces v2.0's silent
     * copy-and-relocate on UAF-S. */
    UAF_CAP_REG_ANY_MEM    = (1ULL << 6),
    UAF_CAP_SYNC_FAULT     = (1ULL << 7), /* MAP_SYNC honoured (UAF-D)     */
    UAF_CAP_HW_CC          = (1ULL << 8), /* congestion control offloaded  */
    /* Bit 9 is reserved. v2.1 referenced a UAF_CAP_MT_POST that was in no
     * enum and had no defined ring discipline (OI-5). Rings are strictly
     * single-producer, single-consumer; a multi-producer discipline needs a
     * compare-and-swap protocol that a later revision may define. */
    UAF_CAP_RESERVED_9     = (1ULL << 9),
};

/* Connection-manager reject reasons. v2.1 used UAF_CM_REASON_CLOSE and
 * assigned it no value. */
enum uaf_cm_reason {
    UAF_CM_REASON_CLOSE          = 0x00, /* orderly disconnect */
    UAF_CM_REASON_NO_QP          = 0x01,
    UAF_CM_REASON_MTU_MISMATCH   = 0x02,
    UAF_CM_REASON_PROFILE        = 0x03,
    UAF_CM_REASON_AUTH           = 0x04,
    UAF_CM_REASON_RESOURCE       = 0x05,
    UAF_CM_REASON_STALE_CONN     = 0x06,
    UAF_CM_REASON_SIMUL_OPEN     = 0x07, /* lost the tie-break of R-5.5-009 */
};

/* ---- Memory region ---------------------------------------------------- */
/* addr is always the address the caller passed to uaf_reg_mr(), or the
 * address uaf_alloc_mr() chose. A backend MUST NOT return a different
 * address than the one it registered. */
/* pal_priv is NOT a member. v2.0 published the backend's private pointer in
 * the application ABI, which froze every backend's internals. A backend
 * instead embeds this struct as the first member of its own private struct
 * and recovers it with container_of:
 *
 *     struct uaf_s_mr { uaf_mr_t pub; int dma_buf_fd; ... };
 *     #define uaf_mr_priv(m) ((struct uaf_s_mr *)(m))
 */
typedef struct uaf_mr {
    uint32_t      struct_size;
    uint32_t      lkey;
    uint32_t      rkey;
    uint32_t      reserved0;
    uaf_domain_t *pd;
    void         *addr;
    uint64_t      length;
    uint64_t      flags;
} uaf_mr_t;
_Static_assert(sizeof(uaf_mr_t) == 48, "uaf_mr layout is ABI");

/* ---- Scatter-gather and work requests --------------------------------- */
struct uaf_sge {
    uint64_t addr;
    uint32_t length;
    uint32_t lkey;
};
_Static_assert(sizeof(struct uaf_sge) == 16, "uaf_sge layout is ABI");

struct uaf_wr {
    uint64_t        wr_id;
    uint32_t        opcode;      /* enum uaf_wr_opcode */
    uint32_t        num_sge;
    struct uaf_sge *sge;
    uint32_t        send_flags;  /* enum uaf_send_flags */
    uint32_t        imm_data;
    uint64_t        remote_addr;
    uint32_t        rkey;
    uint32_t        reserved0;
    uint64_t        compare_add;
    uint64_t        swap;
};
_Static_assert(sizeof(struct uaf_wr) == 64, "uaf_wr layout is ABI");

struct uaf_wc {
    uint64_t wr_id;
    uint32_t opcode;     /* enum uaf_wr_opcode */
    int32_t  status;     /* enum uaf_error */
    uint32_t byte_len;
    uint32_t qp_num;
    uint32_t src_qp;
    uint32_t imm_data;
    uint32_t wc_flags;   /* UAF_WC_* below */
    uint32_t reserved0;
};
_Static_assert(sizeof(struct uaf_wc) == 40, "uaf_wc layout is ABI");

/* v2.0 carried imm_data with no way to know whether it was present. */
#define UAF_WC_WITH_IMM     (1U << 0)
#define UAF_WC_SOLICITED    (1U << 1)
#define UAF_WC_ATOMIC_ORIG  (1U << 2) /* SGE 0 holds the pre-op value */

/* create_qp() in v2.0 took only two CQs: no queue depth, no SGE limit, no
 * queue-pair type, so two implementations would allocate differently. */
/* Reliable connected is the only type v2.1.1 defines. An unreliable
 * datagram type was declared in v2.1 and never specified (OI-4); the
 * enumerator is withdrawn rather than left as a hole. */
enum uaf_qp_type { UAF_QPT_RC = 1 };

struct uaf_qp_init_attr {
    uint32_t struct_size;
    uint32_t qp_type;          /* enum uaf_qp_type */
    uint32_t max_send_wr;      /* MUST be a power of two */
    uint32_t max_recv_wr;      /* MUST be a power of two */
    uint32_t max_send_sge;
    uint32_t max_recv_sge;
    uint32_t max_inline_data;
    uint32_t sq_sig_all;       /* 0: only UAF_SEND_SIGNALED WRs complete */
    uint32_t path;             /* enum uaf_path */
    uint32_t reserved0;
};
_Static_assert(sizeof(struct uaf_qp_init_attr) == 40,
               "uaf_qp_init_attr layout is ABI");

/* Attribute mask for uaf_modify_qp(). v2.0 had no mask, so which fields a
 * transition consumed was unstated. A transition MUST fail with
 * UAF_ERR_INVAL if a field it requires is absent from the mask. */
enum uaf_qp_attr_mask {
    UAF_QP_STATE          = (1U <<  0),
    UAF_QP_SQ_PSN         = (1U <<  1),
    UAF_QP_RQ_PSN         = (1U <<  2),
    UAF_QP_DEST_QPN       = (1U <<  3),
    UAF_QP_PATH_MTU       = (1U <<  4),
    UAF_QP_AV_GID         = (1U <<  5), /* Profile A (IB/RoCE) only */
    UAF_QP_AV_LID         = (1U <<  6), /* Profile A only           */
    UAF_QP_AV_UDP         = (1U <<  7), /* Profile B: IP + UDP port */
    UAF_QP_AV_CXL         = (1U <<  8), /* UAF-D: CXL window        */
    UAF_QP_MAX_RD_ATOMIC  = (1U <<  9),
    UAF_QP_MAX_DEST_RD_AT = (1U << 10),
    UAF_QP_RETRY          = (1U << 11),
    UAF_QP_RNR_RETRY      = (1U << 12),
    UAF_QP_PORT           = (1U << 13),
    UAF_QP_TIMEOUT        = (1U << 14),
};

struct uaf_qp_attr {
    uint32_t struct_size;
    uint32_t qp_state;         /* enum uaf_qp_state */
    uint32_t sq_psn;
    uint32_t rq_psn;
    uint32_t dest_qp_num;
    uint32_t path_mtu;         /* BYTES, including the 64-byte header */
    uint16_t dest_lid;
    uint8_t  max_rd_atomic;
    uint8_t  max_dest_rd_atomic; /* absent in v2.0; responder-side limit */
    uint8_t  retry_cnt;
    uint8_t  rnr_retry;
    uint8_t  port_num;
    uint8_t  reserved0;
    uint8_t  dest_gid[16];     /* Profile A; zero-filled under Profile B */
    /* Profile B address vector. v2.0's RTR rules demanded a GID and a LID
     * from backends that have neither. */
    uint8_t  dest_ip[16];      /* IPv4-mapped IPv6 permitted */
    uint16_t dest_udp_port;
    uint16_t src_udp_port;
    /* rnr_retry above had no defined behaviour in v2.0. */
    uint32_t rnr_timer_ns;     /* responder-not-ready backoff */
    uint64_t timeout_ns;       /* retransmit timeout; 0 selects UAF_RTO_BASE_NS */
};
_Static_assert(sizeof(struct uaf_qp_attr) == 80, "uaf_qp_attr layout is ABI");

/* ---- Device attributes (Section 4.8) ---------------------------------- */
struct uaf_device_attr {
    uint32_t struct_size;      /* caller sets to sizeof(struct uaf_device_attr) */
    uint32_t wire_profiles;    /* enum uaf_wire_profile bitmask */
    uint64_t caps;             /* enum uaf_device_cap bitmask   */
    uint32_t max_qp;
    uint32_t max_cq;
    uint32_t max_mr;
    uint32_t max_sge;
    uint32_t max_inline_data;
    uint32_t max_cqe;
    uint32_t max_sqe;
    uint32_t path_mtu_max;     /* bytes, including header */
    uint64_t max_msg_len;
    uint8_t  max_rd_atomic;
    uint8_t  max_dest_rd_atomic;
    uint8_t  reserved0[6];
};
_Static_assert(sizeof(struct uaf_device_attr) == 64,
               "uaf_device_attr layout is ABI");

/* ---- Connection and peer information --------------------------------- */
/* This struct is host-local. It is NOT the exchange format: it has interior
 * padding and host byte order. Serialise it with uaf_conn_info_serialize()
 * before putting it on a socket (Section 5.5). v2.0 invited a memcpy of a
 * struct with 6 bytes of holes and no byte-order rule.
 *
 * dma_buf_fd is deliberately absent: a file descriptor is process-local and
 * cannot be carried in a connection exchange. It lives in uaf_peer_info,
 * which is same-host only. */
struct uaf_conn_info {
    uint32_t struct_size;
    uint32_t qp_num;
    uint32_t psn;
    uint32_t rkey;
    uint64_t remote_va;
    uint64_t cxl_base;
    uint64_t cxl_size;
    uint8_t  gid[16];
    uint16_t lid;
    uint16_t mtu;              /* bytes, including header */
    uint8_t  sl;
    uint8_t  traffic_class;
    uint8_t  wire_profile;     /* a single enum uaf_wire_profile bit */
    uint8_t  path;             /* enum uaf_path; MUST agree with the peer */
};
_Static_assert(sizeof(struct uaf_conn_info) == 64,
               "uaf_conn_info layout is ABI");

/* Same-host peer mapping. dma_buf_fd MUST be transferred out of band via
 * SCM_RIGHTS over an AF_UNIX socket; it is meaningless to a remote node. */
struct uaf_peer_info {
    uint32_t struct_size;
    uint32_t rkey;
    uint64_t remote_va;
    uint64_t length;
    uint64_t cxl_base;
    int32_t  dma_buf_fd;       /* -1 if unused */
    uint32_t reserved0;
};
_Static_assert(sizeof(struct uaf_peer_info) == 40,
               "uaf_peer_info layout is ABI");

/* ---- DST ------------------------------------------------------------- */
struct uaf_storage_cmd {
    uint32_t struct_size;
    uint32_t opcode;           /* enum uaf_wr_opcode, STORAGE_* only */
    uint64_t wr_id;
    uint64_t lba;
    uint32_t nlb;              /* 1-based block count; 0 is invalid      */
    uint32_t buf_offset;
    void    *buf;
    uint64_t buf_len;          /* was size_t in v2.0: not ABI-stable     */
    uint32_t flags;
    uint32_t reserved0;
};
_Static_assert(sizeof(struct uaf_storage_cmd) == 56,
               "uaf_storage_cmd layout is ABI");

/* Completion ring entry. Exactly 16 bytes, naturally aligned, NOT packed:
 * v2.0's __attribute__((packed)) dropped alignof to 1, which defeats the
 * aligned load a completion ring wants and is undefined behaviour on
 * strict-alignment targets.
 *
 * `phase` is the last byte of the entry and the only one a consumer may read
 * before the rest is known valid. Without it -- v2.0 had 4 reserved bytes
 * and no phase tag -- a poller cannot distinguish a fresh completion from a
 * zeroed slot, because status 0 is also UAF_OK. */
struct uaf_storage_cqe {
    uint16_t cmd_id;
    int16_t  status;            /* enum uaf_error */
    uint32_t bytes_transferred;
    uint32_t latency_ns;        /* clamped, never wrapped, at UINT32_MAX */
    uint16_t reserved0;
    uint8_t  reserved1;
    uint8_t  phase;             /* bit 0: phase tag; bits 7..1 reserved  */
};
_Static_assert(sizeof(struct uaf_storage_cqe) == 16,
               "uaf_storage_cqe is a 16-byte ring entry");
_Static_assert(_Alignof(struct uaf_storage_cqe) == 4,
               "uaf_storage_cqe must not be packed");
_Static_assert(offsetof(struct uaf_storage_cqe, phase) == 15,
               "phase MUST be the last byte written");

#define UAF_CQE_PHASE_MASK  0x01U

/* Overflow-safe containment test: is [off, off+len) inside [0, size)?
 *
 * Every bounds check in this specification is written this way. The natural
 * spelling `off + len > size` wraps for uint64_t and admits a request it should
 * refuse: with off = 2^64 - 8 and len = 16 the sum is 8, which passes. v2.1.3's
 * [R-9.5-002] stated the CXL window check in the wrapping form, and an earlier
 * revision had the same defect in a DST length check.
 *
 * len == 0 is contained in any size, including size == 0. */
static inline int uaf_range_within(uint64_t off, uint64_t len, uint64_t size)
{
    if (len == 0u)   return 1;
    if (off > size)  return 0;
    return len <= size - off;
}

/* The RMT completion the UAF-D agent writes into the CXL window, and the only
 * device-visible RMT completion this specification defines. v2.1.1 told the
 * agent to release-store "the completion" and the host to acquire-load it,
 * naming no object: uaf_wc is 40 bytes, host-side, and has no phase byte.
 *
 * Same discipline as the DST ring: the producer writes bytes 0..30, releases,
 * then stores `phase`; the consumer acquire-loads `phase` first. The host
 * converts an accepted entry into a uaf_wc before handing it to the caller. */
struct uaf_rmt_cqe {
    uint64_t wr_id;
    int32_t  status;            /* enum uaf_error */
    uint32_t byte_len;
    uint32_t imm_data;
    uint32_t wc_flags;          /* UAF_WC_* */
    uint8_t  reserved0[7];      /* MUST be zero */
    uint8_t  phase;             /* bit 0 is the phase tag */
};
_Static_assert(sizeof(struct uaf_rmt_cqe) == 32,
               "uaf_rmt_cqe is a 32-byte ring entry");
_Static_assert(offsetof(struct uaf_rmt_cqe, phase) == 31,
               "phase MUST be the last byte written");
#define UAF_RMT_CQE_ALIGN  32

/* The 16-byte entry above is the DEVICE RING IMAGE. What an application
 * receives from uaf_storage_poll() is this host completion, which carries the
 * caller's full 64-bit wr_id. v2.1 required the library to restore wr_id
 * ([R-6.5-001]) while giving uaf_storage_poll() only the 16-byte image, whose
 * sole identifier is a 16-bit cmd_id -- the requirement had nowhere to land. */
struct uaf_storage_wc {
    uint64_t wr_id;
    int32_t  status;            /* enum uaf_error */
    uint32_t bytes_transferred;
    uint32_t latency_ns;
    uint32_t reserved0;
};
_Static_assert(sizeof(struct uaf_storage_wc) == 24,
               "uaf_storage_wc layout is ABI");

#endif /* UAF_TYPES_H */
