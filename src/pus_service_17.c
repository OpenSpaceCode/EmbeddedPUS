#include "pus_service_17.h"

#include "pus.h"
#include "pus_handler.h"
#include "pus_services.h"

#include <stddef.h>

/* TM[17,2]: no payload; TM[17,4]: apid (2 bytes) */
#define APID_LEN 2u
#define MAX_OUT_LEN (PUS_TM_SEC_HEADER_LEN + APID_LEN)

static pus_status_t build_and_emit(pus_context_t *ctx,
                                   pus_subtype_t subtype,
                                   uint16_t destination_id,
                                   const uint8_t *payload,
                                   uint16_t payload_len)
{
    uint8_t out[MAX_OUT_LEN];
    uint16_t out_len;

    return pus_tm_build(ctx,
                        PUS_SERVICE_TEST,
                        subtype,
                        destination_id,
                        payload,
                        payload_len,
                        out,
                        sizeof(out),
                        &out_len);
}

pus_status_t pus_service_17_emit_alive_report(pus_context_t *ctx, uint16_t destination_id)
{
    if (ctx == NULL)
    {
        return PUS_STATUS_NULL;
    }
    return build_and_emit(ctx, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE_REPORT, destination_id, NULL, 0u);
}

pus_status_t
pus_service_17_emit_connection_report(pus_context_t *ctx, uint16_t apid, uint16_t destination_id)
{
    uint8_t payload[APID_LEN];

    if (ctx == NULL)
    {
        return PUS_STATUS_NULL;
    }

    payload[0] = (uint8_t)(apid >> 8u);
    payload[1] = (uint8_t)(apid & 0xFFu);

    return build_and_emit(ctx,
                          PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION_REPORT,
                          destination_id,
                          payload,
                          APID_LEN);
}

static pus_status_t handle_tc_17_1(pus_context_t *ctx, const pus_tc_packet_t *tc, void *user_data)
{
    (void)user_data;
    return pus_service_17_emit_alive_report(ctx, tc->sec_header.source_id);
}

static pus_status_t handle_tc_17_3(pus_context_t *ctx, const pus_tc_packet_t *tc, void *user_data)
{
    uint16_t apid;

    (void)user_data;

    if (tc->payload_len < APID_LEN)
    {
        return PUS_STATUS_BAD_LENGTH;
    }

    apid = ((uint16_t)tc->payload[0] << 8u) | (uint16_t)tc->payload[1];

    return pus_service_17_emit_connection_report(ctx, apid, tc->sec_header.source_id);
}

pus_status_t pus_service_17_register_handlers(pus_context_t *ctx)
{
    pus_status_t st;

    if (ctx == NULL)
    {
        return PUS_STATUS_NULL;
    }

    st = pus_handler_register(ctx,
                              PUS_SERVICE_TEST,
                              PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
                              handle_tc_17_1,
                              NULL);
    if (st != PUS_STATUS_OK)
    {
        return st;
    }

    return pus_handler_register(ctx,
                                PUS_SERVICE_TEST,
                                PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION,
                                handle_tc_17_3,
                                NULL);
}
