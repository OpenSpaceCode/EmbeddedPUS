#ifndef PUS_SERVICE_20_H
#define PUS_SERVICE_20_H

#include "pus_config.h"
#include "pus_context.h"
#include "pus_types.h"

#include <stdint.h>

/**
 * @brief Parameter getter callback; reads the current value of a parameter.
 *
 * @param[in]     param_id  Parameter ID being read.
 * @param[out]    buf       Output buffer (guaranteed to be >= registered value_len bytes).
 * @param[in]     capacity  Output buffer size in bytes.
 * @param[in,out] user_data Opaque pointer supplied at registration.
 *
 * @return PUS_STATUS_OK on success; any other value aborts the emit.
 */
typedef pus_status_t (*pus_param_getter_t)(uint16_t param_id,
                                           uint8_t *buf,
                                           uint16_t capacity,
                                           void    *user_data);

/**
 * @brief Parameter setter callback; writes a new value for a parameter.
 *
 * @param[in]     param_id  Parameter ID being written.
 * @param[in]     buf       New value bytes.
 * @param[in]     len       Value length in bytes (equals the registered value_len).
 * @param[in,out] user_data Opaque pointer supplied at registration.
 *
 * @return PUS_STATUS_OK on success; any other value is reported as a completion failure.
 */
typedef pus_status_t (*pus_param_setter_t)(uint16_t       param_id,
                                           const uint8_t *buf,
                                           uint16_t       len,
                                           void          *user_data);

/** @brief One entry in the ST[20] parameter table. */
typedef struct
{
    uint16_t           param_id;  /**< Parameter ID. */
    uint16_t           value_len; /**< Fixed wire size of the parameter value in bytes. */
    pus_param_getter_t getter;    /**< Read callback. */
    pus_param_setter_t setter;    /**< Write callback; NULL means read-only. */
    void              *user_data; /**< Forwarded to getter and setter. */
    uint8_t            is_used;   /**< Non-zero when the slot is occupied. */
} pus_param_entry_t;

/** @brief ST[20] service context. Allocate statically. */
typedef struct
{
    pus_param_entry_t params[PUS_SERVICE_20_MAX_PARAMS]; /**< Parameter table. */
} pus_service_20_ctx_t;

/**
 * @brief Initialise an ST[20] context.
 *
 * @param[out] s20 Context to initialise.
 *
 * @return PUS_STATUS_NULL if s20 is NULL, PUS_STATUS_OK otherwise.
 */
pus_status_t pus_service_20_init(pus_service_20_ctx_t *s20);

/**
 * @brief Register a parameter. Re-registering the same param_id updates the entry in place.
 *
 * @param[in,out] s20       ST[20] context.
 * @param[in]     param_id  Parameter ID.
 * @param[in]     value_len Fixed wire size of the parameter value in bytes.
 * @param[in]     getter    Read callback (required).
 * @param[in]     setter    Write callback; pass NULL for a read-only parameter.
 * @param[in]     user_data Forwarded to getter and setter.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_TABLE_FULL, or PUS_STATUS_OK.
 */
pus_status_t pus_service_20_register_param(pus_service_20_ctx_t *s20,
                                           uint16_t              param_id,
                                           uint16_t              value_len,
                                           pus_param_getter_t    getter,
                                           pus_param_setter_t    setter,
                                           void                 *user_data);

/**
 * @brief Build and emit TM[20,2] for the given list of parameter IDs.
 *
 * @param[in,out] ctx       Active PUS context (provides TM sink).
 * @param[in]     s20       ST[20] context.
 * @param[in]     param_ids Array of parameter IDs to include in the report.
 * @param[in]     count     Number of entries in param_ids.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_NO_HANDLER, getter error, PUS_STATUS_BUFFER_TOO_SMALL,
 *         or PUS_STATUS_OK.
 */
pus_status_t pus_service_20_emit_report(pus_context_t        *ctx,
                                        pus_service_20_ctx_t *s20,
                                        const uint16_t       *param_ids,
                                        uint8_t               count);

/**
 * @brief Register TC[20,1] and TC[20,3] handlers with the PUS context.
 *
 * @param[in,out] ctx Active PUS context.
 * @param[in]     s20 ST[20] context passed to the handlers as user_data.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_TABLE_FULL, or PUS_STATUS_OK.
 */
pus_status_t pus_service_20_register_handlers(pus_context_t *ctx, pus_service_20_ctx_t *s20);

#endif /* PUS_SERVICE_20_H */
