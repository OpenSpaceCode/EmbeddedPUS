#ifndef PUS_SERVICE_17_H
#define PUS_SERVICE_17_H

#include <stdint.h>
#include "pus_context.h"
#include "pus_types.h"

/**
 * Emit TM[17,2] are-you-alive connection test report.
 * Payload: empty (ECSS §8.17.3.2).
 */
pus_status_t pus_service_17_emit_alive_report(
	pus_context_t *ctx,
	uint16_t       destination_id);

/**
 * Emit TM[17,4] on-board connection test report.
 * Payload: tested application process ID (2 bytes, ECSS §8.17.3.4).
 */
pus_status_t pus_service_17_emit_connection_report(
	pus_context_t *ctx,
	uint16_t       apid,
	uint16_t       destination_id);

/**
 * Register TC[17,1] and TC[17,3] handlers with the main context.
 * Handlers reply directly to the TC source_id.
 */
pus_status_t pus_service_17_register_handlers(pus_context_t *ctx);

#endif /* PUS_SERVICE_17_H */
