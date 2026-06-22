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

static pus_tc_packet_t make_tc(void)
{
	pus_tc_packet_t tc;
	memset(&tc, 0, sizeof(tc));
	return tc;
}

static int test_register_and_find(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	pus_tc_packet_t tc = make_tc();

	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_register(&ctx, 17u, 1u, dummy_handler, NULL));

	/* Invocation returns whatever dummy_handler returns (OK) */
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_invoke(&ctx, 17u, 1u, &tc));

	return 0;
}

static int test_not_found(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	pus_tc_packet_t tc = make_tc();

	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER,
		pus_handler_invoke(&ctx, 17u, 1u, &tc));

	return 0;
}

static int test_update_on_duplicate(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	pus_tc_packet_t tc = make_tc();

	pus_handler_register(&ctx, 17u, 1u, dummy_handler, NULL);
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_register(&ctx, 17u, 1u, other_handler, NULL));

	/* After update, other_handler (returns HANDLER_FAILED) must be active */
	ASSERT_EQ_INT(PUS_STATUS_HANDLER_FAILED,
		pus_handler_invoke(&ctx, 17u, 1u, &tc));

	/* Update must not add a new slot — verify by counting is_used entries */
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
	pus_tc_packet_t tc = make_tc();

	pus_handler_register(&ctx, 17u,  1u, dummy_handler, NULL);
	pus_handler_register(&ctx, 17u,  3u, dummy_handler, NULL);
	pus_handler_register(&ctx, 20u,  1u, dummy_handler, NULL);

	ASSERT_TRUE(pus_handler_invoke(&ctx, 17u, 1u, &tc) != PUS_STATUS_NO_HANDLER);
	ASSERT_TRUE(pus_handler_invoke(&ctx, 17u, 3u, &tc) != PUS_STATUS_NO_HANDLER);
	ASSERT_TRUE(pus_handler_invoke(&ctx, 20u, 1u, &tc) != PUS_STATUS_NO_HANDLER);
	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER,
		pus_handler_invoke(&ctx, 20u, 3u, &tc));

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
	pus_tc_packet_t tc = make_tc();

	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_handler_register(NULL, 1u, 1u, dummy_handler, NULL));
	ASSERT_EQ_INT(PUS_STATUS_NULL,
		pus_handler_invoke(NULL, 1u, 1u, &tc));

	return 0;
}

static int test_deregister_with_null_handler(void)
{
	pus_context_t ctx;
	pus_init(&ctx);
	pus_tc_packet_t tc = make_tc();

	/* Deregistering when nothing is registered returns NO_HANDLER */
	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER,
		pus_handler_register(&ctx, 17u, 1u, NULL, NULL));

	/* Register, then deregister */
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_register(&ctx, 17u, 1u, dummy_handler, NULL));
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_register(&ctx, 17u, 1u, NULL, NULL));

	/* Slot must be free */
	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER,
		pus_handler_invoke(&ctx, 17u, 1u, &tc));

	/* Freed slot can be reused */
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_register(&ctx, 17u, 1u, dummy_handler, NULL));
	ASSERT_EQ_INT(PUS_STATUS_OK,
		pus_handler_invoke(&ctx, 17u, 1u, &tc));

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
	RUN_TEST(test_deregister_with_null_handler);
	return (pus_test_result_t){ cunit_total_tests - cunit_overall_failures, cunit_total_tests };
}
