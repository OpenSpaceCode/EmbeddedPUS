#ifndef PUS_INTERNAL_H
#define PUS_INTERNAL_H

#include <stddef.h>
#include "pus_config.h"
#include "pus_context.h"
#include "pus_types.h"

/*
 * Fill the fields common to every outgoing TM secondary header.
 * msg_type_counter is copied from ctx->tm_counter but NOT incremented here;
 * callers must increment after all error paths are cleared.
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

/*
 * Returns the table index of the handler for (service, subtype), or -1 if
 * not found.  Internal use only — callers must not store or expose the index.
 */
int pus_handler_find(
	const pus_context_t *ctx,
	pus_service_t        service,
	pus_subtype_t        subtype);

#endif /* PUS_INTERNAL_H */
