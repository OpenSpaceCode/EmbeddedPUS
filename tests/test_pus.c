#include "cunit.h"
#include "test_runners.h"
#include "pus.h"
#include "pus_service_1.h"
#include <string.h>

static uint8_t  g_buf[512];
static uint16_t g_len = 0;

static pus_status_t test_sink(void *ud, const uint8_t *data, uint16_t len)
{
	(void)ud;
	memcpy(g_buf, data, len < sizeof(g_buf) ? len : sizeof(g_buf));
	g_len = len;
	return PUS_STATUS_OK;
}

static uint32_t test_time_source(void *ud) { (void)ud; return 0x12345678u; }

/* Write a minimal valid TC byte buffer: version=1, ack_flags, service, subtype */
static void make_tc_buf(uint8_t *buf, uint8_t svc, uint8_t sub, uint8_t ack)
{
	memset(buf, 0, PUS_TC_SEC_HEADER_LEN);
	buf[0] = (uint8_t)((1u << 4u) | (ack & 0x0Fu));
	buf[1] = svc;
	buf[2] = sub;
}

/* ---- pus_context.c ---- */

static int test_init_null(void)
{
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_init(NULL));
	return 0;
}

static int test_init_with_config_null_ctx(void)
{
	pus_config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_init_with_config(NULL, &cfg));
	return 0;
}

static int test_init_with_config_null_config(void)
{
	pus_context_t ctx;
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_init_with_config(&ctx, NULL));
	return 0;
}

static int test_init_with_config_copies_fields(void)
{
	pus_context_t ctx;
	pus_config_t  cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.default_source_id      = 0x0001u;
	cfg.default_destination_id = 0x0002u;
	cfg.tm_sink                = test_sink;
	cfg.tm_sink_user_data      = (void *)42;
	cfg.time_source            = test_time_source;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init_with_config(&ctx, &cfg));
	ASSERT_EQ_INT(0x0001, ctx.default_source_id);
	ASSERT_EQ_INT(0x0002, ctx.default_destination_id);
	ASSERT_TRUE(ctx.tm_sink        == test_sink);
	ASSERT_TRUE(ctx.tm_sink_user_data == (void *)42);
	ASSERT_TRUE(ctx.time_source    == test_time_source);
	return 0;
}

/* ---- pus_set_tm_sink ---- */

static int test_set_tm_sink_null(void)
{
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_set_tm_sink(NULL, test_sink, NULL));
	return 0;
}

static int test_set_tm_sink_ok(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_set_tm_sink(&ctx, test_sink, (void *)7u));
	ASSERT_TRUE(ctx.tm_sink           == test_sink);
	ASSERT_TRUE(ctx.tm_sink_user_data == (void *)7u);
	return 0;
}

/* ---- pus_tc_decode ---- */

static int test_tc_decode_null(void)
{
	uint8_t         buf[PUS_TC_SEC_HEADER_LEN] = {0};
	pus_tc_packet_t tc;
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_decode(NULL, sizeof(buf), &tc));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_decode(buf,  sizeof(buf), NULL));
	return 0;
}

static int test_tc_decode_bad_length(void)
{
	uint8_t         buf[4] = {0};
	pus_tc_packet_t tc;
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH, pus_tc_decode(buf, sizeof(buf), &tc));
	return 0;
}

static int test_tc_decode_bad_version(void)
{
	uint8_t         buf[PUS_TC_SEC_HEADER_LEN] = {0}; /* version=0 in top nibble */
	pus_tc_packet_t tc;
	ASSERT_EQ_INT(PUS_STATUS_BAD_VERSION, pus_tc_decode(buf, sizeof(buf), &tc));
	return 0;
}

static int test_tc_decode_success(void)
{
	uint8_t         buf[PUS_TC_SEC_HEADER_LEN + 2u];
	pus_tc_packet_t tc;
	make_tc_buf(buf, 17u, 1u, 0u);
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_decode(buf, sizeof(buf), &tc));
	ASSERT_EQ_INT(17, tc.sec_header.service_type_id);
	ASSERT_EQ_INT(1,  tc.sec_header.subtype_id);
	ASSERT_EQ_INT(2,  tc.payload_len);
	return 0;
}

/* ---- pus_tc_process ---- */

static pus_status_t dummy_handler(pus_context_t *ctx, const pus_tc_packet_t *tc, void *ud)
{
	(void)ctx; (void)tc; (void)ud;
	return PUS_STATUS_OK;
}

