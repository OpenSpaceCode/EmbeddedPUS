#ifndef PUS_TYPES_H
#define PUS_TYPES_H

#include <stdint.h>

/** @brief PUS service type identifier. */
typedef uint8_t pus_service_t;

/** @brief PUS service subtype identifier. */
typedef uint8_t pus_subtype_t;

/** @brief Encoded wire length of a TC secondary header in bytes. */
#define PUS_TC_SEC_HEADER_LEN 10u

/** @brief Encoded wire length of a TM secondary header in bytes. */
#define PUS_TM_SEC_HEADER_LEN 12u

/**
 * @brief Return codes for all public EmbeddedPUS functions.
 */
typedef enum
{
    PUS_STATUS_OK = 0,           /**< Operation succeeded. */
    PUS_STATUS_NULL,             /**< A required pointer argument was NULL. */
    PUS_STATUS_BAD_LENGTH,       /**< Buffer length is too short for decoding. */
    PUS_STATUS_BAD_VERSION,      /**< PUS version field is not 1. */
    PUS_STATUS_BAD_SERVICE,      /**< Service type is not recognised. */
    PUS_STATUS_NO_HANDLER,       /**< No handler registered for (service, subtype). */
    PUS_STATUS_HANDLER_FAILED,   /**< Registered handler returned an error. */
    PUS_STATUS_BUFFER_TOO_SMALL, /**< Output buffer capacity is insufficient. */
    PUS_STATUS_TABLE_FULL        /**< Registration table has no free entries. */
} pus_status_t;

/**
 * @brief Decoded TM secondary header fields.
 * @note Do not serialise this struct directly; use pus_tm_sec_header_encode().
 */
typedef struct
{
    uint8_t version;           /**< PUS version (4-bit field). */
    uint8_t time_ref_status;   /**< Time reference status (4-bit field). */
    uint8_t service_type_id;   /**< Service type identifier. */
    uint8_t subtype_id;        /**< Service subtype identifier. */
    uint16_t msg_type_counter; /**< Message type counter. */
    uint16_t destination_id;   /**< Destination application process ID. */
    uint32_t time;             /**< Packet timestamp (mission seconds or ticks). */
    uint8_t spare;             /**< Spare byte (shall be zero). */
} pus_tm_sec_header_t;

/**
 * @brief Decoded TC secondary header fields.
 * @note Do not serialise this struct directly; use pus_tc_sec_header_encode().
 */
typedef struct
{
    uint8_t version;         /**< PUS version (4-bit field). */
    uint8_t ack_flags;       /**< Acceptance/start/progress/completion bits. */
    uint8_t service_type_id; /**< Service type identifier. */
    uint8_t subtype_id;      /**< Service subtype identifier. */
    uint16_t source_id;      /**< Source application process ID. */
    uint32_t time;           /**< Packet timestamp. */
    uint8_t spare;           /**< Spare byte (shall be zero). */
} pus_tc_sec_header_t;

/**
 * @brief Decoded TC packet.
 * @note payload borrows from the caller's buffer; no data is copied.
 */
typedef struct
{
    pus_tc_sec_header_t sec_header; /**< Decoded secondary header. */
    const uint8_t *payload;         /**< Pointer into caller's buffer. */
    uint16_t payload_len;           /**< Payload length in bytes. */
} pus_tc_packet_t;

/**
 * @brief Decoded TM packet.
 * @note payload borrows from the caller's buffer; no data is copied.
 */
typedef struct
{
    pus_tm_sec_header_t sec_header; /**< Decoded secondary header. */
    const uint8_t *payload;         /**< Pointer into caller's buffer. */
    uint16_t payload_len;           /**< Payload length in bytes. */
} pus_tm_packet_t;

#endif /* PUS_TYPES_H */
