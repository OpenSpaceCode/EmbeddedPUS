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

/* TC Decode tests */

static int test_pus_tc_decode_rejects_null(void)
{
	pus_tc_packet_t tc;

	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_decode(NULL, 10u, &tc));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_decode((const uint8_t *)"test", 10u, NULL));

	return 0;
}

static int test_pus_tc_decode_rejects_short_buffer(void)
{
	pus_tc_packet_t tc;
	const uint8_t data[] = {0x10, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

	/* Buffer shorter than TC secondary header */
	ASSERT_EQ_INT(PUS_STATUS_BAD_LENGTH, pus_tc_decode(data, 5u, &tc));

	return 0;
}

static int test_pus_tc_decode_rejects_bad_version(void)
{
	pus_tc_packet_t tc;
	/* Version 0xF in the high nibble = invalid version */
	const uint8_t data[] = {0xF1, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	ASSERT_EQ_INT(PUS_STATUS_BAD_VERSION, pus_tc_decode(data, sizeof(data), &tc));

	return 0;
}

static int test_pus_tc_decode_succeeds(void)
{
	pus_tc_packet_t tc;
	/* Valid TC: version=1, ACK=0, service=17, subtype=1, source=1, time=0 */
	const uint8_t data[] = {0x10, 0x11, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD};

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_decode(data, sizeof(data), &tc));
	ASSERT_EQ_INT(1u, tc.sec_header.version);
	ASSERT_EQ_INT(0u, tc.sec_header.ack_flags);
	ASSERT_EQ_INT(17u, tc.sec_header.service_type_id);
	ASSERT_EQ_INT(1u, tc.sec_header.subtype_id);
	ASSERT_EQ_INT(1u, tc.sec_header.source_id);
	ASSERT_EQ_INT(0u, tc.sec_header.time);
	ASSERT_TRUE(tc.payload == &data[10]);
	ASSERT_EQ_INT(2u, tc.payload_len);

	return 0;
}

static int test_pus_tc_decode_parses_ack_flags(void)
{
	pus_tc_packet_t tc;
	/* ACK flags = 0xA (1010b) = accept + progress */
	const uint8_t data[] = {0x1A, 0x11, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_decode(data, sizeof(data), &tc));
	ASSERT_EQ_INT(0x0Au, tc.sec_header.ack_flags);

	return 0;
}

/* TC Process tests */

static int test_pus_tc_process_rejects_null(void)
{
	pus_context_t ctx;
	const uint8_t data[] = {0x10, 0x11, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_process(NULL, data, sizeof(data)));
	ASSERT_EQ_INT(PUS_STATUS_NULL, pus_tc_process(&ctx, NULL, sizeof(data)));

	return 0;
}

static int test_pus_tc_process_no_handler(void)
{
	pus_context_t ctx;
	const uint8_t data[] = {0x10, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	/* No handler registered for service=255, subtype=255 */
	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER, pus_tc_process(&ctx, data, sizeof(data)));

	return 0;
}

static int test_pus_tc_process_handler_success(void)
{
	pus_context_t ctx;
	const uint8_t data[] = {0x10, 0x11, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_handler_register(
									 &ctx, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
									 test_handler_ok, NULL));

	/* Handler returns PUS_STATUS_OK */
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_tc_process(&ctx, data, sizeof(data)));

	return 0;
}

static int test_pus_tc_process_handler_failure(void)
{
	pus_context_t ctx;
	const uint8_t data[] = {0x10, 0x11, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_handler_register(
									 &ctx, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
									 test_handler_alt, NULL));

	/* Handler returns PUS_STATUS_HANDLER_FAILED */
	ASSERT_EQ_INT(PUS_STATUS_HANDLER_FAILED, pus_tc_process(&ctx, data, sizeof(data)));

	return 0;
}

static int test_pus_tc_process_unknown_service(void)
{
	pus_context_t ctx;
	const uint8_t data[] = {0x10, 0x99, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	/* No handler registered for service=153 */
	ASSERT_EQ_INT(PUS_STATUS_NO_HANDLER, pus_tc_process(&ctx, data, sizeof(data)));

	return 0;
}

static int test_pus_handler_find_returns_index(void)
{
	pus_context_t ctx;

	ASSERT_EQ_INT(PUS_STATUS_OK, pus_init(&ctx));
	ASSERT_EQ_INT(PUS_STATUS_OK, pus_handler_register(
									 &ctx, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE,
									 test_handler_ok, NULL));

	/* Handler should be found */
	ASSERT_TRUE(pus_handler_find(&ctx, PUS_SERVICE_TEST, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE) >= 0);

	/* Non-existent handler should return -1 */
	ASSERT_TRUE(pus_handler_find(&ctx, PUS_SERVICE_TEST, 99u) < 0);
	ASSERT_TRUE(pus_handler_find(&ctx, 99u, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE) < 0);

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
	RUN_TEST(test_pus_tc_decode_rejects_null);
	RUN_TEST(test_pus_tc_decode_rejects_short_buffer);
	RUN_TEST(test_pus_tc_decode_rejects_bad_version);
	RUN_TEST(test_pus_tc_decode_succeeds);
	RUN_TEST(test_pus_tc_decode_parses_ack_flags);
	RUN_TEST(test_pus_tc_process_rejects_null);
	RUN_TEST(test_pus_tc_process_no_handler);
	RUN_TEST(test_pus_tc_process_handler_success);
	RUN_TEST(test_pus_tc_process_handler_failure);
	RUN_TEST(test_pus_tc_process_unknown_service);
	RUN_TEST(test_pus_handler_find_returns_index);

	return 0;
}
