#ifndef PUS_HANDLER_H
#define PUS_HANDLER_H

#include "pus_context.h"

/**
 * Register or update a TC handler for a (service, subtype) pair.
 * Updating an existing pair replaces the handler and user_data in place.
 */
pus_status_t pus_handler_register(
	pus_context_t    *ctx,
	pus_service_t     service,
	pus_subtype_t     subtype,
	pus_tc_handler_t  handler,
	void             *user_data);

/**
 * Return the table index of a registered handler, or -1 if not found.
 */
int pus_handler_find(
	const pus_context_t *ctx,
	pus_service_t        service,
	pus_subtype_t        subtype);

#endif /* PUS_HANDLER_H */
