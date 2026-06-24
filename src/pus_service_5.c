#include "pus_service_5.h"

#include "pus.h"
#include "pus_services.h"

#include <stddef.h>

/* TM[5,x] payload: event_id(2) + aux_data (up to PUS_MAX_TM_PAYLOAD_LEN) */
#define EVID_LEN 2u
#define MAX_PAYLOAD (EVID_LEN + PUS_MAX_TM_PAYLOAD_LEN)
#define MAX_OUT_LEN (PUS_TM_SEC_HEADER_LEN + MAX_PAYLOAD)

pus_status_t pus_service_5_emit(pus_context_t *ctx,
                                pus_subtype_t subtype,
                                uint16_t event_id,
                                const uint8_t *aux_data,
                                uint16_t aux_len)
{
    uint8_t payload[MAX_PAYLOAD];
    uint8_t out[MAX_OUT_LEN];
    uint16_t out_len;

    if (ctx == NULL)
    {
        return PUS_STATUS_NULL;
    }
    if (aux_len > 0u && aux_data == NULL)
    {
        return PUS_STATUS_NULL;
    }
    if (aux_len > PUS_MAX_TM_PAYLOAD_LEN)
    {
        return PUS_STATUS_BUFFER_TOO_SMALL;
    }

    payload[0] = (uint8_t)(event_id >> 8u);
    payload[1] = (uint8_t)(event_id & 0xFFu);

    for (uint16_t i = 0u; i < aux_len; i++)
    {
        payload[EVID_LEN + i] = aux_data[i];
    }

    return pus_tm_build(ctx,
                        PUS_SERVICE_EVENT_REPORTING,
                        subtype,
                        ctx->default_destination_id,
                        payload,
                        (uint16_t)(EVID_LEN + aux_len),
                        out,
                        sizeof(out),
                        &out_len);
}
