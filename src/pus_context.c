#include <string.h>

#include "../include/pus_context.h"

pus_status_t pus_init(pus_context_t *ctx)
{
	pus_config_t default_config = {0};

	return pus_init_with_config(ctx, &default_config);
}

pus_status_t pus_init_with_config(
	pus_context_t *ctx,
	const pus_config_t *config)
{
	if (!ctx)
	{
		return PUS_STATUS_NULL;
	}

	memset(ctx, 0, sizeof(*ctx));

	/* Apply configuration if provided */
	if (config)
	{
		ctx->default_source_id = config->default_source_id;
		ctx->default_destination_id = config->default_destination_id;
		ctx->tm_sink = config->tm_sink;
		ctx->tm_sink_user_data = config->tm_sink_user_data;
	}

	return PUS_STATUS_OK;
}
