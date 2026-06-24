#include "pus_codec.h"

#include <stddef.h>

static uint16_t read_u16_be(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8u) | (uint16_t)p[1];
}

static uint32_t read_u32_be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24u) | ((uint32_t)p[1] << 16u) | ((uint32_t)p[2] << 8u) |
           (uint32_t)p[3];
}

static void write_u16_be(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8u);
    p[1] = (uint8_t)(v & 0xFFu);
}

static void write_u32_be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24u);
    p[1] = (uint8_t)(v >> 16u);
    p[2] = (uint8_t)(v >> 8u);
    p[3] = (uint8_t)(v & 0xFFu);
}

/*
 * TC secondary header wire layout (PUS_TC_SEC_HEADER_LEN = 10 bytes):
 *   [0]    version(4) | ack_flags(4)
 *   [1]    service_type_id
 *   [2]    subtype_id
 *   [3-4]  source_id       big-endian
 *   [5-8]  time            big-endian
 *   [9]    spare
 */

pus_status_t pus_tc_sec_header_decode(const uint8_t *data,
                                      uint16_t len,
                                      pus_tc_sec_header_t *header)
{
    if (data == NULL || header == NULL)
    {
        return PUS_STATUS_NULL;
    }
    if (len < PUS_TC_SEC_HEADER_LEN)
    {
        return PUS_STATUS_BAD_LENGTH;
    }

    header->version = (uint8_t)((data[0] >> 4u) & 0x0Fu);
    header->ack_flags = (uint8_t)(data[0] & 0x0Fu);
    header->service_type_id = data[1];
    header->subtype_id = data[2];
    header->source_id = read_u16_be(&data[3]);
    header->time = read_u32_be(&data[5]);
    header->spare = data[9];

    if (header->version != PUS_VERSION)
    {
        return PUS_STATUS_BAD_VERSION;
    }

    return PUS_STATUS_OK;
}

pus_status_t pus_tc_sec_header_encode(const pus_tc_sec_header_t *header,
                                      uint8_t *out,
                                      uint16_t capacity,
                                      uint16_t *out_len)
{
    if (header == NULL || out == NULL || out_len == NULL)
    {
        return PUS_STATUS_NULL;
    }
    if (capacity < PUS_TC_SEC_HEADER_LEN)
    {
        return PUS_STATUS_BUFFER_TOO_SMALL;
    }

    out[0] = (uint8_t)(((header->version & 0x0Fu) << 4u) | (header->ack_flags & 0x0Fu));
    out[1] = header->service_type_id;
    out[2] = header->subtype_id;
    write_u16_be(&out[3], header->source_id);
    write_u32_be(&out[5], header->time);
    out[9] = header->spare;

    *out_len = PUS_TC_SEC_HEADER_LEN;
    return PUS_STATUS_OK;
}

/*
 * TM secondary header wire layout (PUS_TM_SEC_HEADER_LEN = 12 bytes):
 *   [0]     version(4) | time_ref_status(4)
 *   [1]     service_type_id
 *   [2]     subtype_id
 *   [3-4]   msg_type_counter  big-endian
 *   [5-6]   destination_id    big-endian
 *   [7-10]  time              big-endian
 *   [11]    spare
 */

pus_status_t pus_tm_sec_header_decode(const uint8_t *data,
                                      uint16_t len,
                                      pus_tm_sec_header_t *header)
{
    if (data == NULL || header == NULL)
    {
        return PUS_STATUS_NULL;
    }
    if (len < PUS_TM_SEC_HEADER_LEN)
    {
        return PUS_STATUS_BAD_LENGTH;
    }

    header->version = (uint8_t)((data[0] >> 4u) & 0x0Fu);
    header->time_ref_status = (uint8_t)(data[0] & 0x0Fu);
    header->service_type_id = data[1];
    header->subtype_id = data[2];
    header->msg_type_counter = read_u16_be(&data[3]);
    header->destination_id = read_u16_be(&data[5]);
    header->time = read_u32_be(&data[7]);
    header->spare = data[11];

    return PUS_STATUS_OK;
}

pus_status_t pus_tm_sec_header_encode(const pus_tm_sec_header_t *header,
                                      uint8_t *out,
                                      uint16_t capacity,
                                      uint16_t *out_len)
{
    if (header == NULL || out == NULL || out_len == NULL)
    {
        return PUS_STATUS_NULL;
    }
    if (capacity < PUS_TM_SEC_HEADER_LEN)
    {
        return PUS_STATUS_BUFFER_TOO_SMALL;
    }

    out[0] = (uint8_t)(((header->version & 0x0Fu) << 4u) | (header->time_ref_status & 0x0Fu));
    out[1] = header->service_type_id;
    out[2] = header->subtype_id;
    write_u16_be(&out[3], header->msg_type_counter);
    write_u16_be(&out[5], header->destination_id);
    write_u32_be(&out[7], header->time);
    out[11] = header->spare;

    *out_len = PUS_TM_SEC_HEADER_LEN;
    return PUS_STATUS_OK;
}
