#include <string.h>
#include <stdint.h>
#include "../include/pus.h"
#include "../include/pus_config.h"

int pus_from_space_packet(const sp_packet_t *sp, pus_frame_t *pus) {
	if (!sp || !pus) {
		return 0;
	}

	if (!sp->ph.sec_hdr_flag || sp->sec_hdr_len == 0) {
		return 0;
	}

	pus->is_tm = (sp->ph.type == 0) ? 1 : 0;
	pus->payload = (uint8_t *)(uintptr_t)sp->payload;
	pus->payload_len = sp->payload_len;

	if (pus->is_tm) {
		if (sp->sec_hdr_len < sizeof(pus_tm_sec_header_t)) {
			return 0;
		}
		const pus_tm_sec_header_t *tm = (const pus_tm_sec_header_t *)sp->sec_hdr;
		if (tm->version != PUS_VERSION) {
			return 0;
		}
		memcpy(&pus->sec_header.tm, sp->sec_hdr, sizeof(pus_tm_sec_header_t));
	} else {
		if (sp->sec_hdr_len < sizeof(pus_tc_sec_header_t)) {
			return 0;
		}
		const pus_tc_sec_header_t *tc = (const pus_tc_sec_header_t *)sp->sec_hdr;
		if (tc->version != PUS_VERSION) {
			return 0;
		}
		memcpy(&pus->sec_header.tc, sp->sec_hdr, sizeof(pus_tc_sec_header_t));
	}

	return 1;
}

int pus_to_space_packet(const pus_frame_t *pus, sp_packet_t *sp) {
	if (!pus || !sp) {
		return 0;
	}

	sp_packet_init(sp);

	uint8_t type = pus->is_tm ? 0 : 1;
	sp_set_primary_header(sp, PUS_VERSION, type, 1, 0, SP_SEQ_FLAG_UNSEGMENTED, 0);

	uint16_t sec_hdr_len = pus->is_tm ? sizeof(pus_tm_sec_header_t) :
	                                     sizeof(pus_tc_sec_header_t);

	const uint8_t *sec_hdr = (const uint8_t *)&pus->sec_header;
	sp_set_secondary_header(sp, sec_hdr, sec_hdr_len);
	sp_set_payload(sp, pus->payload, pus->payload_len);

	return 1;
}
