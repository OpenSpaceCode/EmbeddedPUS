#ifndef PUS_SERVICES_H
#define PUS_SERVICES_H

/**
 * @file pus_services.h
 * @brief PUS service and subtype identifiers used by the public API.
 *
 * The constants are provided as raw numeric identifiers so they can be used in
 * packet headers, handler registration, and test vectors without additional
 * conversion.
 */

/**
 * @name PUS service identifiers
 * @{
 */
/** @brief Service 1: telecommand request verification. */
#define PUS_SERVICE_REQUEST_VERIFICATION 1u
/** @brief Service 3: housekeeping reporting. */
#define PUS_SERVICE_HOUSEKEEPING 3u
/** @brief Service 5: event reporting. */
#define PUS_SERVICE_EVENT_REPORTING 5u
/** @brief Service 17: test service. */
#define PUS_SERVICE_TEST 17u
/** @brief Service 20: parameter management. */
#define PUS_SERVICE_PARAMETER_MANAGEMENT 20u
/** @} */

/**
 * @name Service 1 request verification subtypes
 * @{
 */
/** @brief TM[1,1] acceptance success report. */
#define PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_SUCCESS 1u
/** @brief TM[1,2] acceptance failure report. */
#define PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_FAILURE 2u
/** @brief TM[1,3] execution start success report. */
#define PUS_SUBTYPE_VERIFICATION_START_SUCCESS 3u
/** @brief TM[1,4] execution start failure report. */
#define PUS_SUBTYPE_VERIFICATION_START_FAILURE 4u
/** @brief TM[1,5] execution progress success report. */
#define PUS_SUBTYPE_VERIFICATION_PROGRESS_SUCCESS 5u
/** @brief TM[1,6] execution progress failure report. */
#define PUS_SUBTYPE_VERIFICATION_PROGRESS_FAILURE 6u
/** @brief TM[1,7] execution completion success report. */
#define PUS_SUBTYPE_VERIFICATION_COMPLETION_SUCCESS 7u
/** @brief TM[1,8] execution completion failure report. */
#define PUS_SUBTYPE_VERIFICATION_COMPLETION_FAILURE 8u
/** @brief TM[1,10] routing failure report. */
#define PUS_SUBTYPE_VERIFICATION_ROUTING_FAILURE 10u
/** @} */

/**
 * @name Service 3 housekeeping subtypes
 * @{
 */
/** @brief TM[3,25] housekeeping parameter report. */
#define PUS_SUBTYPE_HOUSEKEEPING_PARAMETER_REPORT 25u
/** @brief TM[3,26] housekeeping diagnostic report. */
#define PUS_SUBTYPE_HOUSEKEEPING_DIAGNOSTIC_REPORT 26u
/** @brief TC[3,27] request to generate a housekeeping parameter report. */
#define PUS_SUBTYPE_HOUSEKEEPING_GENERATE_PARAMETER_REPORT 27u
/** @brief TC[3,28] request to generate a housekeeping diagnostic report. */
#define PUS_SUBTYPE_HOUSEKEEPING_GENERATE_DIAGNOSTIC_REPORT 28u
/** @} */

/**
 * @name Service 5 event reporting subtypes
 * @{
 */
/** @brief TM[5,1] informational event report. */
#define PUS_SUBTYPE_EVENT_INFO 1u
/** @brief TM[5,2] low-severity event report. */
#define PUS_SUBTYPE_EVENT_LOW_SEVERITY 2u
/** @brief TM[5,3] medium-severity event report. */
#define PUS_SUBTYPE_EVENT_MEDIUM_SEVERITY 3u
/** @brief TM[5,4] high-severity event report. */
#define PUS_SUBTYPE_EVENT_HIGH_SEVERITY 4u
/** @} */

/**
 * @name Service 17 test subtypes
 * @{
 */
/** @brief TC[17,1] are-you-alive request. */
#define PUS_SUBTYPE_TEST_ARE_YOU_ALIVE 1u
/** @brief TM[17,2] are-you-alive report. */
#define PUS_SUBTYPE_TEST_ARE_YOU_ALIVE_REPORT 2u
/** @brief TC[17,3] on-board connection test request. */
#define PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION 3u
/** @brief TM[17,4] on-board connection test report. */
#define PUS_SUBTYPE_TEST_ON_BOARD_CONNECTION_REPORT 4u
/** @} */

/**
 * @name Service 20 parameter management subtypes
 * @{
 */
/** @brief TC[20,1] report parameter values request. */
#define PUS_SUBTYPE_PARAMETER_REPORT_VALUES 1u
/** @brief TM[20,2] parameter value report. */
#define PUS_SUBTYPE_PARAMETER_VALUE_REPORT 2u
/** @brief TC[20,3] set parameter values request. */
#define PUS_SUBTYPE_PARAMETER_SET_VALUES 3u
/** @} */

#endif /* PUS_SERVICES_H */
