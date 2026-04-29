#include "cunit.h"
#include <stdio.h>

int cunit_overall_failures = 0;

extern int test_pus_types_run_all(void);
extern int test_pus_codec_run_all(void);
extern int test_pus_handler_run_all(void);

int main(void)
{
	test_pus_types_run_all();
	test_pus_codec_run_all();
	test_pus_handler_run_all();

	if (cunit_overall_failures == 0)
	{
		printf("ALL TESTS PASSED\n");
		return 0;
	}
	else
	{
		printf("%d TEST(S) FAILED\n", cunit_overall_failures);
		return 1;
	}
}
