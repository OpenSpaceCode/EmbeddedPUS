#ifndef PUS_SERVICE_1_H
#define PUS_SERVICE_1_H

#include "pus_context.h"
#include "pus_types.h"

#include <stdint.h>

/** @defgroup ack_flags TC secondary header ACK flag bits */
/** @{ */
#define PUS_ACK_ACCEPTANCE 0x08u /**< Request acceptance verification report. */
#define PUS_ACK_START 0x04u      /**< Request start of execution verification report. */
/**
 * @brief Request progress verification report.
 * @note  pus_tc_process() does NOT emit progress reports automatically.
 *        Handlers that need them must call pus_service_1_emit_success() with
 *        PUS_SUBTYPE_VERIFICATION_PROGRESS_SUCCESS (or _FAILURE) at suitable
 *        points during their own execution, checking this flag themselves:
 *        @code
 *        if (tc->sec_header.ack_flags & PUS_ACK_PROGRESS) {
 *            pus_service_1_emit_success(ctx, tc,
 *                PUS_SUBTYPE_VERIFICATION_PROGRESS_SUCCESS);
 *        }
 *        @endcode
 */
#define PUS_ACK_PROGRESS 0x02u
#define PUS_ACK_COMPLETION 0x01u /**< Request completion verification report. */
/** @} */

/**
 * @brief Build a ST[01] success report into a caller-provided buffer.
 *
 * @param[in,out] ctx      Active PUS context.
 * @param[in]     tc       The TC being verified.
 * @param[in]     subtype  Subtype constant from pus_services.h (e.g.
 * PUS_SUBTYPE_VERIFICATION_ACCEPTANCE_SUCCESS).
 * @param[out]    out      Output byte buffer.
 * @param[in]     capacity Output buffer size in bytes.
 * @param[out]    out_len  Receives the number of bytes written.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BUFFER_TOO_SMALL, or PUS_STATUS_OK.
 */
pus_status_t pus_service_1_build_success(pus_context_t *ctx,
                                         const pus_tc_packet_t *tc,
                                         pus_subtype_t subtype,
                                         uint8_t *out,
                                         uint16_t capacity,
                                         uint16_t *out_len);

/**
 * @brief Build a ST[01] failure report into a caller-provided buffer.
 *
 * @param[in,out] ctx          Active PUS context.
 * @param[in]     tc           The TC being verified.
 * @param[in]     subtype      Failure subtype (e.g. PUS_SUBTYPE_VERIFICATION_ROUTING_FAILURE).
 * @param[in]     failure_code Application-defined 2-byte failure code appended to the payload.
 * @param[out]    out          Output byte buffer.
 * @param[in]     capacity     Output buffer size in bytes.
 * @param[out]    out_len      Receives the number of bytes written.
 *
 * @return PUS_STATUS_NULL, PUS_STATUS_BUFFER_TOO_SMALL, or PUS_STATUS_OK.
 */
pus_status_t pus_service_1_build_failure(pus_context_t *ctx,
                                         const pus_tc_packet_t *tc,
                                         pus_subtype_t subtype,
                                         uint16_t failure_code,
                                         uint8_t *out,
                                         uint16_t capacity,
                                         uint16_t *out_len);

/**
 * @brief Build a success report and forward it to ctx->tm_sink.
 * Increments ctx->tm_counter regardless of whether a TM sink is configured.
 *
 * @param[in,out] ctx     Active PUS context.
 * @param[in]     tc      The TC being verified.
 * @param[in]     subtype Success subtype constant.
 *
 * @return PUS_STATUS_NULL or PUS_STATUS_OK.
 */
pus_status_t pus_service_1_emit_success(pus_context_t *ctx,
                                        const pus_tc_packet_t *tc,
                                        pus_subtype_t subtype);

/**
 * @brief Build a failure report and forward it to ctx->tm_sink.
 * Increments ctx->tm_counter regardless of whether a TM sink is configured.
 *
 * @param[in,out] ctx          Active PUS context.
 * @param[in]     tc           The TC being verified.
 * @param[in]     subtype      Failure subtype constant.
 * @param[in]     failure_code 2-byte failure code appended to the payload.
 *
 * @return PUS_STATUS_NULL or PUS_STATUS_OK.
 */
pus_status_t pus_service_1_emit_failure(pus_context_t *ctx,
                                        const pus_tc_packet_t *tc,
                                        pus_subtype_t subtype,
                                        uint16_t failure_code);

#endif /* PUS_SERVICE_1_H */
