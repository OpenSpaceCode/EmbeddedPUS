#ifndef PUS_SERVICES_H
#define PUS_SERVICES_H

/** @defgroup services PUS Service identifiers */
/** @{ */
#define PUS_SERVICE_REQUEST_VERIFICATION 1u  /**< ST[01] Request Verification. */
#define PUS_SERVICE_HOUSEKEEPING         3u  /**< ST[03] Housekeeping. */
#define PUS_SERVICE_EVENT_REPORTING      5u  /**< ST[05] Event Reporting. */
#define PUS_SERVICE_TEST                 17u /**< ST[17] Test. */
#define PUS_SERVICE_PARAMETER_MANAGEMENT 20u /**< ST[20] Parameter Management. */
/** @} */

/** @defgroup subtypes_st01 ST[01] Request Verification subtypes */
/** @{ */
#define PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_SUCCESS 1u  /**< TM[1,1]  Acceptance success. */
#define PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_FAILURE 2u  /**< TM[1,2]  Acceptance failure. */
#define PUS_SUBTYPE_VERIFICATION_START_SUCCESS      3u  /**< TM[1,3]  Start of execution success. */
#define PUS_SUBTYPE_VERIFICATION_START_FAILURE      4u  /**< TM[1,4]  Start of execution failure. */
#define PUS_SUBTYPE_VERIFICATION_PROGRESS_SUCCESS   5u  /**< TM[1,5]  Progress success. */
#define PUS_SUBTYPE_VERIFICATION_PROGRESS_FAILURE   6u  /**< TM[1,6]  Progress failure. */
#define PUS_SUBTYPE_VERIFICATION_COMPLETION_SUCCESS 7u  /**< TM[1,7]  Completion success. */
#define PUS_SUBTYPE_VERIFICATION_COMPLETION_FAILURE 8u  /**< TM[1,8]  Completion failure. */
#define PUS_SUBTYPE_VERIFICATION_ROUTING_FAILURE    10u /**< TM[1,10] Routing failure. */
/** @} */

/** @defgroup subtypes_st03 ST[03] Housekeeping subtypes */
/** @{ */
#define PUS_SUBTYPE_HOUSEKEEPING_PARAMETER_REPORT  25u /**< TM[3,25] HK parameter report. */
#define PUS_SUBTYPE_HOUSEKEEPING_DIAGNOSTIC_REPORT 26u /**< TM[3,26] Diagnostic report. */
/** @} */

/** @defgroup subtypes_st05 ST[05] Event Reporting subtypes */
/** @{ */
#define PUS_SUBTYPE_EVENT_INFO            1u /**< TM[5,1] Informative event. */
#define PUS_SUBTYPE_EVENT_LOW_SEVERITY    2u /**< TM[5,2] Low-severity anomaly. */
#define PUS_SUBTYPE_EVENT_MEDIUM_SEVERITY 3u /**< TM[5,3] Medium-severity anomaly. */
#define PUS_SUBTYPE_EVENT_HIGH_SEVERITY   4u /**< TM[5,4] High-severity anomaly. */
/** @} */

/** @defgroup subtypes_st17 ST[17] Test subtypes */
/** @{ */
#define PUS_SUBTYPE_TEST_ARE_YOU_ALIVE              1u /**< TC[17,1] Are-you-alive test. */
#define PUS_SUBTYPE_TEST_ARE_YOU_ALIVE_REPORT       2u /**< TM[17,2] Are-you-alive report. */
#define PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION        3u /**< TC[17,3] On-board connection test. */
#define PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION_REPORT 4u /**< TM[17,4] Connection test report. */
/** @} */

/** @defgroup subtypes_st20 ST[20] Parameter Management subtypes */
/** @{ */
#define PUS_SUBTYPE_PARAMETER_REPORT_VALUES 1u /**< TC[20,1] Request parameter value report. */
#define PUS_SUBTYPE_PARAMETER_VALUE_REPORT  2u /**< TM[20,2] Parameter value report. */
#define PUS_SUBTYPE_PARAMETER_SET_VALUES    3u /**< TC[20,3] Set parameter values. */
/** @} */

#endif /* PUS_SERVICES_H */
