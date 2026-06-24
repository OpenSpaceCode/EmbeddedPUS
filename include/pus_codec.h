#ifndef PUS_CODEC_H
#define PUS_CODEC_H

#include "pus_config.h"
#include "pus_types.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Decode a raw TC secondary header from a byte buffer.
 *
 * @param[in]  data   Input byte buffer.
 * @param[in]  len    Buffer length in bytes (must be >= PUS_TC_SEC_HEADER_LEN).
 * @param[out] header Output struct.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BAD_LENGTH, PUS_STATUS_BAD_VERSION, or PUS_STATUS_OK.
 */
pus_status_t
pus_tc_sec_header_decode(const uint8_t *data, uint16_t len, pus_tc_sec_header_t *header);

/**
 * @brief Encode a TC secondary header into a byte buffer.
 *
 * @param[in]  header       Header struct to encode.
 * @param[out] out          Output byte buffer.
 * @param[in]  out_capacity Output buffer size (must be >= PUS_TC_SEC_HEADER_LEN).
 * @param[out] out_len      Receives the number of bytes written.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BUFFER_TOO_SMALL, or PUS_STATUS_OK.
 */
pus_status_t pus_tc_sec_header_encode(const pus_tc_sec_header_t *header,
                                      uint8_t                   *out,
                                      uint16_t                   out_capacity,
                                      uint16_t                  *out_len);

/**
 * @brief Decode a raw TM secondary header from a byte buffer.
 *
 * @param[in]  data   Input byte buffer.
 * @param[in]  len    Buffer length in bytes (must be >= PUS_TM_SEC_HEADER_LEN).
 * @param[out] header Output struct.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BAD_LENGTH, or PUS_STATUS_OK.
 */
pus_status_t
pus_tm_sec_header_decode(const uint8_t *data, uint16_t len, pus_tm_sec_header_t *header);

/**
 * @brief Encode a TM secondary header into a byte buffer.
 *
 * @param[in]  header       Header struct to encode.
 * @param[out] out          Output byte buffer.
 * @param[in]  out_capacity Output buffer size (must be >= PUS_TM_SEC_HEADER_LEN).
 * @param[out] out_len      Receives the number of bytes written.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BUFFER_TOO_SMALL, or PUS_STATUS_OK.
 */
pus_status_t pus_tm_sec_header_encode(const pus_tm_sec_header_t *header,
                                      uint8_t                   *out,
                                      uint16_t                   out_capacity,
                                      uint16_t                  *out_len);

#endif /* PUS_CODEC_H */
