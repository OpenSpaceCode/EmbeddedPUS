#include "cunit.h"
#include "test_runners.h"
#include "test_helpers.h"
#include "pus_service_3.h"
#include "pus_context.h"
#include "pus_services.h"
#include <string.h>

#define g_buf th_buf
#define g_len th_len
#define make_ctx th_make_ctx

/* Provider that writes 2 bytes: 0xCA, 0xFE */
static pus_status_t simple_provider(uint16_t sid, uint8_t *buf,
                                     uint16_t cap, uint16_t *out_len, void *ud)
{
	(void)sid; (void)cap; (void)ud;
	buf[0]   = 0xCAu;
	buf[1]   = 0xFEu;
	*out_len = 2u;
	return PUS_STATUS_OK;
}

static int test_init_zeroes(void)
{
	pus_service_3_ctx_t s3;
	memset(&s3, 0xFFu, sizeof(s3));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_service_3_init(&s3));

	for (uint8_t i = 0u; i < PUS_SERVICE_3_MAX_STRUCTURES; i++) {
		ASSERT_EQ_INT(0, s3.structures[i].is_used);
	}
	return 0;
}

static int test_register_hk_emit(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);
	g_len = 0;

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_3_register_hk(&s3, 0x0001u, simple_provider, NULL));
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_3_emit_hk(&ctx, &s3, 0x0001u));

	ASSERT_TRUE(g_len > 0);
	/* TM header: service=3, subtype=25 */
	ASSERT_EQ_INT(PUS_SERVICE_HOUSEKEEPING, g_buf[1]);
	ASSERT_EQ_INT(PUS_SUBTYPE_HOUSEKEEPING_PARAMETER_REPORT, g_buf[2]);
	/* payload starts at byte 12: SID hi, SID lo, 0xCA, 0xFE */
	ASSERT_EQ_INT(0x00, g_buf[12]);
	ASSERT_EQ_INT(0x01, g_buf[13]);
	ASSERT_EQ_INT(0xCA, g_buf[14]);
	ASSERT_EQ_INT(0xFE, g_buf[15]);

	return 0;
}

static int test_register_diag_emit(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);
	g_len = 0;

	pus_service_3_register_diag(&s3, 0x0002u, simple_provider, NULL);
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_3_emit_diag(&ctx, &s3, 0x0002u));

	ASSERT_EQ_INT(PUS_SUBTYPE_HOUSEKEEPING_DIAGNOSTIC_REPORT, g_buf[2]);
	ASSERT_EQ_INT(0x00, g_buf[12]);
	ASSERT_EQ_INT(0x02, g_buf[13]);

	return 0;
}

static int test_unknown_sid_returns_no_handler(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);

	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER,
		pus_service_3_emit_hk(&ctx, &s3, 0xFFFFu));

	return 0;
}

static int test_table_full(void)
{
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);

	for (uint8_t i = 0u; i < PUS_SERVICE_3_MAX_STRUCTURES; i++) {
		ASSERT_EQ_INT(PUS_STATUS_OK,
			pus_service_3_register_hk(&s3, (uint16_t)i,
				simple_provider, NULL));
	}

	ASSERT_EQ_INT(PUS_STATUS_TABLE_FULL,
		pus_service_3_register_hk(&s3, 0xFFFFu, simple_provider, NULL));

	return 0;
}

static int test_update_existing(void)
{
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);

	pus_service_3_register_hk(&s3, 0x0001u, simple_provider, NULL);
	/* re-register same SID should succeed and update in place */
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_service_3_register_hk(&s3, 0x0001u, simple_provider, (void *)1));

	/* count should still be 1 */
	int used = 0;
	for (uint8_t i = 0u; i < PUS_SERVICE_3_MAX_STRUCTURES; i++) {
		if (s3.structures[i].is_used) used++;
	}
	ASSERT_EQ_INT(1, used);

	return 0;
}

static uint32_t test_time_source(void *ud) { (void)ud; return 0xABCD1234u; }

static pus_status_t failing_provider(uint16_t sid, uint8_t *buf,
                                      uint16_t cap, uint16_t *out_len, void *ud)
{
	(void)sid; (void)buf; (void)cap; (void)out_len; (void)ud;
	return PUS_STATUS_HANDLER_FAILED;
}

static pus_status_t overflow_provider(uint16_t sid, uint8_t *buf,
                                       uint16_t cap, uint16_t *out_len, void *ud)
{
	(void)sid; (void)buf; (void)ud;
	*out_len = (uint16_t)(cap + 1u);
	return PUS_STATUS_OK;
}

static int test_register_null_s3(void)
{
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_service_3_register_hk(NULL, 0x0001u, simple_provider, NULL));
	return 0;
}

static int test_register_null_provider(void)
{
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_service_3_register_hk(&s3, 0x0001u, NULL, NULL));
	return 0;
}

static int test_emit_null_ctx(void)
{
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_service_3_emit_hk(NULL, &s3, 0x0001u));
	return 0;
}

static int test_emit_null_s3(void)
{
	pus_context_t ctx = make_ctx();
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_service_3_emit_hk(&ctx, NULL, 0x0001u));
	return 0;
}

static int test_provider_failure(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);
	pus_service_3_register_hk(&s3, 0x0001u, failing_provider, NULL);
	ASSERT_EQ_INT(PUS_STATUS_HANDLER_FAILED,
		pus_service_3_emit_hk(&ctx, &s3, 0x0001u));
	return 0;
}

static int test_provider_data_overflow(void)
{
	pus_context_t ctx = make_ctx();
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);
	pus_service_3_register_hk(&s3, 0x0001u, overflow_provider, NULL);
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH,
		pus_service_3_emit_hk(&ctx, &s3, 0x0001u));
	return 0;
}

static int test_emit_no_sink(void)
{
	pus_context_t ctx;
	pus_init(&ctx); /* no sink */
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);
	pus_service_3_register_hk(&s3, 0x0001u, simple_provider, NULL);
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_service_3_emit_hk(&ctx, &s3, 0x0001u));
	return 0;
}

static int test_emit_with_time_source(void)
{
	pus_context_t ctx = make_ctx();
	ctx.time_source = test_time_source;
	pus_service_3_ctx_t s3;
	pus_service_3_init(&s3);
	pus_service_3_register_hk(&s3, 0x0001u, simple_provider, NULL);
	g_len = 0;
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_service_3_emit_hk(&ctx, &s3, 0x0001u));
	ASSERT_EQ_INT(0xAB, g_buf[7]);
	ASSERT_EQ_INT(0xCD, g_buf[8]);
	return 0;
}

pus_test_result_t test_pus_service_3_run_all(void)
{
	RUN_TEST(test_init_zeroes);
	RUN_TEST(test_register_hk_emit);
	RUN_TEST(test_register_diag_emit);
	RUN_TEST(test_unknown_sid_returns_no_handler);
	RUN_TEST(test_table_full);
	RUN_TEST(test_update_existing);
	RUN_TEST(test_register_null_s3);
	RUN_TEST(test_register_null_provider);
	RUN_TEST(test_emit_null_ctx);
	RUN_TEST(test_emit_null_s3);
	RUN_TEST(test_provider_failure);
	RUN_TEST(test_provider_data_overflow);
	RUN_TEST(test_emit_no_sink);
	RUN_TEST(test_emit_with_time_source);
	return (pus_test_result_t){ cunit_total_tests - cunit_overall_failures, cunit_total_tests };
}
