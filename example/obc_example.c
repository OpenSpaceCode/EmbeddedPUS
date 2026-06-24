/*
 * EmbeddedPUS example — small satellite OBC
 *
 * Scenario: a CubeSat OBC receives TC commands over a UART link and sends TM
 * packets back over the same link (half-duplex).  This file shows how to wire
 * up all five PUS services in a realistic but self-contained program.
 *
 * Services used:
 *   ST[01]  Request Verification  — emitted automatically by pus_tc_process()
 *   ST[03]  Housekeeping          — periodic temperature + battery report
 *   ST[05]  Event Reporting       — mode-change and low-battery alerts
 *   ST[17]  Test (are-you-alive)  — handled automatically after register
 *   ST[20]  Parameter Management  — ground can read/set an alert threshold
 *
 * All memory is statically allocated; no heap is used.
 */

#include "pus.h"
#include "pus_service_17.h"
#include "pus_service_20.h"
#include "pus_service_3.h"
#include "pus_service_5.h"
#include "pus_services.h"

#include <stdint.h>
#include <stdio.h> /* printf — replace with your platform output */
#include <string.h>

/* -----------------------------------------------------------------------
 * Platform stubs — replace these with real hardware drivers
 * ----------------------------------------------------------------------- */

static uint32_t platform_uptime_seconds(void *ud)
{
    /* Real OBC: read the mission elapsed time counter. */
    (void)ud;
    static uint32_t t = 0u;
    return t++;
}

static int16_t platform_read_temperature_c10(void)
{
    /* Real OBC: read ADC, convert to 1/10 °C units (e.g. 235 = 23.5 °C). */
    return 235;
}

static uint16_t platform_read_battery_mv(void)
{
    /* Real OBC: read battery monitor register. */
    return 7400u;
}

static void platform_uart_send(const uint8_t *data, uint16_t len)
{
    /* Real OBC: DMA-transfer data to the UART transmit buffer. */
    printf("[TM %3u bytes] ", (unsigned)len);
    for (uint16_t i = 0u; i < len && i < 16u; i++)
    {
        printf("%02X ", data[i]);
    }
    if (len > 16u)
    {
        printf("...");
    }
    printf("\n");
}

/* -----------------------------------------------------------------------
 * TM sink: called by pus_tm_build() for every outgoing TM packet
 * ----------------------------------------------------------------------- */

static pus_status_t tm_sink(void *ud, const uint8_t *data, uint16_t len)
{
    (void)ud;
    platform_uart_send(data, len);
    return PUS_STATUS_OK;
}

/* -----------------------------------------------------------------------
 * Mission constants
 * ----------------------------------------------------------------------- */

#define OBC_SOURCE_ID 0x0001u  /* this OBC's APID */
#define GROUND_DEST_ID 0x0010u /* default ground station APID */

/* HK structure IDs (ST[03]) */
#define SID_POWER_HK 0x0001u
#define SID_THERMAL_HK 0x0002u

/* Event IDs (ST[05]) */
#define EVID_MODE_NOMINAL 0x0101u
#define EVID_BATTERY_LOW 0x0201u

/* Parameter IDs (ST[20]) */
#define PID_BATT_ALERT_MV 0x0001u /* battery low-alert threshold [mV] */

/* -----------------------------------------------------------------------
 * Parameter store — the actual values that ST[20] reads and writes
 * ----------------------------------------------------------------------- */

static uint16_t g_batt_alert_mv = 6800u; /* alert below 6.8 V */

static pus_status_t getter_batt_alert(uint16_t pid, uint8_t *buf, uint16_t cap, void *ud)
{
    (void)pid;
    (void)cap;
    (void)ud;
    buf[0] = (uint8_t)(g_batt_alert_mv >> 8u);
    buf[1] = (uint8_t)(g_batt_alert_mv & 0xFFu);
    return PUS_STATUS_OK;
}

static pus_status_t setter_batt_alert(uint16_t pid, const uint8_t *buf, uint16_t len, void *ud)
{
    (void)pid;
    (void)len;
    (void)ud;
    g_batt_alert_mv = (uint16_t)(((uint16_t)buf[0] << 8u) | (uint16_t)buf[1]);
    return PUS_STATUS_OK;
}

/* -----------------------------------------------------------------------
 * HK providers — called by pus_service_3_emit_hk() when TM[3,25] is due
 * ----------------------------------------------------------------------- */

static pus_status_t
hk_power_provider(uint16_t sid, uint8_t *buf, uint16_t cap, uint16_t *out_len, void *ud)
{
    (void)sid;
    (void)cap;
    (void)ud;
    uint16_t mv = platform_read_battery_mv();
    buf[0] = (uint8_t)(mv >> 8u);
    buf[1] = (uint8_t)(mv & 0xFFu);
    *out_len = 2u;
    return PUS_STATUS_OK;
}

static pus_status_t
hk_thermal_provider(uint16_t sid, uint8_t *buf, uint16_t cap, uint16_t *out_len, void *ud)
{
    (void)sid;
    (void)cap;
    (void)ud;
    int16_t temp = platform_read_temperature_c10();
    buf[0] = (uint8_t)((uint16_t)temp >> 8u);
    buf[1] = (uint8_t)((uint16_t)temp & 0xFFu);
    *out_len = 2u;
    return PUS_STATUS_OK;
}

/* -----------------------------------------------------------------------
 * EmbeddedPUS contexts — statically allocated
 * ----------------------------------------------------------------------- */

static pus_context_t g_pus;
static pus_service_3_ctx_t g_s3;
static pus_service_20_ctx_t g_s20;

/* -----------------------------------------------------------------------
 * Initialisation
 * ----------------------------------------------------------------------- */

