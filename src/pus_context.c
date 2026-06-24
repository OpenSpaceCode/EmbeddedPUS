#include "pus_context.h"

#include <stddef.h>
#include <string.h>

pus_status_t pus_init(pus_context_t *ctx)
{
    if (ctx == NULL)
    {
        return PUS_STATUS_NULL;
    }
    memset(ctx, 0, sizeof(*ctx));
    return PUS_STATUS_OK;
}

pus_status_t pus_init_with_config(pus_context_t *ctx, const pus_config_t *config)
{
    pus_status_t st = pus_init(ctx);
    if (st != PUS_STATUS_OK)
    {
        return st;
    }
    if (config == NULL)
    {
        return PUS_STATUS_NULL;
    }

    ctx->default_source_id      = config->default_source_id;
    ctx->default_destination_id = config->default_destination_id;
    ctx->tm_sink                = config->tm_sink;
    ctx->tm_sink_user_data      = config->tm_sink_user_data;
    ctx->time_source            = config->time_source;
    ctx->time_source_user_data  = config->time_source_user_data;

    return PUS_STATUS_OK;
}
