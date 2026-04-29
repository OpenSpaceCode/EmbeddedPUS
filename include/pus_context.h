#ifndef PUS_CONTEXT_H
#define PUS_CONTEXT_H

#include <stdint.h>

#include "pus_config.h"
#include "pus_types.h"

typedef struct pus_context pus_context_t;

typedef pus_status_t (*pus_tc_handler_t)(
	pus_context_t *ctx,
	const pus_tc_packet_t *tc,
	void *user_data);

typedef struct
{
	pus_service_t service;
	pus_subtype_t subtype;
	pus_tc_handler_t handler;
	void *user_data;
	uint8_t is_used;
} pus_handler_entry_t;

struct pus_context
{
	pus_handler_entry_t handler_table[PUS_MAX_TC_HANDLERS];
	uint16_t handler_count;
};

pus_status_t pus_init(pus_context_t *ctx);

#endif /* PUS_CONTEXT_H */
