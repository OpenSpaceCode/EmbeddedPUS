#ifndef PUS_CONTEXT_H
#define PUS_CONTEXT_H

#include <stdint.h>
#include "pus_config.h"
#include "pus_types.h"

/** @brief Forward declaration so callbacks can reference the context type. */
typedef struct pus_context pus_context_t;

/**
 * @brief TC handler callback invoked by pus_tc_process() for a matched command.
 *
 * @param[in,out] ctx       Active PUS context.
 * @param[in]     tc        Decoded TC packet.
 * @param[in,out] user_data Opaque pointer supplied at registration.
 *
 * @return PUS_STATUS_OK on success; any other value triggers a completion failure report.
 */
typedef pus_status_t (*pus_tc_handler_t)(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	void                  *user_data);

/**
 * @brief TM sink callback that receives every encoded TM packet from pus_tm_build().
 *
 * @param[in,out] user_data Opaque pointer supplied at registration.
 * @param[in]     data      Encoded packet bytes.
 * @param[in]     len       Packet length in bytes.
 *
 * @return PUS_STATUS_OK on success.
 */
typedef pus_status_t (*pus_tm_sink_t)(
	void          *user_data,
	const uint8_t *data,
	uint16_t       len);

/**
 * @brief Optional time source callback.
 *
 * @param[in,out] user_data Opaque pointer supplied at registration.
 *
 * @return Current mission time (seconds or ticks). Zero is written when NULL.
 */
typedef uint32_t (*pus_time_source_t)(void *user_data);

/** @brief One entry in the TC handler dispatch table. */
typedef struct {
	pus_service_t    service;   /**< Service type identifier. */
	pus_subtype_t    subtype;   /**< Service subtype identifier. */
	pus_tc_handler_t handler;   /**< Handler function pointer. */
	void            *user_data; /**< Opaque pointer forwarded to the handler. */
	uint8_t          is_used;   /**< Non-zero when the slot is occupied. */
} pus_handler_entry_t;

/**
 * @brief Optional configuration passed to pus_init_with_config().
 * Zero-initialise and set only the fields you need.
 */
typedef struct {
	uint16_t          default_source_id;       /**< APID of this node. */
	uint16_t          default_destination_id;  /**< Default ground station APID. */
	pus_tm_sink_t     tm_sink;                 /**< TM output callback (may be NULL). */
	void             *tm_sink_user_data;        /**< Forwarded to tm_sink. */
	pus_time_source_t time_source;             /**< Timestamp callback (may be NULL). */
	void             *time_source_user_data;    /**< Forwarded to time_source. */
} pus_config_t;

/**
 * @brief Runtime state for one EmbeddedPUS instance.
 * Allocate statically or on the stack.
 * @warning Not thread-safe. If the context is accessed from more than one task
 *          or interrupt, all calls must be serialised with a mutex or critical
 *          section.
 */
struct pus_context {
	pus_handler_entry_t  handler_table[PUS_MAX_TC_HANDLERS]; /**< TC dispatch table. */
	uint16_t             tm_counter;              /**< Outgoing TM message counter. */
	uint16_t             default_source_id;       /**< APID of this node. */
	uint16_t             default_destination_id;  /**< Default destination APID. */
	pus_tm_sink_t        tm_sink;                 /**< TM output callback. */
	void                *tm_sink_user_data;        /**< Forwarded to tm_sink. */
	pus_time_source_t    time_source;             /**< Timestamp callback. */
	void                *time_source_user_data;    /**< Forwarded to time_source. */
};

/**
 * @brief Initialise a context with all-zero defaults and no TM sink.
 *
 * @param[out] ctx Context to initialise.
 *
 * @return PUS_STATUS_NULL if ctx is NULL, PUS_STATUS_OK otherwise.
 */
pus_status_t pus_init(pus_context_t *ctx);

/**
 * @brief Initialise a context and copy fields from a configuration struct.
 *
 * @param[out] ctx    Context to initialise.
 * @param[in]  config Configuration to copy from.
 *
 * @return PUS_STATUS_NULL if either pointer is NULL, PUS_STATUS_OK otherwise.
 */
pus_status_t pus_init_with_config(pus_context_t *ctx, const pus_config_t *config);

#endif /* PUS_CONTEXT_H */
