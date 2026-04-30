#ifndef PUS_H
#define PUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "pus_config.h"
#include "pus_services.h"
#include "pus_types.h"
#include "pus_codec.h"
#include "pus_context.h"
#include "pus_handler.h"

/**
 * @file pus.h
 * @brief Umbrella public header for the EmbeddedPUS library.
 *
 * Include this header when an application wants the complete public API:
 * configuration constants, service identifiers, packet types, codec helpers,
 * context management, handler registration, and top-level TC/TM functions.
 */

/**
 * @brief Decode a raw TC packet from a byte buffer.
 *
 * The function validates the minimum TC secondary header length, decodes the
 * secondary header, verifies the PUS version, and exposes a payload view into
 * the caller-provided buffer. No packet bytes are copied.
 *
 * @param[in]  data Pointer to raw TC packet bytes.
 * @param[in]  len  Length of @p data in bytes.
 * @param[out] tc   Decoded TC packet view.
 *
 * @retval PUS_STATUS_OK         Packet decoded successfully.
 * @retval PUS_STATUS_NULL       @p data or @p tc was NULL.
 * @retval PUS_STATUS_BAD_LENGTH @p len is shorter than @ref PUS_TC_SEC_HEADER_LEN.
 * @retval PUS_STATUS_BAD_VERSION Decoded version does not match @ref PUS_VERSION.
 */
pus_status_t pus_tc_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tc_packet_t *tc);

/**
 * @brief Decode and route an incoming TC packet to a registered handler.
 *
 * The TC is decoded with pus_tc_decode(), then the service/subtype pair is
 * looked up in the context handler registry. The matching handler receives a
 * decoded packet view whose payload points into the input buffer.
 *
 * @param[in,out] ctx  Initialized PUS context with a handler registry.
 * @param[in]     data Pointer to raw TC packet bytes.
 * @param[in]     len  Length of @p data in bytes.
 *
 * @retval PUS_STATUS_OK         Handler completed successfully.
 * @retval PUS_STATUS_NULL       @p ctx or @p data was NULL.
 * @retval PUS_STATUS_BAD_LENGTH TC decode failed because the buffer was short.
 * @retval PUS_STATUS_BAD_VERSION TC decode failed because the version was unsupported.
 * @retval PUS_STATUS_NO_HANDLER No handler matched the decoded service/subtype.
 * @retval PUS_STATUS_HANDLER_FAILED Typical failure status returned by an
 *                                   application handler.
 *
 * Other non-OK statuses returned by the registered handler may also be
 * propagated to the caller.
 */
pus_status_t pus_tc_process(
	pus_context_t *ctx,
	const uint8_t *data,
	uint16_t len);

/**
 * @brief Build an encoded TM packet into a caller-provided buffer.
 *
 * The function creates a TM secondary header from the supplied service,
 * subtype, destination ID, and the context message counter, appends the
 * optional payload, stores the encoded length, increments the message counter,
 * and forwards the encoded packet to the configured TM sink when one is set.
 *
 * @param[in,out] ctx            Initialized PUS context with counter and sink.
 * @param[in]     service        PUS service type for the telemetry packet.
 * @param[in]     subtype        Service-specific telemetry subtype.
 * @param[in]     destination_id Destination identifier written to the TM header.
 * @param[in]     payload        Payload bytes. Must be non-NULL when
 *                              @p payload_len is nonzero.
 * @param[in]     payload_len    Payload length in bytes.
 * @param[out]    out            Output buffer for the encoded TM packet.
 * @param[in]     out_capacity   Capacity of @p out in bytes.
 * @param[out]    out_len        Encoded packet length on success.
 *
 * @retval PUS_STATUS_OK               TM packet was built and, if configured,
 *                                     accepted by the sink.
 * @retval PUS_STATUS_NULL             A required pointer argument was NULL.
 * @retval PUS_STATUS_BUFFER_TOO_SMALL @p out_capacity cannot hold header plus payload.
 *
 * Other non-OK statuses returned by the configured TM sink may also be
 * propagated to the caller.
 */
pus_status_t pus_tm_build(
	pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype,
	uint16_t destination_id,
	const uint8_t *payload,
	uint16_t payload_len,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len);

/**
 * @brief Set or replace the TM sink callback for a context.
 *
 * Pass NULL for @p sink to disable automatic sink delivery from pus_tm_build().
 *
 * @param[in,out] ctx       Initialized PUS context.
 * @param[in]     sink      TM sink callback, or NULL to clear the sink.
 * @param[in,out] user_data Opaque pointer passed to @p sink.
 *
 * @retval PUS_STATUS_OK   Sink configuration was updated.
 * @retval PUS_STATUS_NULL @p ctx was NULL.
 */
pus_status_t pus_set_tm_sink(
	pus_context_t *ctx,
	pus_tm_sink_t sink,
	void *user_data);

/**
 * @brief Generic PUS frame view used by the optional space packet adapter.
 *
 * The pus_frame_t::is_tm flag selects which secondary header variant in the
 * union is valid. The payload pointer is borrowed; the frame does not own or
 * copy the payload bytes.
 */
typedef struct
{
	uint8_t *payload;	  /**< Payload bytes associated with the frame. */
	uint16_t payload_len; /**< Payload length in bytes. */
	/** @brief Secondary header variant selected by pus_frame_t::is_tm. */
	union
	{
		pus_tm_sec_header_t tm; /**< Telemetry secondary header when pus_frame_t::is_tm is nonzero. */
		pus_tc_sec_header_t tc; /**< Telecommand secondary header when pus_frame_t::is_tm is zero. */
	} sec_header;
	uint8_t is_tm; /**< Nonzero for TM frames, zero for TC frames. */
} pus_frame_t;

#if PUS_ENABLE_SPACE_PACKET_ADAPTER
#include "../external/EmbeddedSpacePacket/include/space_packet.h"

/**
 * @brief Convert an CCSDS SpacePacket packet into a generic PUS frame view.
 *
 * @param[in]  sp  Space packet to inspect.
 * @param[out] pus Output PUS frame view.
 *
 * @return 1 when conversion succeeds, 0 when arguments are NULL, the space
 *         packet has no secondary header, the header is too short, or the PUS
 *         version is unsupported.
 */
int pus_from_space_packet(const sp_packet_t *sp, pus_frame_t *pus);

/**
 * @brief Populate an CCSDS SpacePacket packet from a generic PUS frame.
 *
 * @param[in]  pus PUS frame view to convert.
 * @param[out] sp  Space packet to initialize and populate.
 *
 * @return 1 when conversion succeeds, or 0 when @p pus or @p sp is NULL.
 */
int pus_to_space_packet(const pus_frame_t *pus, sp_packet_t *sp);
#endif

#endif /* PUS_H */
