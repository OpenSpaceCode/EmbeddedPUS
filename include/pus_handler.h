#ifndef PUS_HANDLER_H
#define PUS_HANDLER_H

#include "pus_context.h"

/**
 * @brief Register or update a TC handler for a (service, subtype) pair.
 * If the pair is already registered, the handler and user_data are replaced.
 *
 * Passing NULL for @p handler deregisters the entry for that (service, subtype)
 * pair. If no entry is found for that pair, PUS_STATUS_NO_HANDLER is returned.
 *
 * @param[in,out] ctx       Active PUS context.
 * @param[in]     service   Service type identifier.
 * @param[in]     subtype   Service subtype identifier.
 * @param[in]     handler   Callback to register, or NULL to deregister.
 * @param[in]     user_data Opaque pointer forwarded to the handler (ignored when deregistering).
 *
 * @return PUS_STATUS_NULL if ctx is NULL.
 * @return PUS_STATUS_NO_HANDLER if handler is NULL and no matching entry exists.
 * @return PUS_STATUS_TABLE_FULL if the table is full and no matching entry exists.
 * @return PUS_STATUS_OK on success.
 */
pus_status_t pus_handler_register(
	pus_context_t    *ctx,
	pus_service_t     service,
	pus_subtype_t     subtype,
	pus_tc_handler_t  handler,
	void             *user_data);

/**
 * @brief Invoke the registered handler for a (service, subtype) pair.
 *
 * Calls the handler with @p ctx and @p tc, passing through the stored
 * user_data.
 *
 * @param[in,out] ctx     Active PUS context.
 * @param[in]     service Service type identifier.
 * @param[in]     subtype Service subtype identifier.
 * @param[in]     tc      TC packet to pass to the handler.
 *
 * @return PUS_STATUS_NULL if ctx or tc is NULL.
 * @return PUS_STATUS_NO_HANDLER if no handler is registered for the pair.
 * @return Whatever the handler returns otherwise.
 */
pus_status_t pus_handler_invoke(
	pus_context_t         *ctx,
	pus_service_t          service,
	pus_subtype_t          subtype,
	const pus_tc_packet_t *tc);

#endif /* PUS_HANDLER_H */
