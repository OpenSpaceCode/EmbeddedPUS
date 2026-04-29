#include "../include/pus_codec.h"

static uint16_t pus_read_u16_be(const uint8_t *data)
{
	return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static uint32_t pus_read_u32_be(const uint8_t *data)
{
	return ((uint32_t)data[0] << 24) |
		   ((uint32_t)data[1] << 16) |
		   ((uint32_t)data[2] << 8) |
		   data[3];
}

static void pus_write_u16_be(uint8_t *out, uint16_t value)
{
	out[0] = (uint8_t)(value >> 8);
	out[1] = (uint8_t)(value & 0xffu);
}

static void pus_write_u32_be(uint8_t *out, uint32_t value)
{
	out[0] = (uint8_t)(value >> 24);
	out[1] = (uint8_t)((value >> 16) & 0xffu);
	out[2] = (uint8_t)((value >> 8) & 0xffu);
	out[3] = (uint8_t)(value & 0xffu);
}

pus_status_t pus_tc_sec_header_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tc_sec_header_t *header)
{
	if (!data || !header)
	{
		return PUS_STATUS_NULL;
	}

	if (len < PUS_TC_SEC_HEADER_LEN)
	{
		return PUS_STATUS_BAD_LENGTH;
	}

	header->version = (uint8_t)((data[0] >> 4) & 0x0fu);
	header->ack_flags = (uint8_t)(data[0] & 0x0fu);
	header->service_type_id = data[1];
	header->subtype_id = data[2];
	header->source_id = pus_read_u16_be(&data[3]);
	header->time = pus_read_u32_be(&data[5]);
	header->spare = data[9];

	return PUS_STATUS_OK;
}

pus_status_t pus_tc_sec_header_encode(
	const pus_tc_sec_header_t *header,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len)
{
	if (!header || !out || !out_len)
	{
		return PUS_STATUS_NULL;
	}

	if (out_capacity < PUS_TC_SEC_HEADER_LEN)
	{
		return PUS_STATUS_BUFFER_TOO_SMALL;
	}

	out[0] = (uint8_t)(((header->version & 0x0fu) << 4) |
					   (header->ack_flags & 0x0fu));
	out[1] = header->service_type_id;
	out[2] = header->subtype_id;
	pus_write_u16_be(&out[3], header->source_id);
	pus_write_u32_be(&out[5], header->time);
	out[9] = header->spare;
	*out_len = PUS_TC_SEC_HEADER_LEN;

	return PUS_STATUS_OK;
}

pus_status_t pus_tm_sec_header_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tm_sec_header_t *header)
{
	if (!data || !header)
	{
		return PUS_STATUS_NULL;
	}

	if (len < PUS_TM_SEC_HEADER_LEN)
	{
		return PUS_STATUS_BAD_LENGTH;
	}

	header->version = (uint8_t)((data[0] >> 4) & 0x0fu);
	header->time_ref_status = (uint8_t)(data[0] & 0x0fu);
	header->service_type_id = data[1];
	header->subtype_id = data[2];
	header->msg_type_counter = pus_read_u16_be(&data[3]);
	header->destination_id = pus_read_u16_be(&data[5]);
	header->time = pus_read_u32_be(&data[7]);
	header->spare = data[11];

	return PUS_STATUS_OK;
}

pus_status_t pus_tm_sec_header_encode(
	const pus_tm_sec_header_t *header,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len)
{
	if (!header || !out || !out_len)
	{
		return PUS_STATUS_NULL;
	}

	if (out_capacity < PUS_TM_SEC_HEADER_LEN)
	{
		return PUS_STATUS_BUFFER_TOO_SMALL;
	}

	out[0] = (uint8_t)(((header->version & 0x0fu) << 4) |
					   (header->time_ref_status & 0x0fu));
	out[1] = header->service_type_id;
	out[2] = header->subtype_id;
	pus_write_u16_be(&out[3], header->msg_type_counter);
	pus_write_u16_be(&out[5], header->destination_id);
	pus_write_u32_be(&out[7], header->time);
	out[11] = header->spare;
	*out_len = PUS_TM_SEC_HEADER_LEN;

	return PUS_STATUS_OK;
}
