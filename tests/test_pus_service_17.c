#include "cunit.h"
#include "test_runners.h"
#include "pus_service_17.h"
#include "pus_context.h"
#include "pus_handler.h"
#include "pus_services.h"
#include <string.h>

static uint8_t  g_buf[64];
static uint16_t g_len = 0;

static pus_status_t test_sink(void *ud, const uint8_t *data, uint16_t len)
{
	(void)ud;
	memcpy(g_buf, data, len < sizeof(g_buf) ? len : sizeof(g_buf));
	g_len = len;
	return PUS_STATUS_OK;
}

static pus_context_t make_ctx(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	ctx.tm_sink = test_sink;
	return ctx;
}

static int test_emit_alive_report(void)
{
	pus_context_t ctx = make_ctx();
	g_len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_17_emit_alive_report(&ctx, 0x0042u));

	/* TM[17,2]: header only, no payload */
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN, g_len);
	ASSERT_EQ_INT(PUS_SERVICE_TEST,                    g_buf[1]);
	ASSERT_EQ_INT(PUS_SUBTYPE_TEST_ARE_YOU_ALIVE_REPORT, g_buf[2]);
	/* destination_id = 0x0042 */
	ASSERT_EQ_INT(0x00, g_buf[5]);
	ASSERT_EQ_INT(0x42, g_buf[6]);

	return 0;
}

static int test_emit_connection_report(void)
{
	pus_context_t ctx = make_ctx();
	g_len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_17_emit_connection_report(&ctx, 0xBEEFu, 0x0001u));

	/* TM[17,4]: header(12) + APID(2) */
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN + 2, g_len);
	ASSERT_EQ_INT(PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION_REPORT, g_buf[2]);
	ASSERT_EQ_INT(0xBE, g_buf[12]);
	ASSERT_EQ_INT(0xEF, g_buf[13]);

	return 0;
}

static int test_handler_17_1_responds(void)
{
	pus_context_t ctx = make_ctx();
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_17_register_handlers(&ctx));

	int idx = pus_handler_find(&ctx, PUS_SERVICE_TEST,
		PUS_SUBTYPE_TEST_ARE_YOU_ALIVE);
	ASSERT_TRUE(idx >= 0);

	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.sec_header.source_id = 0x0099u;
	g_len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));

	/* handler must emit TM[17,2] back to TC source */
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN, g_len);
	ASSERT_EQ_INT(PUS_SUBTYPE_TEST_ARE_YOU_ALIVE_REPORT, g_buf[2]);
	ASSERT_EQ_INT(0x00, g_buf[5]);
	ASSERT_EQ_INT(0x99, g_buf[6]);

	return 0;
}

static int test_handler_17_3_echoes_apid(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_17_register_handlers(&ctx);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_TEST,
		PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION);
	ASSERT_TRUE(idx >= 0);

	const uint8_t payload[] = { 0x04u, 0x2Au }; /* APID = 0x042A */
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.sec_header.source_id = 0x0001u;
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);
	g_len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));

	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN + 2, g_len);
	ASSERT_EQ_INT(PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION_REPORT, g_buf[2]);
	ASSERT_EQ_INT(0x04, g_buf[12]);
	ASSERT_EQ_INT(0x2A, g_buf[13]);

	return 0;
}

static int test_handler_17_3_bad_length(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_17_register_handlers(&ctx);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_TEST,
		PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION);
	ASSERT_TRUE(idx >= 0);

	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = NULL;
	tc.payload_len = 1u; /* too short */

	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));

	return 0;
}

pus_test_result_t test_pus_service_17_run_all(void)
{
	RUN_TEST(test_emit_alive_report);
	RUN_TEST(test_emit_connection_report);
	RUN_TEST(test_handler_17_1_responds);
	RUN_TEST(test_handler_17_3_echoes_apid);
	RUN_TEST(test_handler_17_3_bad_length);
	return (pus_test_result_t){ cunit_total_tests - cunit_overall_failures, cunit_total_tests };
}
