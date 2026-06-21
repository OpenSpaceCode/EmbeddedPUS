#include "cunit.h"
#include "test_runners.h"
#include "pus_service_1.h"
#include "pus_context.h"
#include "pus_services.h"
#include "pus_codec.h"
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

static pus_tc_packet_t make_tc(uint8_t svc, uint8_t sub, uint16_t src)
{
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.sec_header.service_type_id = svc;
	tc.sec_header.subtype_id      = sub;
	tc.sec_header.source_id       = src;
	return tc;
}

static int test_build_success_payload(void)
{
	pus_context_t ctx = make_ctx();
	pus_tc_packet_t tc = make_tc(17u, 1u, 0x0042u);
	uint8_t  out[32];
	uint16_t len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_1_build_success(&ctx, &tc,
			PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_SUCCESS,
			out, sizeof(out), &len));

	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN + 4, len);
	/* TM header: service=1, subtype=1 */
	ASSERT_EQ_INT(1,    out[1]);
	ASSERT_EQ_INT(PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_SUCCESS, out[2]);
	/* payload: [TC service][TC subtype][src_hi][src_lo] */
	ASSERT_EQ_INT(17,   out[12]);
	ASSERT_EQ_INT(1,    out[13]);
	ASSERT_EQ_INT(0x00, out[14]);
	ASSERT_EQ_INT(0x42, out[15]);

	return 0;
}

static int test_build_failure_payload(void)
{
	pus_context_t ctx = make_ctx();
	pus_tc_packet_t tc = make_tc(20u, 3u, 0x0100u);
	uint8_t  out[32];
	uint16_t len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_1_build_failure(&ctx, &tc,
			PUS_SUBTYPE_VERIFICATION_ROUTING_FAILURE,
			0xABCDu,
			out, sizeof(out), &len));

	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN + 6, len);
	ASSERT_EQ_INT(1,    out[1]);
	ASSERT_EQ_INT(PUS_SUBTYPE_VERIFICATION_ROUTING_FAILURE, out[2]);
	ASSERT_EQ_INT(20,   out[12]);
	ASSERT_EQ_INT(3,    out[13]);
	ASSERT_EQ_INT(0x01, out[14]);
	ASSERT_EQ_INT(0x00, out[15]);
	ASSERT_EQ_INT(0xAB, out[16]);
	ASSERT_EQ_INT(0xCD, out[17]);

	return 0;
}

static int test_emit_success_calls_sink(void)
{
	pus_context_t ctx = make_ctx();
	pus_tc_packet_t tc = make_tc(17u, 1u, 0x0005u);
	g_len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_1_emit_success(&ctx, &tc,
			PUS_SUBTYPE_VERIFICATION_COMPLETION_SUCCESS));

	ASSERT_TRUE(g_len > 0);
	ASSERT_EQ_INT(1,    g_buf[1]);  /* service 1 */
	ASSERT_EQ_INT(PUS_SUBTYPE_VERIFICATION_COMPLETION_SUCCESS, g_buf[2]);

	return 0;
}

static int test_emit_failure_calls_sink(void)
{
	pus_context_t ctx = make_ctx();
	pus_tc_packet_t tc = make_tc(5u, 1u, 0x0001u);
	g_len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_1_emit_failure(&ctx, &tc,
			PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_FAILURE,
			0x1234u));

	ASSERT_TRUE(g_len > 0);
	ASSERT_EQ_INT(PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_FAILURE, g_buf[2]);
	ASSERT_EQ_INT(0x12, g_buf[16]);
	ASSERT_EQ_INT(0x34, g_buf[17]);

	return 0;
}

static int test_emit_no_sink_ok(void)
{
	pus_context_t ctx;
	pus_init(&ctx);  /* no sink */
	pus_tc_packet_t tc = make_tc(1u, 1u, 0u);

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_1_emit_success(&ctx, &tc,
			PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_SUCCESS));
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_1_emit_failure(&ctx, &tc,
			PUS_SUBTYPE_VERIFICATION_ROUTING_FAILURE, 0u));

	return 0;
}

pus_test_result_t test_pus_service_1_run_all(void)
{
	RUN_TEST(test_build_success_payload);
	RUN_TEST(test_build_failure_payload);
	RUN_TEST(test_emit_success_calls_sink);
	RUN_TEST(test_emit_failure_calls_sink);
	RUN_TEST(test_emit_no_sink_ok);
	return (pus_test_result_t){ cunit_total_tests - cunit_overall_failures, cunit_total_tests };
}
