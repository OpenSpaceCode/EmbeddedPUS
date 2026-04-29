#include <string.h>

#include "../include/pus_context.h"

pus_status_t pus_init(pus_context_t *ctx)
{
	if (!ctx)
	{
		return PUS_STATUS_NULL;
	}

	memset(ctx, 0, sizeof(*ctx));

	return PUS_STATUS_OK;
}
