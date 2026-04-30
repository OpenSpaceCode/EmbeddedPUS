#include <string.h>
#include <stdint.h>
#include "../include/pus.h"
#include "../include/pus_config.h"

pus_status_t pus_tc_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tc_packet_t *tc)
{
	pus_status_t status = PUS_STATUS_OK;

	if (!data || !tc)
	{
		return PUS_STATUS_NULL;
	}

	/* Validate minimum length for TC secondary header */
	if (len < PUS_TC_SEC_HEADER_LEN)
	{
		return PUS_STATUS_BAD_LENGTH;
	}

	/* Decode the TC secondary header */
	status = pus_tc_sec_header_decode(data, len, &tc->sec_header);
	if (status != PUS_STATUS_OK)
	{
		return status;
	}

	/* Validate PUS version */
	if (tc->sec_header.version != PUS_VERSION)
	{
		return PUS_STATUS_BAD_VERSION;
	}

	/* Set payload pointer and length */
	tc->payload = data + PUS_TC_SEC_HEADER_LEN;
	tc->payload_len = (uint16_t)(len - PUS_TC_SEC_HEADER_LEN);

	return PUS_STATUS_OK;
}

pus_status_t pus_tc_process(
	pus_context_t *ctx,
	const uint8_t *data,
	uint16_t len)
{
	pus_status_t status = PUS_STATUS_OK;
	pus_tc_packet_t tc;
	int handler_idx = -1;
	pus_tc_handler_t handler = NULL;
	void *user_data = NULL;
	uint8_t ack_accept = 0u;
	uint8_t ack_start = 0u;
	uint8_t ack_progress = 0u;
	uint8_t ack_complete = 0u;

	if (!ctx || !data)
	{
		return PUS_STATUS_NULL;
	}

	/* Decode the TC packet */
	status = pus_tc_decode(data, len, &tc);
	if (status != PUS_STATUS_OK)
	{
		/* Decode failed - could not determine ACK flags */
		return status;
	}

	/* Parse ACK flags */
	ack_accept = (tc.sec_header.ack_flags & 0x08u) ? 1u : 0u;
	ack_start = (tc.sec_header.ack_flags & 0x04u) ? 1u : 0u;
	ack_progress = (tc.sec_header.ack_flags & 0x02u) ? 1u : 0u;
	ack_complete = (tc.sec_header.ack_flags & 0x01u) ? 1u : 0u;

	/* ACK flags are stored for future Service 1 verification report generation */
	(void)ack_accept;
	(void)ack_start;
	(void)ack_progress;
	(void)ack_complete;

	/* Look up the handler */
	handler_idx = pus_handler_find(ctx, tc.sec_header.service_type_id, tc.sec_header.subtype_id);
	if (handler_idx < 0)
	{
		/* No handler found - emit TM[1,10] routing failure if requested */
		/* TODO: Implement Service 1 routing failure report */
		return PUS_STATUS_NO_HANDLER;
	}

	/* Get handler and user data */
	handler = ctx->handler_table[handler_idx].handler;
	user_data = ctx->handler_table[handler_idx].user_data;

	/* TODO: Emit TM[1,3] start success if requested (ack_start) */

	/* Call the handler */
	status = handler(ctx, &tc, user_data);

	/* TODO: Emit TM[1,7] completion success or TM[1,8] completion failure based on ack_complete */

	return status;
}

