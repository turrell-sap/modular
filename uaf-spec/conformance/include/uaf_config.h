/* uaf_config.h -- UAF-SPEC-001 v2.1 compile-time configuration.
 *
 * NORMATIVE. Values here that appear on the wire or in an ABI struct are
 * fixed by the specification and MUST NOT be changed by an implementation.
 * Values marked TUNABLE are defaults only; the authoritative limits for a
 * live device are those reported by uaf_query_device() (see UAF-SPEC-001
 * Section 4.8). An application MUST NOT assume a compile-time limit matches
 * the library it is linked against.
 */
#ifndef UAF_CONFIG_H
#define UAF_CONFIG_H

#define UAF_VERSION_MAJOR       2
#define UAF_VERSION_MINOR       1
#define UAF_VERSION_PATCH       4

/* Feature flags. These select OPTIONAL code paths inside one build of the
 * library. They MUST NOT change the layout or size of any ABI struct; every
 * ABI struct is asserted to a fixed size in uaf_types.h. */
#define UAF_ENABLE_ATOMICS      1
#define UAF_ENABLE_DST          1
#define UAF_ENABLE_CXL_CACHE    0
#define UAF_ENABLE_DEBUG        0

/* Resource limits (TUNABLE defaults; query at runtime). */
#define UAF_MAX_QP              4096
#define UAF_MAX_CQ              4096
#define UAF_MAX_MR              65536
#define UAF_MAX_SGE             16
#define UAF_MAX_INLINE          256
#define UAF_CQ_DEPTH_DEFAULT    1024
#define UAF_SQ_DEPTH_DEFAULT    256

/* Ring depths MUST be powers of two so that index wrap is a mask, never a
 * modulo by a runtime value (Section 10.3). */
#define UAF_IS_POW2(x)          (((x) != 0u) && (((x) & ((x) - 1u)) == 0u))

/* ---- Wire protocol constants (NORMATIVE, Section 5) -------------------- */
#define UAF_WIRE_MAGIC          0x55414652U /* "UAFR" */
#define UAF_WIRE_VERSION        2
#define UAF_WIRE_HDR_SIZE       64

/* UAF_WIRE_MTU_DEFAULT is the PATH MTU: it counts the 64-byte UAF header.
 * The largest payload one packet may carry is therefore MTU - 64. v2.0
 * defined MTU as payload only, which made a maximum packet 4160 bytes and
 * would not fit a 4 KiB path MTU. */
#define UAF_WIRE_MTU_DEFAULT    4096
#define UAF_WIRE_MAX_SEG(mtu)   ((uint32_t)((mtu) - UAF_WIRE_HDR_SIZE))
#define UAF_WIRE_SEG_DEFAULT    UAF_WIRE_MAX_SEG(UAF_WIRE_MTU_DEFAULT) /* 4032 */

/* Largest single RMT message. seg_off and msg_len are 32-bit. */
#define UAF_WIRE_MAX_MSG        0xFFFFFFFFU

/* Header flags (offset 6, 2 bytes, network order). */
#define UAF_WIRE_FLAG_INLINE    (1U << 0)
#define UAF_WIRE_FLAG_IMM       (1U << 1)
#define UAF_WIRE_FLAG_SOLICITED (1U << 2)
#define UAF_WIRE_FLAG_ACK_REQ   (1U << 3)
#define UAF_WIRE_FLAG_DATA_CRC  (1U << 4) /* data_crc32c field is valid */
#define UAF_WIRE_FLAG_RETRY     (1U << 5) /* this is a retransmission */
/* Set on an ACK to reflect that CE-marked data arrived in the interval the ACK
 * covers. v2.1 told the receiver to reflect IP ECN CE "in the next ACK" and
 * gave the ACK no field or flag able to carry it. */
#define UAF_WIRE_FLAG_ECN_ECHO  (1U << 6)
#define UAF_WIRE_FLAG_RESERVED  0xFF80U   /* MUST be zero on transmit */

/* ---- Reliability (NORMATIVE, Section 5.6) ------------------------------
 * v2.0 specified a 100 ms base RTO in a fabric targeting 1.5 us, and a
 * fixed 64-segment window that caps a 400 Gbps link at ~205 Gbps once the
 * round trip reaches 10 us. Both are corrected here. */
#define UAF_RTO_BASE_NS         50000ULL   /* 50 us base retransmit timeout */
#define UAF_RTO_MAX_NS          2000000ULL /* 2 ms ceiling after backoff    */
#define UAF_RETRY_MAX           8
#define UAF_WINDOW_DEFAULT      256        /* segments; see UAF_WINDOW_MIN  */

/* ACK scheduling. Named by [R-5.6-008] and [R-5.6-009]; v2.1.1 left both out
 * of this header, so a requirement referred to a constant an implementer could
 * not find. */
#define UAF_ACK_REQ_INTERVAL    (UAF_WINDOW_DEFAULT / 2)  /* 128 segments */
#define UAF_ACK_COALESCE_NS     25000ULL                  /* 25 us        */

/* A conformant implementation MUST size the send window to at least the
 * bandwidth-delay product of the path, in segments. */
#define UAF_WINDOW_MIN(bps, rtt_ns, seg) \
    (uint32_t)((((uint64_t)(bps) / 8u) * (uint64_t)(rtt_ns) / 1000000000ull \
                + (uint64_t)(seg) - 1u) / (uint64_t)(seg))

/* ---- Security (NORMATIVE, Section 11) --------------------------------- */
/* Consecutive rkey validation failures on one QP, counted ONLY for packets
 * that passed the link authenticator, after which the responder MUST throttle
 * validation on that QP. It MUST NOT change queue-pair state: v2.1 moved the
 * QP to UAF_QPS_ERR here, which handed any host able to reach the UDP port an
 * off-path kill in exactly 16 datagrams and contradicted [R-5.7-001]. */
#define UAF_RKEY_FAIL_MAX       16
/* Minimum interval between rkey validations once throttling is engaged. At
 * this rate the 2^32 key space takes over a century to search. */
#define UAF_RKEY_THROTTLE_NS    1000000ULL   /* 1 ms */
#define UAF_CM_NONCE_BYTES      8
#define UAF_CM_MAC_BYTES        16  /* HMAC-SHA256 truncated to 128 bits */
#define UAF_CM_REPLAY_WINDOW_NS 2000000000ULL /* 2 s */

/* ---- Completion ring alignment (NORMATIVE, Section 6.3) --------------- */
#define UAF_CQE_ALIGN           16

#endif /* UAF_CONFIG_H */
