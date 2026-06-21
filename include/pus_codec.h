#ifndef PUS_CODEC_H
#define PUS_CODEC_H

#include <stddef.h>
#include <stdint.h>
#include "pus_config.h"
#include "pus_types.h"
#include "pus_context.h"

/*
 * Fill TM secondary header fields that are common to every outgoing TM packet.
 * msg_type_counter is read from ctx->tm_counter but NOT incremented here —
 * callers must do ctx->tm_counter++ only after all error paths are cleared and
 * the packet is confirmed emitted. This single definition replaces the
 * duplicated 8-line block that previously appeared in every service file.
 */
static inline void pus_tm_hdr_fill(
	pus_context_t       *ctx,
	pus_tm_sec_header_t *hdr,
	pus_service_t        service,
	pus_subtype_t        subtype,
	uint16_t             dest_id)
{
	hdr->version          = PUS_VERSION;
	hdr->time_ref_status  = 0u;
	hdr->service_type_id  = service;
	hdr->subtype_id       = subtype;
	hdr->msg_type_counter = ctx->tm_counter;
	hdr->destination_id   = dest_id;
	hdr->time             = (ctx->time_source != NULL)
	                        ? ctx->time_source(ctx->time_source_user_data) : 0u;
	hdr->spare            = 0u;
}

pus_status_t pus_tc_sec_header_decode(
	const uint8_t       *data,
	uint16_t             len,
	pus_tc_sec_header_t *header);

pus_status_t pus_tc_sec_header_encode(
	const pus_tc_sec_header_t *header,
	uint8_t                   *out,
	uint16_t                   out_capacity,
	uint16_t                  *out_len);

pus_status_t pus_tm_sec_header_decode(
	const uint8_t       *data,
	uint16_t             len,
	pus_tm_sec_header_t *header);

pus_status_t pus_tm_sec_header_encode(
	const pus_tm_sec_header_t *header,
	uint8_t                   *out,
	uint16_t                   out_capacity,
	uint16_t                  *out_len);

#endif /* PUS_CODEC_H */
