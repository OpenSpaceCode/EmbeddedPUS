#ifndef PUS_CONTEXT_H
#define PUS_CONTEXT_H

#include <stdint.h>
#include "pus_config.h"
#include "pus_types.h"

/** @brief Forward declaration so callbacks can reference the context type. */
typedef struct pus_context pus_context_t;

/** @brief Handler callback invoked by pus_tc_process() for a matched TC. */
typedef pus_status_t (*pus_tc_handler_t)(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	void                  *user_data);

/** @brief Sink callback that receives every encoded TM packet from pus_tm_build(). */
typedef pus_status_t (*pus_tm_sink_t)(
	void          *user_data,
	const uint8_t *data,
	uint16_t       len);

/**
 * Optional time source callback. Returns a 32-bit timestamp (seconds or
 * mission ticks). If NULL, pus_tm_build() writes zero into the time field.
 */
typedef uint32_t (*pus_time_source_t)(void *user_data);

/** @brief One entry in the TC handler dispatch table. */
typedef struct {
	pus_service_t    service;
	pus_subtype_t    subtype;
	pus_tc_handler_t handler;
	void            *user_data;
	uint8_t          is_used;
} pus_handler_entry_t;

/** @brief Optional configuration passed to pus_init_with_config(). */
typedef struct {
	uint16_t          default_source_id;
	uint16_t          default_destination_id;
	pus_tm_sink_t     tm_sink;
	void             *tm_sink_user_data;
	pus_time_source_t time_source;
	void             *time_source_user_data;
} pus_config_t;

/** @brief Runtime state for an EmbeddedPUS instance. Allocate statically. */
struct pus_context {
	pus_handler_entry_t  handler_table[PUS_MAX_TC_HANDLERS];
	uint16_t             handler_count;
	uint16_t             tm_counter;
	uint16_t             default_source_id;
	uint16_t             default_destination_id;
	pus_tm_sink_t        tm_sink;
	void                *tm_sink_user_data;
	pus_time_source_t    time_source;
	void                *time_source_user_data;
};

pus_status_t pus_init(pus_context_t *ctx);
pus_status_t pus_init_with_config(pus_context_t *ctx, const pus_config_t *config);

#endif /* PUS_CONTEXT_H */
