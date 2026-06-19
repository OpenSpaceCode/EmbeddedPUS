#ifndef PUS_H
#define PUS_H

#include <stdint.h>
#include "pus_config.h"
#include "pus_services.h"
#include "pus_types.h"
#include "pus_codec.h"
#include "pus_context.h"
#include "pus_handler.h"

/**
 * Decode a raw TC secondary header and expose a payload view.
 * No bytes are copied; tc->payload points into the caller's buffer.
 */
pus_status_t pus_tc_decode(
	const uint8_t   *data,
	uint16_t         len,
	pus_tc_packet_t *tc);

/**
 * Decode, route, and process a raw TC buffer.
 * Emits Service 1 verification reports via the context TM sink based on ACK flags.
 */
pus_status_t pus_tc_process(
	pus_context_t *ctx,
	const uint8_t *data,
	uint16_t       len);

/**
 * Build an encoded TM secondary header plus payload into a caller-owned buffer.
 * Increments the context message counter and forwards the packet to the TM sink.
 */
pus_status_t pus_tm_build(
	pus_context_t *ctx,
	pus_service_t  service,
	pus_subtype_t  subtype,
	uint16_t       destination_id,
	const uint8_t *payload,
	uint16_t       payload_len,
	uint8_t       *out,
	uint16_t       out_capacity,
	uint16_t      *out_len);

/** Set or replace the TM sink. Pass NULL to disable automatic forwarding. */
pus_status_t pus_set_tm_sink(
	pus_context_t *ctx,
	pus_tm_sink_t  sink,
	void          *user_data);

#endif /* PUS_H */
