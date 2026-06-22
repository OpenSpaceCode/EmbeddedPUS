#ifndef PUS_HANDLER_H
#define PUS_HANDLER_H

#include "pus_context.h"

/**
 * @brief Register or update a TC handler for a (service, subtype) pair.
 * If the pair is already registered, the handler and user_data are replaced.
 *
 * @param[in,out] ctx       Active PUS context.
 * @param[in]     service   Service type identifier.
 * @param[in]     subtype   Service subtype identifier.
 * @param[in]     handler   Callback invoked when a matching TC is received.
 * @param[in]     user_data Opaque pointer forwarded to the handler.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_TABLE_FULL, or PUS_STATUS_OK.
 */
pus_status_t pus_handler_register(
	pus_context_t    *ctx,
	pus_service_t     service,
	pus_subtype_t     subtype,
	pus_tc_handler_t  handler,
	void             *user_data);

/**
 * @brief Return the handler table index for a (service, subtype) pair.
 *
 * @param[in] ctx     Active PUS context.
 * @param[in] service Service type identifier.
 * @param[in] subtype Service subtype identifier.
 *
 * @return Table index (>= 0) if found, -1 if not registered.
 */
int pus_handler_find(
	const pus_context_t *ctx,
	pus_service_t        service,
	pus_subtype_t        subtype);

#endif /* PUS_HANDLER_H */
