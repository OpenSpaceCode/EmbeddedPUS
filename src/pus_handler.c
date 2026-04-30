#include "../include/pus_handler.h"

static int pus_handler_find_index(
	const pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype)
{
	uint16_t i = 0u;

	for (i = 0u; i < PUS_MAX_TC_HANDLERS; i++)
	{
		if (!ctx->handler_table[i].is_used)
		{
			continue;
		}
		if (ctx->handler_table[i].service != service)
		{
			continue;
		}
		if (ctx->handler_table[i].subtype == subtype)
		{
			return (int)i;
		}
	}

	return -1;
}

static int pus_handler_find_free_index(const pus_context_t *ctx)
{
	uint16_t i = 0u;

	for (i = 0u; i < PUS_MAX_TC_HANDLERS; i++)
	{
		if (!ctx->handler_table[i].is_used)
		{
			return (int)i;
		}
	}

	return -1;
}

pus_status_t pus_handler_register(
	pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype,
	pus_tc_handler_t handler,
	void *user_data)
{
	int index = -1;

	if (!ctx || !handler)
	{
		return PUS_STATUS_NULL;
	}

	index = pus_handler_find_index(ctx, service, subtype);
	if (index >= 0)
	{
		ctx->handler_table[index].handler = handler;
		ctx->handler_table[index].user_data = user_data;
		return PUS_STATUS_OK;
	}

	index = pus_handler_find_free_index(ctx);
	if (index < 0)
	{
		return PUS_STATUS_TABLE_FULL;
	}

	ctx->handler_table[index].service = service;
	ctx->handler_table[index].subtype = subtype;
	ctx->handler_table[index].handler = handler;
	ctx->handler_table[index].user_data = user_data;
	ctx->handler_table[index].is_used = 1u;
	ctx->handler_count++;

	return PUS_STATUS_OK;
}

int pus_handler_find(
	const pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype)
{
	if (!ctx)
	{
		return -1;
	}

	return pus_handler_find_index(ctx, service, subtype);
}