pus_status_t pus_tm_build(
	pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype,
	uint16_t destination_id,
	const uint8_t *payload,
	uint16_t payload_len,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len)
{
	pus_status_t status = PUS_STATUS_OK;
	pus_tm_sec_header_t sec_header;
	uint16_t encoded_len = 0u;
	uint16_t total_len = 0u;

	/* Validate required pointers */
	if (!ctx || !out || !out_len)
	{
		return PUS_STATUS_NULL;
	}

	/* Calculate total required length */
	total_len = PUS_TM_SEC_HEADER_LEN + payload_len;

	/* Validate output buffer capacity */
	if (out_capacity < total_len)
	{
		return PUS_STATUS_BUFFER_TOO_SMALL;
	}

	/* Build TM secondary header */
	sec_header.version = PUS_VERSION;
	sec_header.time_ref_status = 0u;
	sec_header.service_type_id = service;
	sec_header.subtype_id = subtype;
	sec_header.msg_type_counter = ctx->tm_counter;
	sec_header.destination_id = destination_id;
	sec_header.time = 0u; /* TODO: Add time source */
	sec_header.spare = 0u;

	/* Encode the TM secondary header */
	status = pus_tm_sec_header_encode(&sec_header, out, out_capacity, &encoded_len);
	if (status != PUS_STATUS_OK)
	{
		return status;
	}

	/* Copy payload after the secondary header */
	if (payload && payload_len > 0)
	{
		memcpy(&out[encoded_len], payload, payload_len);
	}

	/* Increment message counter */
	ctx->tm_counter++;

	/* Set output length */
	*out_len = total_len;

	/* If TM sink is configured, send the TM packet */
	if (ctx->tm_sink)
	{
		status = ctx->tm_sink(ctx->tm_sink_user_data, out, total_len);
	}

	return status;
}

pus_status_t pus_set_tm_sink(
	pus_context_t *ctx,
	pus_tm_sink_t sink,
	void *user_data)
{
	if (!ctx)
	{
		return PUS_STATUS_NULL;
	}

	ctx->tm_sink = sink;
	ctx->tm_sink_user_data = user_data;

	return PUS_STATUS_OK;
}

#if PUS_ENABLE_SPACE_PACKET_ADAPTER
int pus_from_space_packet(const sp_packet_t *sp, pus_frame_t *pus)
{
	if (!sp || !pus)
	{
		return 0;
	}

	if (!sp->ph.sec_hdr_flag || sp->sec_hdr_len == 0)
	{
		return 0;
	}

	pus->is_tm = (sp->ph.type == 0) ? 1 : 0;
	pus->payload = (uint8_t *)(uintptr_t)sp->payload;
	pus->payload_len = sp->payload_len;

	if (pus->is_tm)
	{
		if (sp->sec_hdr_len < sizeof(pus_tm_sec_header_t))
		{
			return 0;
		}
		const pus_tm_sec_header_t *tm = (const pus_tm_sec_header_t *)sp->sec_hdr;
		if (tm->version != PUS_VERSION)
		{
			return 0;
		}
		memcpy(&pus->sec_header.tm, sp->sec_hdr, sizeof(pus_tm_sec_header_t));
	}
	else
	{
		if (sp->sec_hdr_len < sizeof(pus_tc_sec_header_t))
		{
			return 0;
		}
		const pus_tc_sec_header_t *tc = (const pus_tc_sec_header_t *)sp->sec_hdr;
		if (tc->version != PUS_VERSION)
		{
			return 0;
		}
		memcpy(&pus->sec_header.tc, sp->sec_hdr, sizeof(pus_tc_sec_header_t));
	}

	return 1;
}

int pus_to_space_packet(const pus_frame_t *pus, sp_packet_t *sp)
{
	if (!pus || !sp)
	{
		return 0;
	}

	sp_packet_init(sp);

	uint8_t type = pus->is_tm ? 0 : 1;
	sp_set_primary_header(sp, PUS_VERSION, type, 1, 0, SP_SEQ_FLAG_UNSEGMENTED, 0);

	uint16_t sec_hdr_len = pus->is_tm ? sizeof(pus_tm_sec_header_t) : sizeof(pus_tc_sec_header_t);

	const uint8_t *sec_hdr = (const uint8_t *)&pus->sec_header;
	sp_set_secondary_header(sp, sec_hdr, sec_hdr_len);
	sp_set_payload(sp, pus->payload, pus->payload_len);

	return 1;
}
#endif
