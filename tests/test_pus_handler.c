#include <string.h>

#include "cunit.h"
#include "../include/pus.h"

int test_pus_handler_run_all(void);

static pus_status_t test_handler_ok(
	pus_context_t *ctx,
	const pus_tc_packet_t *tc,
	void *user_data)
{
	(void)ctx;
	(void)tc;
	(void)user_data;
	return PUS_STATUS_OK;
}

static pus_status_t test_handler_alt(
	pus_context_t *ctx,
	const pus_tc_packet_t *tc,
	void *user_data)
{
	(void)ctx;
	(void)tc;
	(void)user_data;
	return PUS_STATUS_HANDLER_FAILED;
}

static int test_pus_init_rejects_null(void)
{
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_init(NULL));
	return 0;
}

static int test_pus_init_clears_context(void)
{
	pus_context_t ctx;
	uint16_t i = 0u;

	memset(&ctx, 0xa5, sizeof(ctx));

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	ASSERT_EQ_INT(0, ctx.handler_count);

	for (i = 0u; i < PUS_MAX_TC_HANDLERS; i++)
	{
		ASSERT_EQ_INT(0, ctx.handler_table[i].is_used);
	}

	return 0;
}

static int test_pus_handler_register_adds_entry(void)
{
	pus_context_t ctx;
	void *user_data = &ctx;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_handler_register(
									 &ctx, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
									 test_handler_ok, user_data));

	ASSERT_EQ_INT(1, ctx.handler_count);
	ASSERT_EQ_INT(1, ctx.handler_table[0].is_used);
	ASSERT_EQ_INT(PUS_SERVICE_TEST, ctx.handler_table[0].service);
	ASSERT_EQ_INT(PUS_SUBTYPE_TEST_ARE_YOU_ALIVE, ctx.handler_table[0].subtype);
	ASSERT_TRUE(ctx.handler_table[0].handler == test_handler_ok);
	ASSERT_TRUE(ctx.handler_table[0].user_data == user_data);

	return 0;
}

static int test_pus_handler_register_updates_existing_entry(void)
{
	pus_context_t ctx;
	void *first_user_data = &ctx;
	void *second_user_data = &ctx.handler_count;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_handler_register(
									 &ctx, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
									 test_handler_ok, first_user_data));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_handler_register(
									 &ctx, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
									 test_handler_alt, second_user_data));

	ASSERT_EQ_INT(1, ctx.handler_count);
	ASSERT_TRUE(ctx.handler_table[0].handler == test_handler_alt);
	ASSERT_TRUE(ctx.handler_table[0].user_data == second_user_data);

	return 0;
}

static int test_pus_handler_register_rejects_null_inputs(void)
{
	pus_context_t ctx;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_handler_register(
									   NULL, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
									   test_handler_ok, NULL));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_handler_register(
									   &ctx, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
									   NULL, NULL));

	return 0;
}

static int test_pus_handler_register_fails_when_table_is_full(void)
{
	pus_context_t ctx;
	uint16_t i = 0u;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));

	for (i = 0u; i < PUS_MAX_TC_HANDLERS; i++)
	{
		ASSERT_EQ_INT(PUS_STATUS_OK, pus_handler_register(
										 &ctx, (pus_service_t)(i + 1u), (pus_subtype_t)(i + 2u),
										 test_handler_ok, NULL));
	}

	ASSERT_EQ_INT(PUS_MAX_TC_HANDLERS, ctx.handler_count);
	ASSERT_EQ_INT(PUS_STATUS_TABLE_FULL, pus_handler_register(
											 &ctx, 0xf0u, 0x0fu, test_handler_ok, NULL));

	return 0;
}

int test_pus_handler_run_all(void)
{
	RUN_TEST(test_pus_init_rejects_null);
	RUN_TEST(test_pus_init_clears_context);
	RUN_TEST(test_pus_handler_register_adds_entry);
	RUN_TEST(test_pus_handler_register_updates_existing_entry);
	RUN_TEST(test_pus_handler_register_rejects_null_inputs);
	RUN_TEST(test_pus_handler_register_fails_when_table_is_full);

	return 0;
}
