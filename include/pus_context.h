#ifndef PUS_CONTEXT_H
#define PUS_CONTEXT_H

#include <stdint.h>

#include "pus_config.h"
#include "pus_types.h"

/**
 * @file pus_context.h
 * @brief Runtime context, callbacks, and initialization API for EmbeddedPUS.
 *
 * A context owns the TC handler registry, the TM message counter, default
 * routing identifiers, and the optional telemetry sink callback. Initialize a
 * context with pus_init() or pus_init_with_config() before using it with any
 * other API.
 */

/**
 * @brief Runtime state handle used by the PUS library.
 *
 * The structure is defined in this header so applications can allocate it
 * statically, on the stack, or inside another object. After initialization,
 * callers should prefer the public APIs instead of editing fields directly.
 */
typedef struct pus_context pus_context_t;

/**
 * @brief Application callback invoked for a routed telecommand.
 *
 * @param[in,out] ctx       PUS context that is processing the command.
 * @param[in]     tc        Decoded TC packet view. The payload points into the
 *                          input buffer passed to pus_tc_process().
 * @param[in,out] user_data Opaque pointer supplied during handler registration.
 *
 * @return @ref PUS_STATUS_OK when the command was handled successfully, or an
 *         error status to propagate back to pus_tc_process().
 */
typedef pus_status_t (*pus_tc_handler_t)(
	pus_context_t *ctx,
	const pus_tc_packet_t *tc,
	void *user_data);

/**
 * @brief Application callback used to consume an encoded telemetry packet.
 *
 * The sink is called by pus_tm_build() after the TM header and payload have
 * been encoded into the caller-provided output buffer.
 *
 * @param[in,out] user_data Opaque pointer supplied with pus_set_tm_sink() or
 *                          pus_init_with_config().
 * @param[in]     data      Encoded TM packet bytes.
 * @param[in]     len       Number of valid bytes in @p data.
 *
 * @return @ref PUS_STATUS_OK on success, or another status to return from
 *         pus_tm_build().
 */
typedef pus_status_t (*pus_tm_sink_t)(
	void *user_data,
	const uint8_t *data,
	uint16_t len);

/**
 * @brief Entry in the telecommand handler registry.
 *
 * Each used entry maps one service/subtype pair to an application callback and
 * its opaque user data. Entries are managed by pus_handler_register().
 */
typedef struct
{
	pus_service_t service;       /**< Registered PUS service type. */
	pus_subtype_t subtype;       /**< Registered service-specific subtype. */
	pus_tc_handler_t handler;    /**< Callback invoked for matching TCs. */
	void *user_data;             /**< Opaque pointer passed to the registered handler. */
	uint8_t is_used;             /**< Nonzero when this registry slot is active. */
} pus_handler_entry_t;

/**
 * @brief Runtime configuration used when initializing a PUS context.
 *
 * Passing NULL to pus_init_with_config() is valid and produces the same zeroed
 * defaults as pus_init(). The TM sink fields configure automatic transmission
 * of telemetry produced by pus_tm_build().
 */
typedef struct
{
	/** Default source ID copied to the context for application use. */
	uint16_t default_source_id;
	/** Default destination ID copied to the context for application use. */
	uint16_t default_destination_id;
	/** Maximum TC payload length intended for application-level validation. */
	uint16_t max_tc_payload_len;
	/** Maximum TM payload length intended for application-level validation. */
	uint16_t max_tm_payload_len;
	/** TM sink callback for automatic TM transmission. */
	pus_tm_sink_t tm_sink;
	/** User data passed to the TM sink callback. */
	void *tm_sink_user_data;
} pus_config_t;

/**
 * @brief Runtime state for an EmbeddedPUS instance.
 *
 * The context is intentionally self-contained and has no dynamic allocation.
 * Use pus_init() or pus_init_with_config() to clear and configure it before
 * registering handlers or processing packets.
 */
struct pus_context
{
	/** TC handler registry table. */
	pus_handler_entry_t handler_table[PUS_MAX_TC_HANDLERS];
	/** Number of active entries in pus_context::handler_table. */
	uint16_t handler_count;
	/** TM message counter copied into each TM secondary header and then incremented. */
	uint16_t tm_counter;
	/** Default source ID available to application code. */
	uint16_t default_source_id;
	/** Default destination ID available to application code. */
	uint16_t default_destination_id;
	/** TM sink callback for automatic TM transmission. */
	pus_tm_sink_t tm_sink;
	/** User data passed to pus_context::tm_sink. */
	void *tm_sink_user_data;
};

/**
 * @brief Initialize a PUS context with zeroed default configuration.
 *
 * This is equivalent to calling pus_init_with_config() with a zero-filled
 * configuration object.
 *
 * @param[out] ctx Context to initialize.
 *
 * @retval PUS_STATUS_OK   Context was initialized.
 * @retval PUS_STATUS_NULL @p ctx was NULL.
 */
pus_status_t pus_init(pus_context_t *ctx);

/**
 * @brief Initialize a PUS context from a runtime configuration.
 *
 * The context is cleared before source/destination defaults and TM sink
 * settings are copied from @p config.
 *
 * @param[out] ctx    Context to initialize.
 * @param[in]  config Optional configuration. When NULL, zeroed defaults are
 *                    used.
 *
 * @retval PUS_STATUS_OK   Context was initialized.
 * @retval PUS_STATUS_NULL @p ctx was NULL.
 */
pus_status_t pus_init_with_config(
	pus_context_t *ctx,
	const pus_config_t *config);

#endif /* PUS_CONTEXT_H */
