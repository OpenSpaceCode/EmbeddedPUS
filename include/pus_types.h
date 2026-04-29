#ifndef PUS_TYPES_H
#define PUS_TYPES_H

#include <stdint.h>

typedef uint8_t pus_service_t;
typedef uint8_t pus_subtype_t;

#define PUS_TC_SEC_HEADER_LEN 10u
#define PUS_TM_SEC_HEADER_LEN 12u

typedef enum
{
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

typedef struct
{
    uint8_t version : 4;
    uint8_t time_ref_status : 4;
    uint8_t service_type_id : 8;
    uint8_t subtype_id : 8;
    uint16_t msg_type_counter : 16;
    uint16_t destination_id : 16;
    uint32_t time;
    uint8_t spare;
} pus_tm_sec_header_t;

typedef struct
{
    uint8_t version : 4;
    uint8_t ack_flags : 4;
    uint8_t service_type_id : 8;
    uint8_t subtype_id : 8;
    uint16_t source_id : 16;
    uint32_t time;
    uint8_t spare;
} pus_tc_sec_header_t;

typedef struct
{
    pus_tc_sec_header_t sec_header;
    const uint8_t *payload;
    uint16_t payload_len;
} pus_tc_packet_t;

typedef struct
{
    pus_tm_sec_header_t sec_header;
    const uint8_t *payload;
    uint16_t payload_len;
} pus_tm_packet_t;

#endif /* PUS_TYPES_H */
