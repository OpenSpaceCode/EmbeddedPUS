#ifndef PUS_SERVICE_1_H
#define PUS_SERVICE_1_H

#include <stdint.h>
#include "pus_context.h"
#include "pus_types.h"

/** ACK flag bits in TC secondary header ack_flags. */
#define PUS_ACK_ACCEPTANCE 0x08u
#define PUS_ACK_START      0x04u
#define PUS_ACK_PROGRESS   0x02u
#define PUS_ACK_COMPLETION 0x01u

/**
 * Build an encoded Service 1 success report (TM[1,1], TM[1,3], TM[1,7])
 * into a caller-provided buffer. Use subtype constants from pus_services.h.
 */
pus_status_t pus_service_1_build_success(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype,
	uint8_t               *out,
	uint16_t               capacity,
	uint16_t              *out_len);

/**
 * Build an encoded Service 1 failure report (TM[1,2], TM[1,4], TM[1,8], TM[1,10])
 * into a caller-provided buffer.
 */
pus_status_t pus_service_1_build_failure(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype,
	uint16_t               failure_code,
	uint8_t               *out,
	uint16_t               capacity,
	uint16_t              *out_len);

/**
 * Build and forward a success report to ctx->tm_sink.
 * Returns PUS_STATUS_OK silently when no sink is configured.
 */
pus_status_t pus_service_1_emit_success(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype);

/**
 * Build and forward a failure report to ctx->tm_sink.
 * Returns PUS_STATUS_OK silently when no sink is configured.
 */
pus_status_t pus_service_1_emit_failure(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype,
	uint16_t               failure_code);

#endif /* PUS_SERVICE_1_H */
