#ifndef PUS_CODEC_H
#define PUS_CODEC_H

#include <stdint.h>

#include "pus_types.h"

/**
 * @file pus_codec.h
 * @brief Encoding and decoding helpers for PUS secondary headers.
 *
 * These functions translate between the public header structures and the
 * fixed-size wire-format byte sequences used by EmbeddedPUS. Multi-byte
 * integer fields are encoded and decoded in big-endian order.
 */

/**
 * @brief Decode a TC secondary header from wire-format bytes.
 *
 * @param[in]  data   Buffer containing at least @ref PUS_TC_SEC_HEADER_LEN
 *                    bytes of TC secondary header data.
 * @param[in]  len    Number of bytes available in @p data.
 * @param[out] header Decoded TC secondary header fields.
 *
 * @retval PUS_STATUS_OK         Header decoded successfully.
 * @retval PUS_STATUS_NULL       @p data or @p header was NULL.
 * @retval PUS_STATUS_BAD_LENGTH @p len is shorter than the TC secondary header.
 */
pus_status_t pus_tc_sec_header_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tc_sec_header_t *header);

/**
 * @brief Encode a TC secondary header into wire-format bytes.
 *
 * @param[in]  header       TC secondary header fields to encode.
 * @param[out] out          Output buffer that receives encoded bytes.
 * @param[in]  out_capacity Capacity of @p out in bytes.
 * @param[out] out_len      Number of bytes written on success.
 *
 * @retval PUS_STATUS_OK               Header encoded successfully.
 * @retval PUS_STATUS_NULL             A required pointer argument was NULL.
 * @retval PUS_STATUS_BUFFER_TOO_SMALL @p out_capacity is too small.
 */
pus_status_t pus_tc_sec_header_encode(
	const pus_tc_sec_header_t *header,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len);

/**
 * @brief Decode a TM secondary header from wire-format bytes.
 *
 * @param[in]  data   Buffer containing at least @ref PUS_TM_SEC_HEADER_LEN
 *                    bytes of TM secondary header data.
 * @param[in]  len    Number of bytes available in @p data.
 * @param[out] header Decoded TM secondary header fields.
 *
 * @retval PUS_STATUS_OK         Header decoded successfully.
 * @retval PUS_STATUS_NULL       @p data or @p header was NULL.
 * @retval PUS_STATUS_BAD_LENGTH @p len is shorter than the TM secondary header.
 */
pus_status_t pus_tm_sec_header_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tm_sec_header_t *header);

/**
 * @brief Encode a TM secondary header into wire-format bytes.
 *
 * @param[in]  header       TM secondary header fields to encode.
 * @param[out] out          Output buffer that receives encoded bytes.
 * @param[in]  out_capacity Capacity of @p out in bytes.
 * @param[out] out_len      Number of bytes written on success.
 *
 * @retval PUS_STATUS_OK               Header encoded successfully.
 * @retval PUS_STATUS_NULL             A required pointer argument was NULL.
 * @retval PUS_STATUS_BUFFER_TOO_SMALL @p out_capacity is too small.
 */
pus_status_t pus_tm_sec_header_encode(
	const pus_tm_sec_header_t *header,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len);

#endif /* PUS_CODEC_H */
