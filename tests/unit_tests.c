#include "test_runners.h"
#include <stdio.h>

#define REPORT(label, r) \
	printf("  %-14s Passed %d/%d\n\n", label ":", (r).passed, (r).total)

int main(void)
{
	pus_test_result_t r;
	int total_passed = 0;
	int total_tests  = 0;

	r = test_pus_run_all();            REPORT("pus",        r); total_passed += r.passed; total_tests += r.total;
	r = test_pus_codec_run_all();      REPORT("codec",      r); total_passed += r.passed; total_tests += r.total;
	r = test_pus_handler_run_all();    REPORT("handler",    r); total_passed += r.passed; total_tests += r.total;
	r = test_pus_service_1_run_all();  REPORT("service_1",  r); total_passed += r.passed; total_tests += r.total;
	r = test_pus_service_3_run_all();  REPORT("service_3",  r); total_passed += r.passed; total_tests += r.total;
	r = test_pus_service_5_run_all();  REPORT("service_5",  r); total_passed += r.passed; total_tests += r.total;
	r = test_pus_service_17_run_all(); REPORT("service_17", r); total_passed += r.passed; total_tests += r.total;
	r = test_pus_service_20_run_all(); REPORT("service_20", r); total_passed += r.passed; total_tests += r.total;

	printf("  ------------------------------\n");
	printf("  %-14s Passed %d/%d\n", "All UT:", total_passed, total_tests);

	return (total_passed == total_tests) ? 0 : 1;
}
