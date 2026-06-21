#ifndef PUS_SERVICE_5_H
#define PUS_SERVICE_5_H

#include <stdint.h>
#include "pus_context.h"
#include "pus_types.h"

/**
 * Emit a Service 5 event report TM packet.
 *
 * subtype   — one of PUS_SUBTYPE_EVENT_* from pus_services.h
 * event_id  — ECSS Event Definition ID (EVID), 2 bytes on the wire
 * aux_data  — optional auxiliary data appended after the event ID (may be NULL)
 * aux_len   — length of aux_data in bytes (0 when aux_data is NULL)
 *
 * Returns PUS_STATUS_OK silently when no TM sink is configured.
 */
pus_status_t pus_service_5_emit(
	pus_context_t *ctx,
	pus_subtype_t  subtype,
	uint16_t       event_id,
	const uint8_t *aux_data,
	uint16_t       aux_len);

#endif /* PUS_SERVICE_5_H */
