#ifndef PUS_HANDLER_H
#define PUS_HANDLER_H

#include "pus_context.h"

/**
 * @file pus_handler.h
 * @brief Telecommand handler registration and lookup API.
 */

/**
 * @brief Register or update a TC handler for a service/subtype pair.
 *
 * If the same service/subtype pair is already registered, the callback and
 * user data are replaced in place and the registry size is unchanged.
 *
 * @param[in,out] ctx       Initialized PUS context.
 * @param[in]     service   PUS service type to match.
 * @param[in]     subtype   Service-specific subtype to match.
 * @param[in]     handler   Callback to invoke for matching TCs.
 * @param[in,out] user_data Opaque pointer passed to @p handler.
 *
 * @retval PUS_STATUS_OK         Handler was registered or updated.
 * @retval PUS_STATUS_NULL       @p ctx or @p handler was NULL.
 * @retval PUS_STATUS_TABLE_FULL No free registry slot was available.
 */
pus_status_t pus_handler_register(
	pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype,
	pus_tc_handler_t handler,
	void *user_data);

/**
 * @brief Find a registered handler for a service/subtype pair.
 *
 * @param[in] ctx     Initialized PUS context.
 * @param[in] service PUS service type to match.
 * @param[in] subtype Service-specific subtype to match.
 *
 * @return Zero-based handler table index, or -1 when not found or when
 *         @p ctx is NULL.
 */
int pus_handler_find(
	const pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype);

#endif /* PUS_HANDLER_H */
