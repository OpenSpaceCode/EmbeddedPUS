#ifndef PUS_SERVICE_17_H
#define PUS_SERVICE_17_H

#include "pus_context.h"
#include "pus_types.h"

#include <stdint.h>

/**
 * @brief Emit TM[17,2] are-you-alive report (ECSS §8.17.3.2).
 * Payload is empty.
 *
 * @param[in,out] ctx            Active PUS context (provides TM sink).
 * @param[in]     destination_id Destination APID for the report.
 *
 * @return PUS_STATUS_NULL or PUS_STATUS_OK.
 */
pus_status_t pus_service_17_emit_alive_report(pus_context_t *ctx, uint16_t destination_id);

/**
 * @brief Emit TM[17,4] on-board connection test report (ECSS §8.17.3.4).
 * Payload: tested application process APID (2 bytes).
 *
 * @param[in,out] ctx            Active PUS context (provides TM sink).
 * @param[in]     apid           Tested application process ID echoed in the payload.
 * @param[in]     destination_id Destination APID for the report.
 *
 * @return PUS_STATUS_NULL or PUS_STATUS_OK.
 */
pus_status_t pus_service_17_emit_connection_report(pus_context_t *ctx,
                                                   uint16_t apid,
                                                   uint16_t destination_id);

/**
 * @brief Register TC[17,1] and TC[17,3] handlers with the PUS context.
 * Handlers reply directly to the source_id of the incoming TC.
 *
 * @param[in,out] ctx Active PUS context.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_TABLE_FULL, or PUS_STATUS_OK.
 */
pus_status_t pus_service_17_register_handlers(pus_context_t *ctx);

#endif /* PUS_SERVICE_17_H */