static void pus_app_init(void)
{
    /* --- Core context --- */
    pus_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.default_source_id = OBC_SOURCE_ID;
    cfg.default_destination_id = GROUND_DEST_ID;
    cfg.tm_sink = tm_sink;
    cfg.time_source = platform_uptime_seconds;
    pus_init_with_config(&g_pus, &cfg);

    /* --- ST[17]: auto-respond to TC[17,1] are-you-alive --- */
    pus_service_17_register_handlers(&g_pus);

    /* --- ST[03]: housekeeping structures --- */
    pus_service_3_init(&g_s3);
    pus_service_3_register_hk(&g_s3, SID_POWER_HK, hk_power_provider, NULL);
    pus_service_3_register_hk(&g_s3, SID_THERMAL_HK, hk_thermal_provider, NULL);

    /* --- ST[20]: on-board parameters --- */
    pus_service_20_init(&g_s20);
    pus_service_20_register_param(&g_s20,
                                  PID_BATT_ALERT_MV,
                                  2u,
                                  getter_batt_alert,
                                  setter_batt_alert,
                                  NULL);
    pus_service_20_register_handlers(&g_pus, &g_s20);

    /* Announce nominal mode via ST[05] */
    pus_service_5_emit(&g_pus, PUS_SUBTYPE_EVENT_INFO, EVID_MODE_NOMINAL, NULL, 0u);
}

/* -----------------------------------------------------------------------
 * Periodic housekeeping downlink (call every HK cycle, e.g. every 10 s)
 * ----------------------------------------------------------------------- */

static void pus_app_send_housekeeping(void)
{
    pus_service_3_emit_hk(&g_pus, &g_s3, SID_THERMAL_HK);
    pus_service_3_emit_hk(&g_pus, &g_s3, SID_POWER_HK);

    /* Low-battery check: emit ST[05] alert if below threshold */
    if (platform_read_battery_mv() < g_batt_alert_mv)
    {
        uint8_t aux[2];
        uint16_t mv = platform_read_battery_mv();
        aux[0] = (uint8_t)(mv >> 8u);
        aux[1] = (uint8_t)(mv & 0xFFu);
        pus_service_5_emit(&g_pus,
                           PUS_SUBTYPE_EVENT_HIGH_SEVERITY,
                           EVID_BATTERY_LOW,
                           aux,
                           sizeof(aux));
    }
}

/* -----------------------------------------------------------------------
 * TC receive path (call when a raw TC frame arrives from the uplink)
 * ----------------------------------------------------------------------- */

static void pus_app_receive_tc(const uint8_t *raw, uint16_t len)
{
    /*
     * pus_tc_process() decodes the TC secondary header, looks up the
     * handler registered for (service, subtype), calls it, and emits
     * TM[1,x] verification reports according to the TC ack_flags.
     *
     * Unrecognised commands receive a TM[1,10] routing failure report.
     */
    pus_tc_process(&g_pus, raw, len);
}

/* -----------------------------------------------------------------------
 * Demo main — simulates a few loop iterations without real hardware
 * ----------------------------------------------------------------------- */

int main(void)
{
    pus_app_init();

    printf("\n--- HK cycle 1 ---\n");
    pus_app_send_housekeeping();

    /*
     * Simulate an incoming TC[17,1] are-you-alive.
     * Wire format: version=1 in top nibble, ack_flags=0, svc=17, sub=1,
     * source_id=0x0010 (ground station), rest zeroed.
     */
    static const uint8_t tc_17_1[] = {
        0x10, /* (version=1 << 4) | ack=0 */
        17u,  /* service */
        1u,   /* subtype */
        0x00,
        0x10, /* source_id = 0x0010 */
        0x00,
        0x00,
        0x00,
        0x00,
        0x00 /* time + spare */
    };
    printf("\n--- Ground TC[17,1] are-you-alive ---\n");
    pus_app_receive_tc(tc_17_1, sizeof(tc_17_1));

    /*
     * Simulate TC[20,1] — request parameter report for PID_BATT_ALERT_MV.
     * Payload: N=1, PID=0x0001
     */
    static const uint8_t tc_20_1[] = {
        0x10, /* version=1, no ack */
        20u,  /* service */
        1u,   /* subtype */
        0x00,
        0x10, /* source_id */
        0x00,
        0x00,
        0x00,
        0x00,
        0x00, /* time + spare */
        0x01, /* N = 1 parameter */
        0x00,
        0x01 /* PID = 0x0001 */
    };
    printf("\n--- Ground TC[20,1] request parameter report ---\n");
    pus_app_receive_tc(tc_20_1, sizeof(tc_20_1));

    /*
     * Simulate TC[20,3] — set PID_BATT_ALERT_MV to 7000 mV (0x1B58).
     * Payload: N=1, PID=0x0001, value=0x1B58
     */
    static const uint8_t tc_20_3[] = {
        0x1F, /* version=1, all ack flags set */
        20u,  /* service */
        3u,   /* subtype */
        0x00,
        0x10, /* source_id */
        0x00,
        0x00,
        0x00,
        0x00,
        0x00, /* time + spare */
        0x01, /* N = 1 parameter */
        0x00,
        0x01, /* PID = 0x0001 */
        0x1B,
        0x58 /* new value = 7000 mV */
    };
    printf("\n--- Ground TC[20,3] set battery alert threshold to 7000 mV ---\n");
    pus_app_receive_tc(tc_20_3, sizeof(tc_20_3));

    printf("\n--- HK cycle 2 (threshold now 7000 mV, 7400 mV battery is OK) ---\n");
    pus_app_send_housekeeping();

    return 0;
}
