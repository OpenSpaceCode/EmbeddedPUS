#include "pus_service_3.h"
#include "pus_services.h"
#include "pus_codec.h"
#include <string.h>
#include <stddef.h>

/* TM[3,25/26] payload: SID(2) + provider data (up to PUS_MAX_TM_PAYLOAD_LEN) */
#define SID_LEN     2u
#define MAX_OUT_LEN (PUS_TM_SEC_HEADER_LEN + SID_LEN + PUS_MAX_TM_PAYLOAD_LEN)

static int find_entry(const pus_service_3_ctx_t *s3, uint16_t sid, uint8_t kind)
{
	for (uint8_t i = 0u; i < PUS_SERVICE_3_MAX_STRUCTURES; i++) {
		if (s3->structures[i].is_used &&
		    s3->structures[i].sid  == sid &&
		    s3->structures[i].kind == kind) {
			return (int)i;
		}
	}
	return -1;
}

static pus_status_t register_structure(
	pus_service_3_ctx_t *s3,
	uint16_t             sid,
	uint8_t              kind,
	pus_hk_provider_t    provider,
	void                *user_data)
{
	if (s3 == NULL || provider == NULL) {
		return PUS_STATUS_NULL;
	}

	for (uint8_t i = 0u; i < PUS_SERVICE_3_MAX_STRUCTURES; i++) {
		if (s3->structures[i].is_used &&
		    s3->structures[i].sid  == sid &&
		    s3->structures[i].kind == kind) {
			s3->structures[i].provider  = provider;
			s3->structures[i].user_data = user_data;
			return PUS_STATUS_OK;
		}
	}

	for (uint8_t i = 0u; i < PUS_SERVICE_3_MAX_STRUCTURES; i++) {
		if (!s3->structures[i].is_used) {
			s3->structures[i].sid       = sid;
			s3->structures[i].kind      = kind;
			s3->structures[i].provider  = provider;
			s3->structures[i].user_data = user_data;
			s3->structures[i].is_used   = 1u;
			return PUS_STATUS_OK;
		}
	}

	return PUS_STATUS_TABLE_FULL;
}

static pus_status_t emit_report(
	pus_context_t       *ctx,
	pus_service_3_ctx_t *s3,
	uint16_t             sid,
	uint8_t              kind)
{
	pus_status_t        st;
	pus_tm_sec_header_t hdr;
	uint8_t             out[MAX_OUT_LEN];
	uint16_t            hdr_len;
	uint16_t            data_len = 0u;
	int                 idx;

	if (ctx == NULL || s3 == NULL) {
		return PUS_STATUS_NULL;
	}

	idx = find_entry(s3, sid, kind);
	if (idx < 0) {
		return PUS_STATUS_NO_HANDLER;
	}

	hdr.version          = PUS_VERSION;
	hdr.time_ref_status  = 0u;
	hdr.service_type_id  = PUS_SERVICE_HOUSEKEEPING;
	hdr.subtype_id       = (kind == PUS_SERVICE_3_KIND_DIAG)
	                       ? PUS_SUBTYPE_HOUSEKEEPING_DIAGNOSTIC_REPORT
	                       : PUS_SUBTYPE_HOUSEKEEPING_PARAMETER_REPORT;
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

	out[hdr_len]     = (uint8_t)(sid >> 8u);
	out[hdr_len + 1u] = (uint8_t)(sid & 0xFFu);

	st = s3->structures[idx].provider(
		sid,
		&out[hdr_len + SID_LEN],
		(uint16_t)(sizeof(out) - hdr_len - SID_LEN),
		&data_len,
		s3->structures[idx].user_data);
	if (st != PUS_STATUS_OK) {
		return st;
	}

	if (ctx->tm_sink == NULL) {
		return PUS_STATUS_OK;
	}
	return ctx->tm_sink(ctx->tm_sink_user_data, out, (uint16_t)(hdr_len + SID_LEN + data_len));
}

pus_status_t pus_service_3_init(pus_service_3_ctx_t *s3)
{
	if (s3 == NULL) {
		return PUS_STATUS_NULL;
	}
	memset(s3, 0, sizeof(*s3));
	return PUS_STATUS_OK;
}

pus_status_t pus_service_3_register_hk(
	pus_service_3_ctx_t *s3,
	uint16_t             sid,
	pus_hk_provider_t    provider,
	void                *user_data)
{
	return register_structure(s3, sid, PUS_SERVICE_3_KIND_HK, provider, user_data);
}

pus_status_t pus_service_3_register_diag(
	pus_service_3_ctx_t *s3,
	uint16_t             sid,
	pus_hk_provider_t    provider,
	void                *user_data)
{
	return register_structure(s3, sid, PUS_SERVICE_3_KIND_DIAG, provider, user_data);
}

pus_status_t pus_service_3_emit_hk(
	pus_context_t       *ctx,
	pus_service_3_ctx_t *s3,
	uint16_t             sid)
{
	return emit_report(ctx, s3, sid, PUS_SERVICE_3_KIND_HK);
}

pus_status_t pus_service_3_emit_diag(
	pus_context_t       *ctx,
	pus_service_3_ctx_t *s3,
	uint16_t             sid)
{
	return emit_report(ctx, s3, sid, PUS_SERVICE_3_KIND_DIAG);
}

