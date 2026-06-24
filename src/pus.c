#include "pus.h"

#include "pus_codec.h"
#include "pus_handler.h"
#include "pus_internal.h"
#include "pus_service_1.h"

#include <stddef.h>

pus_status_t pus_set_tm_sink(pus_context_t *ctx, pus_tm_sink_t sink, void *user_data)
{
    if (ctx == NULL)
    {
        return PUS_STATUS_NULL;
    }
    ctx->tm_sink = sink;
    ctx->tm_sink_user_data = user_data;
    return PUS_STATUS_OK;
}

pus_status_t pus_tc_decode(const uint8_t *data, uint16_t len, pus_tc_packet_t *tc)
{
    pus_status_t st;

    if (data == NULL || tc == NULL)
    {
        return PUS_STATUS_NULL;
    }
    if (len < PUS_TC_SEC_HEADER_LEN)
    {
        return PUS_STATUS_BAD_LENGTH;
    }

    st = pus_tc_sec_header_decode(data, len, &tc->sec_header);
    if (st != PUS_STATUS_OK)
    {
        return st;
    }

    tc->payload = &data[PUS_TC_SEC_HEADER_LEN];
    tc->payload_len = (uint16_t)(len - PUS_TC_SEC_HEADER_LEN);

    return PUS_STATUS_OK;
}

pus_status_t pus_tc_process(pus_context_t *ctx, const uint8_t *data, uint16_t len)
{
    pus_tc_packet_t tc;
    pus_status_t st;
    int idx;
    uint8_t ack;

    if (ctx == NULL || data == NULL)
    {
        return PUS_STATUS_NULL;
    }

    st = pus_tc_decode(data, len, &tc);
    if (st != PUS_STATUS_OK)
    {
        return st;
    }

    ack = tc.sec_header.ack_flags;

    /* Find the handler before emitting acceptance success: routing failure is a
     * sub-class of acceptance failure so TM[1,1] must not precede TM[1,10]. */
    idx = pus_handler_find(ctx, tc.sec_header.service_type_id, tc.sec_header.subtype_id);

    if (idx < 0)
    {
        pus_service_1_emit_failure(ctx, &tc, PUS_SUBTYPE_VERIFICATION_ROUTING_FAILURE, 0x0001u);
        return PUS_STATUS_NO_HANDLER;
    }

    if (ack & PUS_ACK_ACCEPTANCE)
    {
        pus_service_1_emit_success(ctx, &tc, PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_SUCCESS);
    }

    if (ack & PUS_ACK_START)
    {
        pus_service_1_emit_success(ctx, &tc, PUS_SUBTYPE_VERIFICATION_START_SUCCESS);
    }

    st = ctx->handler_table[idx].handler(ctx, &tc, ctx->handler_table[idx].user_data);

    if (ack & PUS_ACK_COMPLETION)
    {
        if (st == PUS_STATUS_OK)
        {
            pus_service_1_emit_success(ctx, &tc, PUS_SUBTYPE_VERIFICATION_COMPLETION_SUCCESS);
        }
        else
        {
            pus_service_1_emit_failure(ctx,
                                       &tc,
                                       PUS_SUBTYPE_VERIFICATION_COMPLETION_FAILURE,
                                       (uint16_t)st);
        }
    }

    return st;
}

pus_status_t pus_tm_build(pus_context_t *ctx,
                          pus_service_t service,
                          pus_subtype_t subtype,
                          uint16_t destination_id,
                          const uint8_t *payload,
                          uint16_t payload_len,
                          uint8_t *out,
                          uint16_t out_capacity,
                          uint16_t *out_len)
{
    pus_tm_sec_header_t hdr;
    uint16_t hdr_len;

    if (ctx == NULL || out == NULL || out_len == NULL)
    {
        return PUS_STATUS_NULL;
    }
    /* Guard against uint16_t wrap: check payload alone first, then sum. */
    if (payload_len > PUS_MAX_TM_PAYLOAD_LEN)
    {
        return PUS_STATUS_BUFFER_TOO_SMALL;
    }
    if (out_capacity < (uint16_t)(PUS_TM_SEC_HEADER_LEN + payload_len))
    {
        return PUS_STATUS_BUFFER_TOO_SMALL;
    }

    pus_tm_hdr_fill(ctx, &hdr, service, subtype, destination_id);
    (void)pus_tm_sec_header_encode(&hdr, out, out_capacity, &hdr_len);

    for (uint16_t i = 0u; i < payload_len; i++)
    {
        out[hdr_len + i] = (payload != NULL) ? payload[i] : 0u;
    }

    *out_len = hdr_len + payload_len;
    ctx->tm_counter++;

    if (ctx->tm_sink != NULL)
    {
        return ctx->tm_sink(ctx->tm_sink_user_data, out, *out_len);
    }
    return PUS_STATUS_OK;
}
