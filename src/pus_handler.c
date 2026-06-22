#include "pus_handler.h"
#include "pus_internal.h"
#include <stddef.h>

pus_status_t pus_handler_register(
	pus_context_t    *ctx,
	pus_service_t     service,
	pus_subtype_t     subtype,
	pus_tc_handler_t  handler,
	void             *user_data)
{
	if (ctx == NULL) {
		return PUS_STATUS_NULL;
	}

	/* NULL handler means deregister: find and clear the existing entry. */
	if (handler == NULL) {
		for (uint16_t i = 0u; i < PUS_MAX_TC_HANDLERS; i++) {
			if (ctx->handler_table[i].is_used &&
			    ctx->handler_table[i].service == service &&
			    ctx->handler_table[i].subtype == subtype) {
				ctx->handler_table[i].is_used = 0u;
				return PUS_STATUS_OK;
			}
		}
		return PUS_STATUS_NO_HANDLER;
	}

	{
		int32_t first_free = -1;
		for (uint16_t i = 0u; i < PUS_MAX_TC_HANDLERS; i++) {
			if (ctx->handler_table[i].is_used) {
				if (ctx->handler_table[i].service == service &&
				    ctx->handler_table[i].subtype == subtype) {
					ctx->handler_table[i].handler   = handler;
					ctx->handler_table[i].user_data = user_data;
					return PUS_STATUS_OK;
				}
			} else if (first_free < 0) {
				first_free = (int32_t)i;
			}
		}
		if (first_free < 0) {
			return PUS_STATUS_TABLE_FULL;
		}
		ctx->handler_table[first_free].service   = service;
		ctx->handler_table[first_free].subtype   = subtype;
		ctx->handler_table[first_free].handler   = handler;
		ctx->handler_table[first_free].user_data = user_data;
		ctx->handler_table[first_free].is_used   = 1u;
		return PUS_STATUS_OK;
	}
}

int pus_handler_find(
	const pus_context_t *ctx,
	pus_service_t        service,
	pus_subtype_t        subtype)
{
	if (ctx == NULL) {
		return -1;
	}

	for (uint16_t i = 0u; i < PUS_MAX_TC_HANDLERS; i++) {
		if (ctx->handler_table[i].is_used &&
		    ctx->handler_table[i].service == service &&
		    ctx->handler_table[i].subtype == subtype) {
			return (int)i;
		}
	}

	return -1;
}

pus_status_t pus_handler_invoke(
	pus_context_t         *ctx,
	pus_service_t          service,
	pus_subtype_t          subtype,
	const pus_tc_packet_t *tc)
{
	int idx;

	if (ctx == NULL || tc == NULL) {
		return PUS_STATUS_NULL;
	}

	idx = pus_handler_find(ctx, service, subtype);
	if (idx < 0) {
		return PUS_STATUS_NO_HANDLER;
	}

	return ctx->handler_table[idx].handler(ctx, tc,
	                                       ctx->handler_table[idx].user_data);
}
