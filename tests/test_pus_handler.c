#include "cunit.h"
#include "test_runners.h"
#include "pus_handler.h"
#include "pus_context.h"
#include <string.h>

static pus_status_t dummy_handler(pus_context_t *ctx, const pus_tc_packet_t *tc, void *ud)
{
	(void)ctx; (void)tc; (void)ud;
	return PUS_STATUS_OK;
}

static pus_status_t other_handler(pus_context_t *ctx, const pus_tc_packet_t *tc, void *ud)
{
	(void)ctx; (void)tc; (void)ud;
	return PUS_STATUS_HANDLER_FAILED;
}

static int test_register_and_find(void)
{
	pus_context_t ctx;
	pus_init(&ctx);

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_register(&ctx, 17u, 1u, dummy_handler, NULL));

	int idx = pus_handler_find(&ctx, 17u, 1u);
	ASSERT_TRUE(idx >= 0);
	ASSERT_TRUE(ctx.handler_table[idx].handler == dummy_handler);

	return 0;
}

static int test_not_found(void)
{
	pus_context_t ctx;
	pus_init(&ctx);

	ASSERT_EQ_INT(-1, pus_handler_find(&ctx, 17u, 1u));

	return 0;
}

static int test_update_on_duplicate(void)
{
	pus_context_t ctx;
	pus_init(&ctx);

	pus_handler_register(&ctx, 17u, 1u, dummy_handler, NULL);
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_register(&ctx, 17u, 1u, other_handler, NULL));

	int idx = pus_handler_find(&ctx, 17u, 1u);
	ASSERT_TRUE(idx >= 0);
	ASSERT_TRUE(ctx.handler_table[idx].handler == other_handler);
	/* update must not add a new slot — verify by counting is_used entries */
	int used = 0;
	for (uint16_t i = 0u; i < PUS_MAX_TC_HANDLERS; i++) {
		if (ctx.handler_table[i].is_used) used++;
	}
	ASSERT_EQ_INT(1, used);

	return 0;
}

static int test_multiple_services(void)
{
	pus_context_t ctx;
	pus_init(&ctx);

	pus_handler_register(&ctx, 17u,  1u, dummy_handler, NULL);
	pus_handler_register(&ctx, 17u,  3u, dummy_handler, NULL);
	pus_handler_register(&ctx, 20u,  1u, dummy_handler, NULL);

	ASSERT_TRUE(pus_handler_find(&ctx, 17u, 1u) >= 0);
	ASSERT_TRUE(pus_handler_find(&ctx, 17u, 3u) >= 0);
	ASSERT_TRUE(pus_handler_find(&ctx, 20u, 1u) >= 0);
	ASSERT_EQ_INT(-1, pus_handler_find(&ctx, 20u, 3u));

	return 0;
}

static int test_table_full(void)
{
	pus_context_t ctx;
	pus_init(&ctx);

	for (uint8_t i = 0u; i < PUS_MAX_TC_HANDLERS; i++) {
		ASSERT_EQ_INT(PUS_STATUS_OK,
			pus_handler_register(&ctx, 1u, i, dummy_handler, NULL));
	}

	ASSERT_EQ_INT(PUS_STATUS_TABLE_FULL,
		pus_handler_register(&ctx, 1u, PUS_MAX_TC_HANDLERS, dummy_handler, NULL));

	return 0;
}

static int test_null_context(void)
{
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_handler_register(NULL, 1u, 1u, dummy_handler, NULL));
	ASSERT_EQ_INT(-1, pus_handler_find(NULL, 1u, 1u));

	return 0;
}

pus_test_result_t test_pus_handler_run_all(void)
{
	RUN_TEST(test_register_and_find);
	RUN_TEST(test_not_found);
	RUN_TEST(test_update_on_duplicate);
	RUN_TEST(test_multiple_services);
	RUN_TEST(test_table_full);
	RUN_TEST(test_null_context);
	return (pus_test_result_t){ cunit_total_tests - cunit_overall_failures, cunit_total_tests };
}
