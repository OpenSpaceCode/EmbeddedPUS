#include "cunit.h"
#include "test_runners.h"
#include "test_helpers.h"
#include "pus_service_20.h"
#include "pus_context.h"
#include "pus_handler.h"
#include "pus_services.h"
#include <string.h>

#define g_buf th_buf
#define g_len th_len
#define make_ctx th_make_ctx

/* 4-byte big-endian parameter */
static uint32_t g_param_val = 0xDEADBEEFu;

static pus_status_t param_getter(uint16_t pid, uint8_t *buf, uint16_t cap, void *ud)
{
	(void)pid; (void)cap; (void)ud;
	buf[0] = (uint8_t)(g_param_val >> 24u);
	buf[1] = (uint8_t)(g_param_val >> 16u);
	buf[2] = (uint8_t)(g_param_val >>  8u);
	buf[3] = (uint8_t)(g_param_val & 0xFFu);
	return PUS_STATUS_OK;
}

static pus_status_t param_setter(uint16_t pid, const uint8_t *buf, uint16_t len, void *ud)
{
	(void)pid; (void)len; (void)ud;
	g_param_val = ((uint32_t)buf[0] << 24u) | ((uint32_t)buf[1] << 16u)
	            | ((uint32_t)buf[2] <<  8u) |  (uint32_t)buf[3];
	return PUS_STATUS_OK;
}

static int test_init_zeroes(void)
{
	pus_service_20_ctx_t s20;
	memset(&s20, 0xFFu, sizeof(s20));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_service_20_init(&s20));

	for (uint8_t i = 0u; i < PUS_SERVICE_20_MAX_PARAMS; i++) {
		ASSERT_EQ_INT(0, s20.params[i].is_used);
	}
	return 0;
}

static int test_register_and_emit_single(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	g_param_val = 0x01020304u;
	g_len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_20_register_param(&s20, 0x0010u, 4u,
			param_getter, param_setter, NULL));

	const uint16_t ids[] = { 0x0010u };
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_20_emit_report(&ctx, &s20, ids, 1u));

	/* payload: N(1) + PID(2) + value(4) = 7 bytes after header */
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN + 7, g_len);
	ASSERT_EQ_INT(PUS_SERVICE_PARAMETER_MANAGEMENT, g_buf[1]);
	ASSERT_EQ_INT(PUS_SUBTYPE_PARAMETER_VALUE_REPORT, g_buf[2]);
	ASSERT_EQ_INT(1,    g_buf[12]); /* N = 1 */
	ASSERT_EQ_INT(0x00, g_buf[13]); /* PID hi */
	ASSERT_EQ_INT(0x10, g_buf[14]); /* PID lo */
	ASSERT_EQ_INT(0x01, g_buf[15]);
	ASSERT_EQ_INT(0x02, g_buf[16]);
	ASSERT_EQ_INT(0x03, g_buf[17]);
	ASSERT_EQ_INT(0x04, g_buf[18]);

	return 0;
}

static int test_emit_multiple_params(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	g_len = 0;

	pus_service_20_register_param(&s20, 0x0001u, 4u, param_getter, NULL, NULL);
	pus_service_20_register_param(&s20, 0x0002u, 4u, param_getter, NULL, NULL);

	const uint16_t ids[] = { 0x0001u, 0x0002u };
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_20_emit_report(&ctx, &s20, ids, 2u));

	/* N(1) + 2×[PID(2)+value(4)] = 13 bytes */
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN + 13, g_len);
	ASSERT_EQ_INT(2, g_buf[12]); /* N = 2 */

	return 0;
}

static int test_set_values_tc(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_param(&s20, 0x0010u, 4u,
		param_getter, param_setter, NULL);
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_20_register_handlers(&ctx, &s20));

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_SET_VALUES);
	ASSERT_TRUE(idx >= 0);

	/* TC[20,3] payload: N=1, PID=0x0010, value=0xCAFEBABE */
	const uint8_t payload[] = {
		0x01u,
		0x00u, 0x10u,
		0xCAu, 0xFEu, 0xBAu, 0xBEu
	};
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);

	g_param_val = 0u;
	ASSERT_EQ_INT(PUS_STATUS_OK,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	ASSERT_EQ_INT((int)0xCAFEBABEu, (int)g_param_val);

	return 0;
}

static int test_readonly_param_returns_failed(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	/* setter = NULL → read-only */
	pus_service_20_register_param(&s20, 0x0020u, 4u,
		param_getter, NULL, NULL);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_SET_VALUES);
	ASSERT_TRUE(idx >= 0);

	const uint8_t payload[] = { 0x01u, 0x00u, 0x20u, 0x00u, 0x00u, 0x00u, 0x01u };
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);

	ASSERT_EQ_INT(PUS_STATUS_HANDLER_FAILED,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));

	return 0;
}

