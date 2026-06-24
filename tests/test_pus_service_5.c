#include "cunit.h"
#include "pus_context.h"
#include "pus_service_5.h"
#include "pus_services.h"
#include "test_helpers.h"
#include "test_runners.h"

#include <string.h>

#define g_buf th_buf
#define g_len th_len
#define make_ctx th_make_ctx

static int test_emit_info(void)
{
    pus_context_t ctx = make_ctx();
    g_len = 0;

    ASSERT_EQ_INT(PUS_STATUS_OK,
                  pus_service_5_emit(&ctx, PUS_SUBTYPE_EVENT_INFO, 0x1234u, NULL, 0u));

    /* len = header(12) + event_id(2) */
    ASSERT_EQ_INT(14, g_len);
    ASSERT_EQ_INT(PUS_SERVICE_EVENT_REPORTING, g_buf[1]);
    ASSERT_EQ_INT(PUS_SUBTYPE_EVENT_INFO, g_buf[2]);
    ASSERT_EQ_INT(0x12, g_buf[12]);
    ASSERT_EQ_INT(0x34, g_buf[13]);

    return 0;
}

static int test_emit_high_severity(void)
{
    pus_context_t ctx = make_ctx();
    g_len = 0;

    ASSERT_EQ_INT(PUS_STATUS_OK,
                  pus_service_5_emit(&ctx, PUS_SUBTYPE_EVENT_HIGH_SEVERITY, 0xABCDu, NULL, 0u));

    ASSERT_EQ_INT(PUS_SUBTYPE_EVENT_HIGH_SEVERITY, g_buf[2]);
    ASSERT_EQ_INT(0xAB, g_buf[12]);
    ASSERT_EQ_INT(0xCD, g_buf[13]);

    return 0;
}

static int test_emit_with_aux_data(void)
{
    pus_context_t ctx = make_ctx();
    const uint8_t aux[] = {0x01u, 0x02u, 0x03u};
    g_len = 0;

    ASSERT_EQ_INT(PUS_STATUS_OK,
                  pus_service_5_emit(&ctx, PUS_SUBTYPE_EVENT_LOW_SEVERITY, 0x0010u, aux, 3u));

    /* len = header(12) + event_id(2) + aux(3) */
    ASSERT_EQ_INT(17, g_len);
    ASSERT_EQ_INT(0x00, g_buf[12]);
    ASSERT_EQ_INT(0x10, g_buf[13]);
    ASSERT_EQ_INT(0x01, g_buf[14]);
    ASSERT_EQ_INT(0x02, g_buf[15]);
    ASSERT_EQ_INT(0x03, g_buf[16]);

    return 0;
}

static int test_emit_no_sink_ok(void)
{
    pus_context_t ctx;
    pus_init(&ctx); /* no sink */

    ASSERT_EQ_INT(PUS_STATUS_OK,
                  pus_service_5_emit(&ctx, PUS_SUBTYPE_EVENT_INFO, 0x0001u, NULL, 0u));

    return 0;
}

static int test_all_subtypes_produce_correct_service(void)
{
    pus_context_t ctx = make_ctx();
    const uint8_t subtypes[] = {PUS_SUBTYPE_EVENT_INFO,
                                PUS_SUBTYPE_EVENT_LOW_SEVERITY,
                                PUS_SUBTYPE_EVENT_MEDIUM_SEVERITY,
                                PUS_SUBTYPE_EVENT_HIGH_SEVERITY};

    for (int i = 0; i < 4; i++)
    {
        g_len = 0;
        ASSERT_EQ_INT(PUS_STATUS_OK, pus_service_5_emit(&ctx, subtypes[i], 0u, NULL, 0u));
        ASSERT_EQ_INT(PUS_SERVICE_EVENT_REPORTING, g_buf[1]);
        ASSERT_EQ_INT(subtypes[i], g_buf[2]);
    }

    return 0;
}

static uint32_t test_time_source(void *ud)
{
    (void)ud;
    return 0xDEADBEEFu;
}

static int test_null_ctx(void)
{
    ASSERT_EQ_INT(PUS_STATUS_NULL,
                  pus_service_5_emit(NULL, PUS_SUBTYPE_EVENT_INFO, 0x0001u, NULL, 0u));
    return 0;
}

static int test_null_aux_data_with_len(void)
{
    pus_context_t ctx = make_ctx();
    /* aux_len > 0 but aux_data == NULL → PUS_STATUS_NULL */
    ASSERT_EQ_INT(PUS_STATUS_NULL,
                  pus_service_5_emit(&ctx, PUS_SUBTYPE_EVENT_INFO, 0x0001u, NULL, 1u));
    return 0;
}

static int test_aux_overflow(void)
{
    pus_context_t ctx = make_ctx();
    const uint8_t dummy = 0u;
    ASSERT_EQ_INT(PUS_STATUS_BUFFER_TOO_SMALL,
                  pus_service_5_emit(&ctx,
                                     PUS_SUBTYPE_EVENT_INFO,
                                     0x0001u,
                                     &dummy,
                                     (uint16_t)(PUS_MAX_TM_PAYLOAD_LEN + 1u)));
    return 0;
}

static int test_emit_with_time_source(void)
{
    pus_context_t ctx = make_ctx();
    ctx.time_source = test_time_source;
    g_len = 0;
    ASSERT_EQ_INT(PUS_STATUS_OK,
                  pus_service_5_emit(&ctx, PUS_SUBTYPE_EVENT_INFO, 0x0001u, NULL, 0u));
    /* time at bytes [7-10] in TM header */
    ASSERT_EQ_INT(0xDE, g_buf[7]);
    ASSERT_EQ_INT(0xAD, g_buf[8]);
    return 0;
}

pus_test_result_t test_pus_service_5_run_all(void)
{
    RUN_TEST(test_emit_info);
    RUN_TEST(test_emit_high_severity);
    RUN_TEST(test_emit_with_aux_data);
    RUN_TEST(test_emit_no_sink_ok);
    RUN_TEST(test_all_subtypes_produce_correct_service);
    RUN_TEST(test_null_ctx);
    RUN_TEST(test_null_aux_data_with_len);
    RUN_TEST(test_aux_overflow);
    RUN_TEST(test_emit_with_time_source);
    return (pus_test_result_t){cunit_total_tests - cunit_overall_failures, cunit_total_tests};
}
