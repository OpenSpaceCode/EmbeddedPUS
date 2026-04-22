#ifndef PUS_TYPES_H
#define PUS_TYPES_H

#include <stdint.h>

typedef uint8_t pus_service_t;
typedef uint8_t pus_subtype_t;


typedef struct
{
    uint8_t version : 4;
    uint8_t time_ref_status : 4;
    uint8_t serivce_type_id : 8;
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
    uint8_t serivce_type_id : 8;
    uint8_t subtype_id : 8;
    uint16_t source_id : 16;
    uint32_t time;
    uint8_t spare;
} pus_tc_sec_header_t;

#endif /* PUS_TYPES_H */