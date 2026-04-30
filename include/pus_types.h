#ifndef PUS_TYPES_H
#define PUS_TYPES_H

#include <stdint.h>

/**
 * @file pus_types.h
 * @brief Core EmbeddedPUS scalar types, status codes, and packet structures.
 *
 * This header contains the public data model shared by the codec, handler,
 * context, and top-level PUS APIs. Multi-byte fields are encoded on the wire
 * by the codec functions in big-endian order.
 */

/**
 * @brief Numeric identifier of an ECSS PUS service.
 *
 * Use the service constants declared in pus_services.h where available.
 */
typedef uint8_t pus_service_t;

/**
 * @brief Numeric identifier of a service-specific PUS subtype.
 *
 * Subtype values are interpreted together with the service identifier.
 */
typedef uint8_t pus_subtype_t;

/** @brief Encoded length, in bytes, of a TC secondary header. */
#define PUS_TC_SEC_HEADER_LEN 10u

/** @brief Encoded length, in bytes, of a TM secondary header. */
#define PUS_TM_SEC_HEADER_LEN 12u

/**
 * @brief Result codes returned by EmbeddedPUS public APIs.
 *
 * Functions return @ref PUS_STATUS_OK on success and one of the other values
 * when validation, routing, buffer management, or user handler execution fails.
 */
typedef enum
{
    PUS_STATUS_OK = 0,          /**< Operation completed successfully. */
    PUS_STATUS_NULL,            /**< A required pointer argument was NULL. */
    PUS_STATUS_BAD_LENGTH,      /**< Input data was shorter than the required packet/header length. */
    PUS_STATUS_BAD_VERSION,     /**< Decoded PUS version did not match @ref PUS_VERSION. */
    PUS_STATUS_BAD_SERVICE,     /**< Service identifier is unsupported or invalid for the operation. */
    PUS_STATUS_NO_HANDLER,      /**< No TC handler is registered for the requested service/subtype. */
    PUS_STATUS_HANDLER_FAILED,  /**< A registered TC handler reported failure. */
    PUS_STATUS_BUFFER_TOO_SMALL, /**< Output buffer capacity is insufficient for the encoded data. */
    PUS_STATUS_TABLE_FULL       /**< The handler registry has no free slots. */
} pus_status_t;

/**
 * @brief Decoded PUS telemetry secondary header.
 *
 * The fields represent the logical TM header after decoding. Do not serialize
 * this structure with raw memory copies for external data exchange; use
 * pus_tm_sec_header_encode() so byte order and field packing stay explicit.
 */
typedef struct
{
    uint8_t version : 4;              /**< PUS version field, expected to match @ref PUS_VERSION. */
    uint8_t time_ref_status : 4;      /**< Time reference status nibble carried in TM packets. */
    uint8_t service_type_id : 8;      /**< PUS service type that produced this telemetry. */
    uint8_t subtype_id : 8;           /**< Service-specific telemetry subtype. */
    uint16_t msg_type_counter : 16;   /**< Message counter value assigned by the emitting context. */
    uint16_t destination_id : 16;     /**< Destination identifier for the telemetry packet. */
    uint32_t time;                    /**< Packet timestamp value as carried on the wire. */
    uint8_t spare;                    /**< Spare byte reserved by the secondary header format. */
} pus_tm_sec_header_t;

/**
 * @brief Decoded PUS telecommand secondary header.
 *
 * The fields represent the logical TC header after decoding. Use
 * pus_tc_sec_header_encode() and pus_tc_sec_header_decode() when converting
 * between this structure and wire-format bytes.
 */
typedef struct
{
    uint8_t version : 4;          /**< PUS version field, expected to match @ref PUS_VERSION. */
    uint8_t ack_flags : 4;        /**< Service 1 verification request flags from the TC header. */
    uint8_t service_type_id : 8;  /**< PUS service type requested by the telecommand. */
    uint8_t subtype_id : 8;       /**< Service-specific telecommand subtype. */
    uint16_t source_id : 16;      /**< Source identifier of the telecommand issuer. */
    uint32_t time;                /**< Packet timestamp value as carried on the wire. */
    uint8_t spare;                /**< Spare byte reserved by the secondary header format. */
} pus_tc_sec_header_t;

/**
 * @brief Decoded telecommand packet view.
 *
 * The payload pointer references the caller-provided input buffer passed to
 * pus_tc_decode(); the library does not copy or own that memory.
 */
typedef struct
{
    pus_tc_sec_header_t sec_header; /**< Decoded TC secondary header. */
    const uint8_t *payload;         /**< Pointer to the first payload byte in the decoded TC buffer. */
    uint16_t payload_len;           /**< Payload length in bytes. */
} pus_tc_packet_t;

/**
 * @brief Telemetry packet view.
 *
 * This structure is useful when a caller wants to describe TM content as a
 * header plus payload before encoding or forwarding it.
 */
typedef struct
{
    pus_tm_sec_header_t sec_header; /**< TM secondary header fields. */
    const uint8_t *payload;         /**< Pointer to payload bytes; may be NULL when payload_len is 0. */
    uint16_t payload_len;           /**< Payload length in bytes. */
} pus_tm_packet_t;

#endif /* PUS_TYPES_H */
