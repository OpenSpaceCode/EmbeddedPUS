#ifndef TEST_RUNNERS_H
#define TEST_RUNNERS_H

typedef struct {
	int passed;
	int total;
} pus_test_result_t;

pus_test_result_t test_pus_run_all(void);
pus_test_result_t test_pus_codec_run_all(void);
pus_test_result_t test_pus_handler_run_all(void);
pus_test_result_t test_pus_service_1_run_all(void);
pus_test_result_t test_pus_service_3_run_all(void);
pus_test_result_t test_pus_service_5_run_all(void);
pus_test_result_t test_pus_service_17_run_all(void);
pus_test_result_t test_pus_service_20_run_all(void);

#endif /* TEST_RUNNERS_H */
