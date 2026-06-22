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
 * @brief Decode a raw TC secondary header and expose a zero-copy payload view.
 *
 * @param[in]  data Input byte buffer.
 * @param[in]  len  Buffer length in bytes.
 * @param[out] tc   Decoded packet; payload points into data.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BAD_LENGTH, PUS_STATUS_BAD_VERSION, or PUS_STATUS_OK.
 */
pus_status_t pus_tc_decode(
	const uint8_t   *data,
	uint16_t         len,
	pus_tc_packet_t *tc);

/**
 * @brief Decode, route, and process a raw TC buffer.
 * Emits ST[01] verification reports according to the TC ack_flags.
 * Unrecognised commands receive a TM[1,10] routing failure report.
 *
 * @param[in,out] ctx  Active PUS context.
 * @param[in]     data Incoming TC byte buffer.
 * @param[in]     len  Buffer length in bytes.
 *
 * @return Handler return value, or PUS_STATUS_NO_HANDLER if no match was found.
 */
pus_status_t pus_tc_process(
	pus_context_t *ctx,
	const uint8_t *data,
	uint16_t       len);

/**
 * @brief Build an encoded TM packet into a caller-owned buffer and forward it to ctx->tm_sink.
 * Increments the message counter on success.
 * @note If payload is NULL and payload_len > 0, the payload region is zero-filled.
 *
 * @param[in,out] ctx          Active PUS context.
 * @param[in]     service      Service type identifier.
 * @param[in]     subtype      Service subtype identifier.
 * @param[in]     destination_id Destination APID written into the TM header.
 * @param[in]     payload      Application payload bytes (may be NULL to zero-fill).
 * @param[in]     payload_len  Payload length in bytes.
 * @param[out]    out          Caller-supplied output buffer.
 * @param[in]     out_capacity Output buffer size in bytes.
 * @param[out]    out_len      Receives the number of bytes written.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BUFFER_TOO_SMALL, or PUS_STATUS_OK.
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

/**
 * @brief Set or replace the TM sink. Pass NULL to disable forwarding.
 *
 * @param[in,out] ctx       Active PUS context.
 * @param[in]     sink      New TM sink callback (may be NULL).
 * @param[in]     user_data Opaque pointer forwarded to the sink on every call.
 *
 * @return PUS_STATUS_NULL if ctx is NULL, PUS_STATUS_OK otherwise.
 */
pus_status_t pus_set_tm_sink(
	pus_context_t *ctx,
	pus_tm_sink_t  sink,
	void          *user_data);

#endif /* PUS_H */
