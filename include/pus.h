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
