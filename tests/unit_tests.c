#include "cunit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_case_0(void) {

  return 0;
}

int main(void) {
  RUN_TEST(test_case_0);

  if (cunit_overall_failures == 0) {
    printf("ALL TESTS PASSED\n");
    return 0;
  } else {
    printf("%d TEST(S) FAILED\n", cunit_overall_failures);
    return 1;
  }
}