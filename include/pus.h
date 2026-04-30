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
 * Decode a raw TC packet from a byte buffer.
 *
 * Validates the buffer length, decodes the TC secondary header,
 * and exposes the payload pointer and length.
 *
 * @param data      Pointer to the raw TC packet bytes
 * @param len       Length of the data buffer
 * @param tc        Output structure to fill with decoded TC packet
 * @return          PUS_STATUS_OK on success, error code on failure
 */
pus_status_t pus_tc_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tc_packet_t *tc);

/**
 * Process an incoming TC packet.
 *
 * This function:
 * 1. Decodes the TC secondary header
 * 2. Validates PUS version and service type
 * 3. Emits Service 1 verification telemetry based on ACK flags
 * 4. Routes the TC to the registered handler
 * 5. Emits completion verification telemetry
 *
 * @param ctx       PUS context with handler registry
 * @param data      Pointer to the raw TC packet bytes
 * @param len       Length of the data buffer
 * @return          PUS_STATUS_OK on success, error code on failure
 */
pus_status_t pus_tc_process(
	pus_context_t *ctx,
	const uint8_t *data,
	uint16_t len);

/**
 * Build a TM packet.
 *
 * This function:
 * 1. Validates arguments and output buffer capacity
 * 2. Fills a TM secondary header with service/subtype/destination
 * 3. Increments the message counter
 * 4. Encodes the TM secondary header
 * 5. Appends the payload
 * 6. Optionally passes result to TM sink callback
 *
 * @param ctx           PUS context with counter and sink
 * @param service       PUS service type for TM
 * @param subtype       PUS subtype for TM
 * @param destination_id Destination ID for TM
 * @param payload       Pointer to payload data (can be NULL if payload_len is 0)
 * @param payload_len   Length of payload data
 * @param out           Output buffer for encoded TM packet
 * @param out_capacity  Capacity of output buffer
 * @param out_len       Output: actual length of encoded TM packet
 * @return              PUS_STATUS_OK on success, error code on failure
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
 * Set the TM sink callback.
 *
 * @param ctx           PUS context
 * @param sink          TM sink callback function
 * @param user_data     User data passed to sink callback
 * @return              PUS_STATUS_OK on success, error code on failure
 */
pus_status_t pus_set_tm_sink(
	pus_context_t *ctx,
	pus_tm_sink_t sink,
	void *user_data);

typedef struct
{
	uint8_t *payload;
	uint16_t payload_len;
	union
	{
		pus_tm_sec_header_t tm;
		pus_tc_sec_header_t tc;
	} sec_header;
	uint8_t is_tm;
} pus_frame_t;

#if PUS_ENABLE_SPACE_PACKET_ADAPTER
#include "../external/EmbeddedSpacePacket/include/space_packet.h"

int pus_from_space_packet(const sp_packet_t *sp, pus_frame_t *pus);
int pus_to_space_packet(const pus_frame_t *pus, sp_packet_t *sp);
#endif

#endif /* PUS_H */
