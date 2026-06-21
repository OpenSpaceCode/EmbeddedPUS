#include "pus_handler.h"
#include <stddef.h>

pus_status_t pus_handler_register(
	pus_context_t    *ctx,
	pus_service_t     service,
	pus_subtype_t     subtype,
	pus_tc_handler_t  handler,
	void             *user_data)
{
	if (ctx == NULL || handler == NULL) {
		return PUS_STATUS_NULL;
	}

	for (uint16_t i = 0u; i < PUS_MAX_TC_HANDLERS; i++) {
		if (ctx->handler_table[i].is_used &&
		    ctx->handler_table[i].service == service &&
		    ctx->handler_table[i].subtype == subtype) {
			ctx->handler_table[i].handler   = handler;
			ctx->handler_table[i].user_data = user_data;
			return PUS_STATUS_OK;
		}
	}

	for (uint16_t i = 0u; i < PUS_MAX_TC_HANDLERS; i++) {
		if (!ctx->handler_table[i].is_used) {
			ctx->handler_table[i].service   = service;
			ctx->handler_table[i].subtype   = subtype;
			ctx->handler_table[i].handler   = handler;
			ctx->handler_table[i].user_data = user_data;
			ctx->handler_table[i].is_used   = 1u;
			ctx->handler_count++;
			return PUS_STATUS_OK;
		}
	}

	return PUS_STATUS_TABLE_FULL;
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