static int test_unknown_param_returns_no_handler(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);

	const uint16_t ids[] = { 0xFFFFu };
	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER,
		pus_service_20_emit_report(&ctx, &s20, ids, 1u));

	return 0;
}

static pus_status_t failing_getter(uint16_t pid, uint8_t *buf, uint16_t cap, void *ud)
{
	(void)pid; (void)buf; (void)cap; (void)ud;
	return PUS_STATUS_HANDLER_FAILED;
}

static pus_status_t failing_setter(uint16_t pid, const uint8_t *buf, uint16_t len, void *ud)
{
	(void)pid; (void)buf; (void)len; (void)ud;
	return PUS_STATUS_HANDLER_FAILED;
}

static int test_init_null(void)
{
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_service_20_init(NULL));
	return 0;
}

static int test_register_null_args(void)
{
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_service_20_register_param(NULL, 0x0001u, 4u, param_getter, NULL, NULL));
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_service_20_register_param(&s20, 0x0001u, 4u, NULL, NULL, NULL));
	return 0;
}

static int test_register_update_existing(void)
{
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_param(&s20, 0x0001u, 4u, param_getter, param_setter, NULL);
	/* re-register same PID — should update in place */
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_20_register_param(&s20, 0x0001u, 4u, param_getter, NULL, NULL));
	int used = 0;
	for (uint8_t i = 0u; i < PUS_SERVICE_20_MAX_PARAMS; i++) {
		if (s20.params[i].is_used) used++;
	}
	ASSERT_EQ_INT(1, used);
	return 0;
}

static int test_register_table_full(void)
{
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	for (uint8_t i = 0u; i < PUS_SERVICE_20_MAX_PARAMS; i++) {
		ASSERT_EQ_INT(PUS_STATUS_OK,
			pus_service_20_register_param(&s20, (uint16_t)i, 4u,
				param_getter, NULL, NULL));
	}
	ASSERT_EQ_INT(PUS_STATUS_TABLE_FULL,
		pus_service_20_register_param(&s20, 0xFFFFu, 4u, param_getter, NULL, NULL));
	return 0;
}

static int test_emit_report_null(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	const uint16_t ids[] = { 0x0001u };
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_service_20_emit_report(NULL, &s20, ids, 1u));
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_service_20_emit_report(&ctx, NULL, ids, 1u));
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_service_20_emit_report(&ctx, &s20, NULL, 1u));
	return 0;
}

static int test_emit_report_getter_failure(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_param(&s20, 0x0001u, 4u, failing_getter, NULL, NULL);
	const uint16_t ids[] = { 0x0001u };
	ASSERT_EQ_INT(PUS_STATUS_HANDLER_FAILED,
		pus_service_20_emit_report(&ctx, &s20, ids, 1u));
	return 0;
}

static int test_emit_report_buffer_too_small(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	/* value_len=254: hdr(12)+N(1)+PID(2)+value(254) = 269 > MAX_OUT_LEN(268) */
	pus_service_20_register_param(&s20, 0x0001u, 254u, param_getter, NULL, NULL);
	const uint16_t ids[] = { 0x0001u };
	ASSERT_EQ_INT(PUS_STATUS_BUFFER_TOO_SMALL,
		pus_service_20_emit_report(&ctx, &s20, ids, 1u));
	return 0;
}

static int test_emit_report_no_sink(void)
{
	pus_context_t        ctx;
	pus_service_20_ctx_t s20;
	pus_init(&ctx); /* no sink */
	pus_service_20_init(&s20);
	pus_service_20_register_param(&s20, 0x0001u, 4u, param_getter, NULL, NULL);
	const uint16_t ids[] = { 0x0001u };
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_20_emit_report(&ctx, &s20, ids, 1u));
	return 0;
}

/* --- TC[20,1] handler paths --- */

static int test_tc_20_1_success(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_param(&s20, 0x0001u, 4u, param_getter, NULL, NULL);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_REPORT_VALUES);
	ASSERT_TRUE(idx >= 0);

	/* payload: N=1, PID=0x0001 */
	const uint8_t payload[] = { 0x01u, 0x00u, 0x01u };
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);

	ASSERT_EQ_INT(PUS_STATUS_OK,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

static int test_tc_20_1_bad_length(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_REPORT_VALUES);
	ASSERT_TRUE(idx >= 0);

	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = NULL;
	tc.payload_len = 0u;
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

static int test_tc_20_1_n_too_large(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_REPORT_VALUES);
	ASSERT_TRUE(idx >= 0);

	const uint8_t payload[] = { (uint8_t)(PUS_SERVICE_20_MAX_PARAMS + 1u) };
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

static int test_tc_20_1_payload_too_short(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_REPORT_VALUES);
	ASSERT_TRUE(idx >= 0);

	/* N=2 but only 2 bytes total — need 1+2*2=5 */
	const uint8_t payload[] = { 0x02u, 0x00u };
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

/* --- TC[20,3] handler paths --- */

static int test_tc_20_3_bad_length(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_SET_VALUES);
	ASSERT_TRUE(idx >= 0);

	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = NULL;
	tc.payload_len = 0u;
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

static int test_tc_20_3_truncated_pid(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_SET_VALUES);
	ASSERT_TRUE(idx >= 0);

	/* N=1 but only 2 bytes (N + 1 PID byte) — need N + PID(2) = 3 */
	const uint8_t payload[] = { 0x01u, 0x00u };
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

static int test_tc_20_3_unknown_pid(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_SET_VALUES);
	ASSERT_TRUE(idx >= 0);

	const uint8_t payload[] = { 0x01u, 0xFFu, 0xFFu }; /* PID=0xFFFF not registered */
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);
	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

