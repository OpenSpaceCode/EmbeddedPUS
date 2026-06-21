#include "pus_service_5.h"
#include "pus_services.h"
#include "pus_codec.h"
#include <stddef.h>

/* TM[5,x] payload: event_id(2) + aux_data (up to PUS_MAX_TM_PAYLOAD_LEN) */
#define EVID_LEN    2u
#define MAX_OUT_LEN (PUS_TM_SEC_HEADER_LEN + EVID_LEN + PUS_MAX_TM_PAYLOAD_LEN)

pus_status_t pus_service_5_emit(
	pus_context_t *ctx,
	pus_subtype_t  subtype,
	uint16_t       event_id,
	const uint8_t *aux_data,
	uint16_t       aux_len)
{
	pus_status_t        st;
	pus_tm_sec_header_t hdr;
	uint8_t             out[MAX_OUT_LEN];
	uint16_t            hdr_len;
	uint16_t            total_len;

	if (ctx == NULL) {
		return PUS_STATUS_NULL;
	}

	if (aux_len > PUS_MAX_TM_PAYLOAD_LEN) {
		return PUS_STATUS_BUFFER_TOO_SMALL;
	}

	hdr.version          = PUS_VERSION;
	hdr.time_ref_status  = 0u;
	hdr.service_type_id  = PUS_SERVICE_EVENT_REPORTING;
	hdr.subtype_id       = subtype;
	hdr.msg_type_counter = ctx->tm_counter++;
	hdr.destination_id   = ctx->default_destination_id;
	hdr.time             = (ctx->time_source != NULL)
	                       ? ctx->time_source(ctx->time_source_user_data)
	                       : 0u;
	hdr.spare            = 0u;

	st = pus_tm_sec_header_encode(&hdr, out, sizeof(out), &hdr_len);
	if (st != PUS_STATUS_OK) {
		return st;
	}

	out[hdr_len]      = (uint8_t)(event_id >> 8u);
	out[hdr_len + 1u] = (uint8_t)(event_id & 0xFFu);

	for (uint16_t i = 0u; i < aux_len; i++) {
		out[hdr_len + EVID_LEN + i] = aux_data[i];
	}

	total_len = hdr_len + EVID_LEN + aux_len;

	if (ctx->tm_sink == NULL) {
		return PUS_STATUS_OK;
	}
	return ctx->tm_sink(ctx->tm_sink_user_data, out, total_len);
}
