#ifndef PUS_SERVICE_3_H
#define PUS_SERVICE_3_H

#include "pus_config.h"
#include "pus_context.h"
#include "pus_types.h"

#include <stdint.h>

#define PUS_SERVICE_3_KIND_HK 0u   /**< Housekeeping structure kind. */
#define PUS_SERVICE_3_KIND_DIAG 1u /**< Diagnostic structure kind. */

/**
 * @brief HK data provider callback, called by the emit functions.
 *
 * @param[in]     sid       Structure ID being collected.
 * @param[out]    buf       Output buffer to write parameter data into.
 * @param[in]     capacity  Buffer size in bytes.
 * @param[out]    out_len   Receives the number of bytes written (must be <= capacity).
 * @param[in,out] user_data Opaque pointer supplied at registration.
 *
 * @return PUS_STATUS_OK on success; any other value aborts the emit.
 */
typedef pus_status_t (*pus_hk_provider_t)(uint16_t sid,
                                          uint8_t *buf,
                                          uint16_t capacity,
                                          uint16_t *out_len,
                                          void *user_data);

/** @brief One entry in the ST[03] structure table. */
typedef struct
{
    uint16_t sid;               /**< Structure ID. */
    uint8_t kind;               /**< PUS_SERVICE_3_KIND_HK or PUS_SERVICE_3_KIND_DIAG. */
    pus_hk_provider_t provider; /**< Data provider callback. */
    void *user_data;            /**< Forwarded to provider. */
    uint8_t is_used;            /**< Non-zero when the slot is occupied. */
} pus_hk_entry_t;

/** @brief ST[03] service context. Allocate statically. */
typedef struct
{
    pus_hk_entry_t structures[PUS_SERVICE_3_MAX_STRUCTURES]; /**< Structure table. */
} pus_service_3_ctx_t;

/**
 * @brief Initialise an ST[03] context.
 *
 * @param[out] s3 Context to initialise.
 *
 * @return PUS_STATUS_NULL if s3 is NULL, PUS_STATUS_OK otherwise.
 */
pus_status_t pus_service_3_init(pus_service_3_ctx_t *s3);

/**
 * @brief Register a housekeeping structure provider.
 * Re-registering the same SID updates the entry in place.
 *
 * @param[in,out] s3        ST[03] context.
 * @param[in]     sid       Structure ID to register.
 * @param[in]     provider  Data provider callback.
 * @param[in]     user_data Forwarded to provider.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_TABLE_FULL, or PUS_STATUS_OK.
 */
pus_status_t pus_service_3_register_hk(pus_service_3_ctx_t *s3,
                                       uint16_t sid,
                                       pus_hk_provider_t provider,
                                       void *user_data);

/**
 * @brief Register a diagnostic structure provider.
 * Re-registering the same SID updates the entry in place.
 *
 * @param[in,out] s3        ST[03] context.
 * @param[in]     sid       Structure ID to register.
 * @param[in]     provider  Data provider callback.
 * @param[in]     user_data Forwarded to provider.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_TABLE_FULL, or PUS_STATUS_OK.
 */
pus_status_t pus_service_3_register_diag(pus_service_3_ctx_t *s3,
                                         uint16_t sid,
                                         pus_hk_provider_t provider,
                                         void *user_data);

/**
 * @brief Build and emit TM[3,25] for a registered HK structure.
 *
 * @param[in,out] ctx Active PUS context (provides TM sink).
 * @param[in]     s3  ST[03] context.
 * @param[in]     sid Structure ID to report.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_NO_HANDLER, provider error, or PUS_STATUS_OK.
 */
pus_status_t pus_service_3_emit_hk(pus_context_t *ctx, pus_service_3_ctx_t *s3, uint16_t sid);

/**
 * @brief Build and emit TM[3,26] for a registered diagnostic structure.
 *
 * @param[in,out] ctx Active PUS context (provides TM sink).
 * @param[in]     s3  ST[03] context.
 * @param[in]     sid Structure ID to report.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_NO_HANDLER, provider error, or PUS_STATUS_OK.
 */
pus_status_t pus_service_3_emit_diag(pus_context_t *ctx, pus_service_3_ctx_t *s3, uint16_t sid);

#endif /* PUS_SERVICE_3_H */