static int test_tc_20_3_truncated_value(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_param(&s20, 0x0010u, 4u, param_getter, param_setter, NULL);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_SET_VALUES);
	ASSERT_TRUE(idx >= 0);

	/* N=1, PID=0x0010, only 3 value bytes instead of required 4 */
	const uint8_t payload[] = { 0x01u, 0x00u, 0x10u, 0x01u, 0x02u, 0x03u };
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

static int test_tc_20_3_setter_failure(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	pus_service_20_register_param(&s20, 0x0010u, 4u,
		param_getter, failing_setter, NULL);
	pus_service_20_register_handlers(&ctx, &s20);

	int idx = pus_handler_find(&ctx, PUS_SERVICE_PARAMETER_MANAGEMENT,
		PUS_SUBTYPE_PARAMETER_SET_VALUES);
	ASSERT_TRUE(idx >= 0);

	const uint8_t payload[] = { 0x01u, 0x00u, 0x10u, 0xCAu, 0xFEu, 0xBAu, 0xBEu };
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	tc.payload     = payload;
	tc.payload_len = sizeof(payload);
	ASSERT_EQ_INT(PUS_STATUS_HANDLER_FAILED,
		ctx.handler_table[idx].handler(&ctx, &tc,
			ctx.handler_table[idx].user_data));
	return 0;
}

static int test_emit_report_count_zero(void)
{
	pus_context_t        ctx = make_ctx();
	pus_service_20_ctx_t s20;
	pus_service_20_init(&s20);
	g_len = 0;

	/* count=0: payload is just the N byte; no param data */
	const uint16_t ids[] = { 0u }; /* content irrelevant — loop won't execute */
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_20_emit_report(&ctx, &s20, ids, 0u));

	/* header(12) + N(1) = 13 bytes */
	ASSERT_EQ_INT(PUS_TM_SEC_HEADER_LEN + 1, g_len);
	ASSERT_EQ_INT(0, g_buf[PUS_TM_SEC_HEADER_LEN]); /* N = 0 */
	ASSERT_EQ_INT(1, ctx.tm_counter);
	return 0;
}

static int test_register_handlers_null(void)
{
	pus_context_t        ctx;
	pus_service_20_ctx_t s20;
	pus_init(&ctx);
	pus_service_20_init(&s20);
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_service_20_register_handlers(NULL, &s20));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_service_20_register_handlers(&ctx, NULL));
	return 0;
}

pus_test_result_t test_pus_service_20_run_all(void)
{
	RUN_TEST(test_init_zeroes);
	RUN_TEST(test_register_and_emit_single);
	RUN_TEST(test_emit_multiple_params);
	RUN_TEST(test_set_values_tc);
	RUN_TEST(test_readonly_param_returns_failed);
	RUN_TEST(test_unknown_param_returns_no_handler);
	RUN_TEST(test_init_null);
	RUN_TEST(test_register_null_args);
	RUN_TEST(test_register_update_existing);
	RUN_TEST(test_register_table_full);
	RUN_TEST(test_emit_report_null);
	RUN_TEST(test_emit_report_getter_failure);
	RUN_TEST(test_emit_report_buffer_too_small);
	RUN_TEST(test_emit_report_no_sink);
	RUN_TEST(test_emit_report_count_zero);
	RUN_TEST(test_tc_20_1_success);
	RUN_TEST(test_tc_20_1_bad_length);
	RUN_TEST(test_tc_20_1_n_too_large);
	RUN_TEST(test_tc_20_1_payload_too_short);
	RUN_TEST(test_tc_20_3_bad_length);
	RUN_TEST(test_tc_20_3_truncated_pid);
	RUN_TEST(test_tc_20_3_unknown_pid);
	RUN_TEST(test_tc_20_3_truncated_value);
	RUN_TEST(test_tc_20_3_setter_failure);
	RUN_TEST(test_register_handlers_null);
	return (pus_test_result_t){ cunit_total_tests - cunit_overall_failures, cunit_total_tests };
}
