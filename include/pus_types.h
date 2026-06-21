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

/** @brief Return codes for all public EmbeddedPUS functions. */
typedef enum {
	PUS_STATUS_OK = 0,
	PUS_STATUS_NULL,
	PUS_STATUS_BAD_LENGTH,
	PUS_STATUS_BAD_VERSION,
	PUS_STATUS_BAD_SERVICE,
	PUS_STATUS_NO_HANDLER,
	PUS_STATUS_HANDLER_FAILED,
	PUS_STATUS_BUFFER_TOO_SMALL,
	PUS_STATUS_TABLE_FULL
} pus_status_t;

/**
 * Decoded TM secondary header. Never serialise this struct directly;
 * use pus_tm_sec_header_encode() for wire encoding.
 */
typedef struct {
	uint8_t  version;          /* PUS version (4-bit field) */
	uint8_t  time_ref_status;  /* time reference status (4-bit field) */
	uint8_t  service_type_id;
	uint8_t  subtype_id;
	uint16_t msg_type_counter;
	uint16_t destination_id;
	uint32_t time;
	uint8_t  spare;
} pus_tm_sec_header_t;

/**
 * Decoded TC secondary header. Never serialise this struct directly;
 * use pus_tc_sec_header_encode() for wire encoding.
 */
typedef struct {
	uint8_t  version;         /* PUS version (4-bit field) */
	uint8_t  ack_flags;       /* acceptance/start/progress/completion bits */
	uint8_t  service_type_id;
	uint8_t  subtype_id;
	uint16_t source_id;
	uint32_t time;
	uint8_t  spare;
} pus_tc_sec_header_t;

/** @brief Decoded TC packet view. Payload borrows from the caller's buffer. */
typedef struct {
	pus_tc_sec_header_t  sec_header;
	const uint8_t       *payload;
	uint16_t             payload_len;
} pus_tc_packet_t;

/** @brief Decoded TM packet view. Payload borrows from the caller's buffer. */
typedef struct {
	pus_tm_sec_header_t  sec_header;
	const uint8_t       *payload;
	uint16_t             payload_len;
} pus_tm_packet_t;

#endif /* PUS_TYPES_H */
