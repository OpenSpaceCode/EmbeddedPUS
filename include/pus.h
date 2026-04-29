#ifndef PUS_H
#define PUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "pus_config.h"
#include "pus_services.h"
#include "pus_types.h"

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
