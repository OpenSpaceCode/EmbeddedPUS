#ifndef PUS_SERVICE_5_H
#define PUS_SERVICE_5_H

#include <stdint.h>
#include "pus_context.h"
#include "pus_types.h"

/**
 * @brief Build and emit a ST[05] event report (TM[5,x]).
 * Returns PUS_STATUS_OK silently when no TM sink is configured.
 *
 * @param[in,out] ctx      Active PUS context (provides TM sink).
 * @param[in]     subtype  Event severity; one of PUS_SUBTYPE_EVENT_* from pus_services.h.
 * @param[in]     event_id ECSS Event Definition ID (EVID), written as 2 bytes on the wire.
 * @param[in]     aux_data Optional auxiliary data appended after the event ID (may be NULL).
 * @param[in]     aux_len  Length of aux_data in bytes; must be 0 when aux_data is NULL.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BUFFER_TOO_SMALL, or PUS_STATUS_OK.
 */
pus_status_t pus_service_5_emit(
	pus_context_t *ctx,
	pus_subtype_t  subtype,
	uint16_t       event_id,
	const uint8_t *aux_data,
	uint16_t       aux_len);

#endif /* PUS_SERVICE_5_H */
