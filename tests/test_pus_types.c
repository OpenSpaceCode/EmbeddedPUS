#include "cunit.h"
#include "../include/pus.h"

int test_pus_types_run_all(void);

static int test_pus_service_constants(void)
{
	ASSERT_EQ_INT(1, PUS_SERVICE_REQUEST_VERIFICATION);
	ASSERT_EQ_INT(3, PUS_SERVICE_HOUSEKEEPING);
	ASSERT_EQ_INT(5, PUS_SERVICE_EVENT_REPORTING);
	ASSERT_EQ_INT(17, PUS_SERVICE_TEST);
	ASSERT_EQ_INT(20, PUS_SERVICE_PARAMETER_MANAGEMENT);
	ASSERT_EQ_INT(1, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE);
	ASSERT_EQ_INT(2, PUS_SUBTYPE_TEST_ARE_YOU_ALIVE_REPORT);

	return 0;
}

static int test_pus_secondary_header_lengths(void)
{
	ASSERT_EQ_INT(10, PUS_TC_SEC_HEADER_LEN);
	ASSERT_EQ_INT(12, PUS_TM_SEC_HEADER_LEN);

	return 0;
}

int test_pus_types_run_all(void)
{
	RUN_TEST(test_pus_service_constants);
	RUN_TEST(test_pus_secondary_header_lengths);

	return 0;
}
