/* uaf_core.h -- UAF-SPEC-001 v2.1 public API. NORMATIVE.
 *
 * Return convention (Section 4.7). Two kinds of function, and the
 * difference is stated here because v2.0 left it undefined for the two
 * functions on the hot path:
 *
 *   (a) Control functions return 0 (UAF_OK) on success, or a NEGATIVE
 *       enum uaf_error on failure.
 *   (b) The poll functions -- uaf_poll_cq() and uaf_storage_poll() -- return
 *       a NON-NEGATIVE COUNT of entries written, from 0 to max inclusive, or
 *       a NEGATIVE enum uaf_error. A return of 0 means the queue was empty;
 *       it is not an error, and UAF_ERR_CQ_EMPTY no longer exists. Callers
 *       MUST test `n < 0` for failure, never `n != UAF_OK`.
 */
#ifndef UAF_CORE_H
#define UAF_CORE_H

#include <stdint.h>
#include <stddef.h>
#include "uaf_config.h"
#include "uaf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Library ---------------------------------------------------------- */
int         uaf_init(void);
int         uaf_fini(void);
const char *uaf_version(void);
const char *uaf_backend_name(void);
uint32_t    uaf_abi_version(void); /* UAF_VERSION_MAJOR << 16 | MINOR << 8 */

/* ---- Devices and domains --------------------------------------------- */
int uaf_get_device_list(uaf_device_t ***devs, int *count);
int uaf_free_device_list(uaf_device_t **devs, int count);

/* Section 4.8. The caller MUST set attr->struct_size to
 * sizeof(struct uaf_device_attr); the library fills only the prefix it
 * understands and rewrites struct_size with what it filled. This is the only
 * authoritative source of limits and capabilities: the constants in
 * uaf_config.h are build-time defaults and MUST NOT be relied on. */
int uaf_query_device(uaf_device_t *dev, struct uaf_device_attr *attr);

int uaf_open_device(uaf_device_t *dev, uaf_domain_t **pd);
int uaf_close_device(uaf_device_t *dev);

/* Destroys a protection domain. v2.0 created domains in open_device and
 * provided no way to release one. MUST be called after every MR, CQ and QP
 * in the domain has been destroyed; returns UAF_ERR_BUSY otherwise. */
int uaf_close_domain(uaf_domain_t *pd);

/* ---- Memory regions (Section 4.6) ------------------------------------
 * uaf_reg_mr() registers the caller's memory IN PLACE. On success
 * (*mr)->addr == addr, always. A backend that cannot map arbitrary caller
 * memory MUST fail with UAF_ERR_NOT_SUPPORTED and MUST NOT copy, bounce or
 * relocate the buffer -- v2.0's UAF-S backend allocated a second buffer,
 * copied into it and reported the copy's address, so every later write the
 * application made to its own buffer was invisible to the fabric.
 *
 * Whether in-place registration is available is reported as
 * UAF_CAP_REG_ANY_MEM. A portable application either tests that bit, or
 * simply always allocates through uaf_alloc_mr(), which every backend
 * supports. */
int uaf_reg_mr(uaf_domain_t *pd, void *addr, size_t len,
               uint64_t flags, uaf_mr_t **mr);

/* Allocates DMA-capable memory and registers it in one step. The address is
 * chosen by the backend (dma-heap on UAF-S, a CXL window on UAF-D, host or
 * device memory on UAF-N) and returned in (*mr)->addr. */
int uaf_alloc_mr(uaf_domain_t *pd, size_t len, uint64_t flags, uaf_mr_t **mr);

int uaf_dereg_mr(uaf_mr_t *mr);  /* for uaf_reg_mr()   */
int uaf_free_mr(uaf_mr_t *mr);   /* for uaf_alloc_mr() */

/* ---- Completion queues ------------------------------------------------ */
int uaf_create_cq(uaf_domain_t *pd, int cqe, uaf_cq_t **cq);
int uaf_destroy_cq(uaf_cq_t *cq);

/* Returns the number of work completions written to wc[0..max-1], 0 if the
 * queue was empty, or a negative enum uaf_error. */
int uaf_poll_cq(uaf_cq_t *cq, int max, struct uaf_wc *wc);

/* ---- Queue pairs ----------------------------------------------------- */
/* init carries queue depth, SGE limits and the queue-pair type. v2.0's
 * create_qp took two completion queues and nothing else. */
int uaf_create_qp(uaf_domain_t *pd, uaf_cq_t *scq, uaf_cq_t *rcq,
                  struct uaf_qp_init_attr *init, uaf_qp_t **qp);
/* attr->qp_state MUST equal `state`; the library rejects a mismatch with
 * UAF_ERR_INVAL rather than silently preferring one. attr_mask names the
 * fields this transition may read; a missing required field is UAF_ERR_INVAL. */
int uaf_modify_qp(uaf_qp_t *qp, enum uaf_qp_state state,
                  struct uaf_qp_attr *attr, uint32_t attr_mask);
int uaf_destroy_qp(uaf_qp_t *qp);
int uaf_post_send(uaf_qp_t *qp, struct uaf_wr *wr);
int uaf_post_recv(uaf_qp_t *qp, struct uaf_wr *wr);

/* ---- Connection management ------------------------------------------- */
int uaf_get_conn_info(uaf_qp_t *qp, struct uaf_conn_info *local);
/* uaf_conn_info is host-local and padded; these produce and consume the
 * 64-byte big-endian exchange form of Section 5.5. */
int uaf_conn_info_serialize(const struct uaf_conn_info *ci, uint8_t out[64]);
int uaf_conn_info_deserialize(const uint8_t in[64], struct uaf_conn_info *ci);
int uaf_connect(uaf_qp_t *qp, const struct uaf_conn_info *remote);
int uaf_disconnect(uaf_qp_t *qp);

/* Same-host peer mapping; dma_buf_fd arrives via SCM_RIGHTS. */
int uaf_map_peer(uaf_domain_t *pd, const struct uaf_peer_info *peer,
                 uaf_mr_t **mr);
int uaf_unmap_peer(uaf_mr_t *mr);

/* ---- Direct Storage Transport ---------------------------------------- */
int uaf_storage_open(uaf_domain_t *pd, const char *path,
                     uaf_sq_t **sq, uaf_cr_t **cr);
int uaf_storage_submit(uaf_sq_t *sq, struct uaf_storage_cmd *cmd);

/* Returns the number of completions written, 0 if none were ready, or a
 * negative enum uaf_error. */
int uaf_storage_poll(uaf_cr_t *cr, int max, struct uaf_storage_cqe *cqe);

/* Closes BOTH objects that uaf_storage_open() created. v2.0's close took
 * only the submission queue, so the completion ring -- and, on UAF-N, the
 * cuFile batch handle it shares -- leaked. */
int uaf_storage_close(uaf_sq_t *sq, uaf_cr_t *cr);

/* Moves a storage queue from the ERROR state back to IDLE (Section 6.4).
 * v2.0's state machine showed this edge with no entry point to drive it. */
int uaf_storage_reset(uaf_sq_t *sq, uaf_cr_t *cr);

/* ---- Diagnostics ----------------------------------------------------- */
const char *uaf_strerror(int e);
void        uaf_set_log_level(int level);
void        uaf_set_log_callback(void (*cb)(int level, const char *msg));

#ifdef __cplusplus
}
#endif
#endif /* UAF_CORE_H */
