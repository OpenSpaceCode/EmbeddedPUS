#ifndef PUS_HANDLER_H
#define PUS_HANDLER_H

#include "pus_context.h"

pus_status_t pus_handler_register(
	pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype,
	pus_tc_handler_t handler,
	void *user_data);

#endif /* PUS_HANDLER_H */
