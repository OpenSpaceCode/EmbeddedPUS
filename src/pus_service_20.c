#include "pus_service_20.h"
#include "pus_services.h"
#include "pus_codec.h"
#include "pus_handler.h"
#include <string.h>
#include <stddef.h>

#define MAX_OUT_LEN (PUS_TM_SEC_HEADER_LEN + PUS_MAX_TM_PAYLOAD_LEN)

static int find_param(const pus_service_20_ctx_t *s20, uint16_t param_id)
{
	for (uint8_t i = 0u; i < PUS_SERVICE_20_MAX_PARAMS; i++) {
		if (s20->params[i].is_used && s20->params[i].param_id == param_id) {
			return (int)i;
		}
	}
	return -1;
}

pus_status_t pus_service_20_init(pus_service_20_ctx_t *s20)
{
	if (s20 == NULL) {
		return PUS_STATUS_NULL;
	}
	memset(s20, 0, sizeof(*s20));
	return PUS_STATUS_OK;
}

pus_status_t pus_service_20_register_param(
	pus_service_20_ctx_t *s20,
	uint16_t              param_id,
	uint16_t              value_len,
	pus_param_getter_t    getter,
	pus_param_setter_t    setter,
	void                 *user_data)
{
	if (s20 == NULL || getter == NULL) {
		return PUS_STATUS_NULL;
	}

	/* Update existing entry */
	for (uint8_t i = 0u; i < PUS_SERVICE_20_MAX_PARAMS; i++) {
		if (s20->params[i].is_used && s20->params[i].param_id == param_id) {
			s20->params[i].value_len  = value_len;
			s20->params[i].getter     = getter;
			s20->params[i].setter     = setter;
			s20->params[i].user_data  = user_data;
			return PUS_STATUS_OK;
		}
	}

	/* Find free slot */
	for (uint8_t i = 0u; i < PUS_SERVICE_20_MAX_PARAMS; i++) {
		if (!s20->params[i].is_used) {
			s20->params[i].param_id   = param_id;
			s20->params[i].value_len  = value_len;
			s20->params[i].getter     = getter;
			s20->params[i].setter     = setter;
			s20->params[i].user_data  = user_data;
			s20->params[i].is_used    = 1u;
			return PUS_STATUS_OK;
		}
	}

	return PUS_STATUS_TABLE_FULL;
}

pus_status_t pus_service_20_emit_report(
	pus_context_t        *ctx,
	pus_service_20_ctx_t *s20,
	const uint16_t       *param_ids,
	uint8_t               count)
{
	pus_status_t        st;
	pus_tm_sec_header_t hdr;
	uint8_t             out[MAX_OUT_LEN];
	uint16_t            hdr_len;
	uint16_t            off;

	if (ctx == NULL || s20 == NULL || param_ids == NULL) {
		return PUS_STATUS_NULL;
	}

	/* Fill header fields but do NOT increment ctx->tm_counter yet — any error
	 * below must not consume a sequence number. */
	pus_tm_hdr_fill(ctx, &hdr, PUS_SERVICE_PARAMETER_MANAGEMENT,
	                PUS_SUBTYPE_PARAMETER_VALUE_REPORT,
	                ctx->default_destination_id);

	(void)pus_tm_sec_header_encode(&hdr, out, sizeof(out), &hdr_len);

	/* Payload: N (1 byte) + N × [PID (2) + value (value_len)] */
	off = hdr_len;
	out[off++] = count;

	for (uint8_t i = 0u; i < count; i++) {
		int idx = find_param(s20, param_ids[i]);
		if (idx < 0) {
			return PUS_STATUS_NO_HANDLER;
		}

		uint16_t pid       = param_ids[i];
		uint16_t value_len = s20->params[idx].value_len;

		if ((uint16_t)(off + 2u + value_len) > (uint16_t)sizeof(out)) {
			return PUS_STATUS_BUFFER_TOO_SMALL;
		}

		out[off]      = (uint8_t)(pid >> 8u);
		out[off + 1u] = (uint8_t)(pid & 0xFFu);
		off += 2u;

		st = s20->params[idx].getter(pid, &out[off], value_len, s20->params[idx].user_data);
		if (st != PUS_STATUS_OK) {
			return st;
		}
		off += value_len;
	}

	/* All params resolved successfully — now commit the counter. */
	ctx->tm_counter++;

	if (ctx->tm_sink == NULL) {
		return PUS_STATUS_OK;
	}
	return ctx->tm_sink(ctx->tm_sink_user_data, out, off);
}

static pus_status_t handle_tc_20_1(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	void                  *user_data)
{
	pus_service_20_ctx_t *s20 = (pus_service_20_ctx_t *)user_data;
	uint8_t               n;
	uint16_t              param_ids[PUS_SERVICE_20_MAX_PARAMS];

	if (tc->payload_len < 1u) {
		return PUS_STATUS_BAD_LENGTH;
	}
	n = tc->payload[0];

	if (n > PUS_SERVICE_20_MAX_PARAMS) {
		return PUS_STATUS_BAD_LENGTH;
	}
	if (tc->payload_len < (uint16_t)(1u + (uint16_t)n * 2u)) {
		return PUS_STATUS_BAD_LENGTH;
	}

	for (uint8_t i = 0u; i < n; i++) {
		param_ids[i] = ((uint16_t)tc->payload[1u + (uint16_t)i * 2u] << 8u)
		             |  (uint16_t)tc->payload[2u + (uint16_t)i * 2u];
	}

	return pus_service_20_emit_report(ctx, s20, param_ids, n);
}

static pus_status_t handle_tc_20_3(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	void                  *user_data)
{
	pus_service_20_ctx_t *s20 = (pus_service_20_ctx_t *)user_data;
	uint8_t               n;
	uint16_t              off;

	(void)ctx;

	if (tc->payload_len < 1u) {
		return PUS_STATUS_BAD_LENGTH;
	}
	n   = tc->payload[0];
	off = 1u;

	for (uint8_t i = 0u; i < n; i++) {
		if ((uint16_t)(off + 2u) > tc->payload_len) {
			return PUS_STATUS_BAD_LENGTH;
		}

		uint16_t pid = ((uint16_t)tc->payload[off] << 8u)
		             |  (uint16_t)tc->payload[off + 1u];
		off += 2u;

		int idx = find_param(s20, pid);
		if (idx < 0) {
			return PUS_STATUS_NO_HANDLER;
		}

		uint16_t value_len = s20->params[idx].value_len;

		if ((uint16_t)(off + value_len) > tc->payload_len) {
			return PUS_STATUS_BAD_LENGTH;
		}
		if (s20->params[idx].setter == NULL) {
			return PUS_STATUS_HANDLER_FAILED;
		}

		pus_status_t st = s20->params[idx].setter(
			pid, &tc->payload[off], value_len, s20->params[idx].user_data);
		if (st != PUS_STATUS_OK) {
			return st;
		}
		off += value_len;
	}

	return PUS_STATUS_OK;
}

pus_status_t pus_service_20_register_handlers(
	pus_context_t        *ctx,
	pus_service_20_ctx_t *s20)
{
	pus_status_t st;

	if (ctx == NULL || s20 == NULL) {
		return PUS_STATUS_NULL;
	}

	st = pus_handler_register(ctx,
		PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_REPORT_VALUES,
		handle_tc_20_1, s20);
	if (st != PUS_STATUS_OK) {
		return st;
	}

	return pus_handler_register(ctx,
		PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_SET_VALUES,
		handle_tc_20_3, s20);
}
