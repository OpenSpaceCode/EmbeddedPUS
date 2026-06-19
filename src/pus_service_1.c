#include "pus_service_1.h"
#include "pus_services.h"
#include "pus_codec.h"
#include <stddef.h>

/*
 * Service 1 payload layout:
 *   Success: service(1) | subtype(1) | source_id(2)          = 4 bytes
 *   Failure: service(1) | subtype(1) | source_id(2) | code(2) = 6 bytes
 */
#define SUCCESS_PAYLOAD_LEN 4u
#define FAILURE_PAYLOAD_LEN 6u
#define REPORT_BUF_LEN (PUS_TM_SEC_HEADER_LEN + FAILURE_PAYLOAD_LEN)

static pus_status_t build_report(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype,
	uint16_t               failure_code,
	uint8_t                include_failure_code,
	uint8_t               *out,
	uint16_t               capacity,
	uint16_t              *out_len)
{
	pus_status_t        st;
	pus_tm_sec_header_t hdr;
	uint16_t            hdr_len;
	uint16_t            payload_len;
	uint8_t             payload[FAILURE_PAYLOAD_LEN];
	uint16_t            source_id;

	if (ctx == NULL || tc == NULL || out == NULL || out_len == NULL) {
		return PUS_STATUS_NULL;
	}

	payload_len = include_failure_code ? FAILURE_PAYLOAD_LEN : SUCCESS_PAYLOAD_LEN;

	if (capacity < (uint16_t)(PUS_TM_SEC_HEADER_LEN + payload_len)) {
		return PUS_STATUS_BUFFER_TOO_SMALL;
	}

	hdr.version          = PUS_VERSION;
	hdr.time_ref_status  = 0u;
	hdr.service_type_id  = PUS_SERVICE_REQUEST_VERIFICATION;
	hdr.subtype_id       = subtype;
	hdr.msg_type_counter = ctx->tm_counter++;
	hdr.destination_id   = tc->sec_header.source_id;
	hdr.time             = (ctx->time_source != NULL)
	                       ? ctx->time_source(ctx->time_source_user_data)
	                       : 0u;
	hdr.spare            = 0u;

	st = pus_tm_sec_header_encode(&hdr, out, capacity, &hdr_len);
	if (st != PUS_STATUS_OK) {
		return st;
	}

	source_id    = tc->sec_header.source_id;
	payload[0]   = tc->sec_header.service_type_id;
	payload[1]   = tc->sec_header.subtype_id;
	payload[2]   = (uint8_t)(source_id >> 8u);
	payload[3]   = (uint8_t)(source_id & 0xFFu);

	if (include_failure_code) {
		payload[4] = (uint8_t)(failure_code >> 8u);
		payload[5] = (uint8_t)(failure_code & 0xFFu);
	}

	for (uint16_t i = 0u; i < payload_len; i++) {
		out[hdr_len + i] = payload[i];
	}

	*out_len = hdr_len + payload_len;
	return PUS_STATUS_OK;
}

pus_status_t pus_service_1_build_success(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype,
	uint8_t               *out,
	uint16_t               capacity,
	uint16_t              *out_len)
{
	return build_report(ctx, tc, subtype, 0u, 0u, out, capacity, out_len);
}

pus_status_t pus_service_1_build_failure(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype,
	uint16_t               failure_code,
	uint8_t               *out,
	uint16_t               capacity,
	uint16_t              *out_len)
{
	return build_report(ctx, tc, subtype, failure_code, 1u, out, capacity, out_len);
}

pus_status_t pus_service_1_emit_success(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype)
{
	uint8_t  buf[REPORT_BUF_LEN];
	uint16_t len;
	pus_status_t st;

	if (ctx == NULL || ctx->tm_sink == NULL) {
		return PUS_STATUS_OK;
	}

	st = pus_service_1_build_success(ctx, tc, subtype, buf, sizeof(buf), &len);
	if (st != PUS_STATUS_OK) {
		return st;
	}

	return ctx->tm_sink(ctx->tm_sink_user_data, buf, len);
}

pus_status_t pus_service_1_emit_failure(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	pus_subtype_t          subtype,
	uint16_t               failure_code)
{
	uint8_t  buf[REPORT_BUF_LEN];
	uint16_t len;
	pus_status_t st;

	if (ctx == NULL || ctx->tm_sink == NULL) {
		return PUS_STATUS_OK;
	}

	st = pus_service_1_build_failure(ctx, tc, subtype, failure_code,
	                                 buf, sizeof(buf), &len);
	if (st != PUS_STATUS_OK) {
		return st;
	}

	return ctx->tm_sink(ctx->tm_sink_user_data, buf, len);
}
