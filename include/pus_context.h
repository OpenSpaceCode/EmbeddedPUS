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

typedef pus_status_t (*pus_tm_sink_t)(
	void *user_data,
	const uint8_t *data,
	uint16_t len);

typedef struct
{
	pus_service_t service;
	pus_subtype_t subtype;
	pus_tc_handler_t handler;
	void *user_data;
	uint8_t is_used;
} pus_handler_entry_t;

/**
 * PUS configuration structure.
 * Contains compile-time and runtime configuration for the PUS context.
 */
typedef struct
{
	/** Default source ID for TC packets */
	uint16_t default_source_id;
	/** Default destination ID for TM packets */
	uint16_t default_destination_id;
	/** Maximum TC payload length */
	uint16_t max_tc_payload_len;
	/** Maximum TM payload length */
	uint16_t max_tm_payload_len;
	/** TM sink callback for automatic TM transmission */
	pus_tm_sink_t tm_sink;
	/** User data passed to TM sink */
	void *tm_sink_user_data;
} pus_config_t;

struct pus_context
{
	/** TC handler registry table */
	pus_handler_entry_t handler_table[PUS_MAX_TC_HANDLERS];
	/** Number of registered handlers */
	uint16_t handler_count;
	/** TM message type counter */
	uint16_t tm_counter;
	/** Default source ID for TC packets */
	uint16_t default_source_id;
	/** Default destination ID for TM packets */
	uint16_t default_destination_id;
	/** TM sink callback for automatic TM transmission */
	pus_tm_sink_t tm_sink;
	/** User data passed to TM sink */
	void *tm_sink_user_data;
};

pus_status_t pus_init(pus_context_t *ctx);

pus_status_t pus_init_with_config(
	pus_context_t *ctx,
	const pus_config_t *config);

#endif /* PUS_CONTEXT_H */