static pus_status_t failing_handler(pus_context_t *ctx, const pus_tc_packet_t *tc, void *ud)
{
	(void)ctx; (void)tc; (void)ud;
	return PUS_STATUS_HANDLER_FAILED;
}

static int test_tc_process_null(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	uint8_t buf[PUS_TC_SEC_HEADER_LEN] = {0};
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_process(NULL, buf,  sizeof(buf)));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_process(&ctx, NULL, sizeof(buf)));
	return 0;
}

static int test_tc_process_bad_decode(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	uint8_t buf[PUS_TC_SEC_HEADER_LEN] = {0}; /* version=0 → BAD_VERSION */
	ASSERT_EQ_INT(PUS_STATUS_BAD_VERSION, pus_tc_process(&ctx, buf, sizeof(buf)));
	return 0;
}

static int test_tc_process_no_handler(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	ctx.tm_sink = test_sink;
	uint8_t buf[PUS_TC_SEC_HEADER_LEN];
	make_tc_buf(buf, 99u, 1u, 0u);
	g_len = 0;
	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER, pus_tc_process(&ctx, buf, sizeof(buf)));
	ASSERT_TRUE(g_len > 0); /* TM[1,10] routing failure emitted */
	return 0;
}

static int test_tc_process_all_acks_success(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	ctx.tm_sink = test_sink;
	pus_handler_register(&ctx, 17u, 1u, dummy_handler, NULL);
	uint8_t buf[PUS_TC_SEC_HEADER_LEN];
	make_tc_buf(buf, 17u, 1u,
	            (uint8_t)(PUS_ACK_ACCEPTANCE | PUS_ACK_START | PUS_ACK_COMPLETION));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_process(&ctx, buf, sizeof(buf)));
	return 0;
}

static int test_tc_process_completion_failure(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	ctx.tm_sink = test_sink;
	pus_handler_register(&ctx, 17u, 1u, failing_handler, NULL);
	uint8_t buf[PUS_TC_SEC_HEADER_LEN];
	make_tc_buf(buf, 17u, 1u, PUS_ACK_COMPLETION);
	ASSERT_EQ_INT(PUS_STATUS_HANDLER_FAILED, pus_tc_process(&ctx, buf, sizeof(buf)));
	return 0;
}

/* ---- PUS_ACK_PROGRESS — handler-side responsibility ---- */

/*
 * Mock handler that manually emits TM[1,5] progress success when the TC has
 * the PUS_ACK_PROGRESS flag set, as documented in pus_service_1.h.
 */
static pus_status_t progress_handler(
	pus_context_t         *ctx,
	const pus_tc_packet_t *tc,
	void                  *ud)
{
	(void)ud;
	if (tc->sec_header.ack_flags & PUS_ACK_PROGRESS) {
		pus_service_1_emit_success(ctx, tc,
			PUS_SUBTYPE_VERIFICATION_PROGRESS_SUCCESS);
	}
	return PUS_STATUS_OK;
}

static int test_tc_process_progress_ack_emitted_by_handler(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	ctx.tm_sink = test_sink;
	pus_handler_register(&ctx, 17u, 1u, progress_handler, NULL);

	uint8_t buf[PUS_TC_SEC_HEADER_LEN];
	make_tc_buf(buf, 17u, 1u,
	            (uint8_t)(PUS_ACK_ACCEPTANCE | PUS_ACK_PROGRESS | PUS_ACK_COMPLETION));
	g_len = 0;
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_process(&ctx, buf, sizeof(buf)));

	/* The last packet forwarded to the sink should be TM[1,7] completion success.
	 * TM[1,5] progress was emitted inside the handler; we verify the counter
	 * advanced by 3 (acceptance=1, progress=1, completion=1). */
	ASSERT_EQ_INT(3, ctx.tm_counter);
	/* Last captured packet: service=1, subtype=7 (completion success) */
	ASSERT_EQ_INT(1u, g_buf[1]);
	ASSERT_EQ_INT(PUS_SUBTYPE_VERIFICATION_COMPLETION_SUCCESS, g_buf[2]);
	return 0;
}

/* ---- pus_tm_build ---- */

static int test_tm_build_null(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	uint8_t  buf[64];
	uint16_t len;
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_tm_build(NULL, 1u, 1u, 0u, NULL, 0u, buf,  sizeof(buf), &len));
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_tm_build(&ctx,  1u, 1u, 0u, NULL, 0u, NULL, sizeof(buf), &len));
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_tm_build(&ctx,  1u, 1u, 0u, NULL, 0u, buf,  sizeof(buf), NULL));
	return 0;
}

