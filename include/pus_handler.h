#ifndef PUS_HANDLER_H
#define PUS_HANDLER_H

#include "pus_context.h"

/**
 * Register a TC handler for a specific service/subtype combination.
 *
 * @param ctx           PUS context
 * @param service       PUS service type
 * @param subtype       PUS subtype
 * @param handler       Handler function to call
 * @param user_data     User data passed to handler
 * @return              PUS_STATUS_OK on success, error code on failure
 */
pus_status_t pus_handler_register(
	pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype,
	pus_tc_handler_t handler,
	void *user_data);

/**
 * Find a registered handler for the given service/subtype.
 * Returns the index of the handler or -1 if not found.
 *
 * @param ctx           PUS context
 * @param service       PUS service type
 * @param subtype       PUS subtype
 * @return              Handler index or -1 if not found
 */
int pus_handler_find(
	const pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype);

#endif /* PUS_HANDLER_H */
