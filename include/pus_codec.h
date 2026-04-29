#ifndef PUS_CODEC_H
#define PUS_CODEC_H

#include <stdint.h>

#include "pus_types.h"

pus_status_t pus_tc_sec_header_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tc_sec_header_t *header);

pus_status_t pus_tc_sec_header_encode(
	const pus_tc_sec_header_t *header,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len);

pus_status_t pus_tm_sec_header_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tm_sec_header_t *header);

pus_status_t pus_tm_sec_header_encode(
	const pus_tm_sec_header_t *header,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len);

#endif /* PUS_CODEC_H */