static int test_tm_build_payload_too_large(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	uint8_t  buf[512];
	uint16_t len;
	ASSERT_EQ_INT(PUS_STATUS_BUFFER_TOO_SMALL,
		pus_tm_build(&ctx, 1u, 1u, 0u, NULL,
		             (uint16_t)(PUS_MAX_TM_PAYLOAD_LEN + 1u),
		             buf, sizeof(buf), &len));
	return 0;
}

static int test_tm_build_capacity_too_small(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	uint8_t  buf[4];
	uint16_t len;
	uint8_t  payload[8] = {0};
	ASSERT_EQ_INT(PUS_STATUS_BUFFER_TOO_SMALL,
		pus_tm_build(&ctx, 1u, 1u, 0u, payload, sizeof(payload),
		             buf, sizeof(buf), &len));
	return 0;
}

static int test_tm_build_null_payload_zero_fills(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	ctx.tm_sink     = test_sink;
	ctx.time_source = test_time_source; /* cover time_source != NULL branch */
	g_len = 0;
	uint8_t  buf[64];
	uint16_t len;
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_tm_build(&ctx, 17u, 2u, 0u, NULL, 4u, buf, sizeof(buf), &len));
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN + 4, len);
	ASSERT_EQ_INT(0, buf[PUS_TM_SEC_HEADER_LEN]);
	return 0;
}

static int test_tm_build_no_sink(void)
{
	pus_context_t ctx;
	pus_init(&ctx); /* no sink */
	uint8_t  buf[64];
	uint16_t len;
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_tm_build(&ctx, 17u, 2u, 0u, NULL, 0u, buf, sizeof(buf), &len));
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN, len);
	return 0;
}

/* ---- tm_counter increments ---- */

static int test_tm_counter_increments_with_sink(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	ctx.tm_sink = test_sink;
	uint8_t buf[64]; uint16_t len;

	ASSERT_EQ_INT(0, ctx.tm_counter);
	pus_tm_build(&ctx, 17u, 2u, 0u, NULL, 0u, buf, sizeof(buf), &len);
	ASSERT_EQ_INT(1, ctx.tm_counter);
	pus_tm_build(&ctx, 17u, 2u, 0u, NULL, 0u, buf, sizeof(buf), &len);
	ASSERT_EQ_INT(2, ctx.tm_counter);
	return 0;
}

static int test_tm_counter_increments_without_sink(void)
{
	pus_context_t ctx;
	pus_init(&ctx); /* no sink */
	uint8_t buf[64]; uint16_t len;

	ASSERT_EQ_INT(0, ctx.tm_counter);
	pus_tm_build(&ctx, 17u, 2u, 0u, NULL, 0u, buf, sizeof(buf), &len);
	ASSERT_EQ_INT(1, ctx.tm_counter);
	return 0;
}

pus_test_result_t test_pus_run_all(void)
{
	RUN_TEST(test_init_null);
	RUN_TEST(test_init_with_config_null_ctx);
	RUN_TEST(test_init_with_config_null_config);
	RUN_TEST(test_init_with_config_copies_fields);
	RUN_TEST(test_set_tm_sink_null);
	RUN_TEST(test_set_tm_sink_ok);
	RUN_TEST(test_tc_decode_null);
	RUN_TEST(test_tc_decode_bad_length);
	RUN_TEST(test_tc_decode_bad_version);
	RUN_TEST(test_tc_decode_success);
	RUN_TEST(test_tc_process_null);
	RUN_TEST(test_tc_process_bad_decode);
	RUN_TEST(test_tc_process_no_handler);
	RUN_TEST(test_tc_process_all_acks_success);
	RUN_TEST(test_tc_process_completion_failure);
	RUN_TEST(test_tc_process_progress_ack_emitted_by_handler);
	RUN_TEST(test_tm_build_null);
	RUN_TEST(test_tm_build_payload_too_large);
	RUN_TEST(test_tm_build_capacity_too_small);
	RUN_TEST(test_tm_build_null_payload_zero_fills);
	RUN_TEST(test_tm_build_no_sink);
	RUN_TEST(test_tm_counter_increments_with_sink);
	RUN_TEST(test_tm_counter_increments_without_sink);
	return (pus_test_result_t){ cunit_total_tests - cunit_overall_failures, cunit_total_tests };
}
