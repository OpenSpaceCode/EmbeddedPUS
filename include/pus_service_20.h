#ifndef PUS_SERVICE_20_H
#define PUS_SERVICE_20_H

#include <stdint.h>
#include "pus_config.h"
#include "pus_context.h"
#include "pus_types.h"

/**
 * Read the current value of a parameter into buf.
 * capacity is guaranteed to be >= the registered value_len.
 */
typedef pus_status_t (*pus_param_getter_t)(
	uint16_t  param_id,
	uint8_t  *buf,
	uint16_t  capacity,
	void     *user_data);

/**
 * Write a new value for a parameter from buf.
 * len equals the registered value_len.
 */
typedef pus_status_t (*pus_param_setter_t)(
	uint16_t       param_id,
	const uint8_t *buf,
	uint16_t       len,
	void          *user_data);

typedef struct {
	uint16_t           param_id;
	uint16_t           value_len;  /* fixed wire size of the parameter value */
	pus_param_getter_t getter;
	pus_param_setter_t setter;     /* NULL = read-only */
	void              *user_data;
	uint8_t            is_used;
} pus_param_entry_t;

typedef struct {
	pus_param_entry_t params[PUS_SERVICE_20_MAX_PARAMS];
} pus_service_20_ctx_t;

pus_status_t pus_service_20_init(pus_service_20_ctx_t *s20);

/**
 * Register a parameter. Registering the same param_id again updates
 * its callbacks in place. setter may be NULL for read-only parameters.
 */
pus_status_t pus_service_20_register_param(
	pus_service_20_ctx_t *s20,
	uint16_t              param_id,
	uint16_t              value_len,
	pus_param_getter_t    getter,
	pus_param_setter_t    setter,
	void                 *user_data);

/**
 * Build and emit TM[20,2] for the given list of parameter IDs.
 * Can be called on demand as well as from the TC[20,1] handler.
 */
pus_status_t pus_service_20_emit_report(
	pus_context_t        *ctx,
	pus_service_20_ctx_t *s20,
	const uint16_t       *param_ids,
	uint8_t               count);

/** Register TC[20,1] and TC[20,3] handlers with the main context. */
pus_status_t pus_service_20_register_handlers(
	pus_context_t        *ctx,
	pus_service_20_ctx_t *s20);

#endif /* PUS_SERVICE_20_H */
