#ifndef PUS_CONFIG_H
#define PUS_CONFIG_H

/** @brief PUS version written into every outgoing TM packet. */
#ifndef PUS_VERSION
#define PUS_VERSION 1u
#endif

/** @brief Maximum TC handler entries per context. Override with -DPUS_MAX_TC_HANDLERS=N. */
#ifndef PUS_MAX_TC_HANDLERS
#define PUS_MAX_TC_HANDLERS 16u
#endif

/** @brief Maximum TM application payload length in bytes. Override with -DPUS_MAX_TM_PAYLOAD_LEN=N. */
#ifndef PUS_MAX_TM_PAYLOAD_LEN
#define PUS_MAX_TM_PAYLOAD_LEN 256u
#endif

/** @brief Maximum TC application payload length in bytes. Override with -DPUS_MAX_TC_PAYLOAD_LEN=N. */
#ifndef PUS_MAX_TC_PAYLOAD_LEN
#define PUS_MAX_TC_PAYLOAD_LEN 256u
#endif

/** @brief Maximum HK/diagnostic structures per ST[03] context. Override with -DPUS_SERVICE_3_MAX_STRUCTURES=N. */
#ifndef PUS_SERVICE_3_MAX_STRUCTURES
#define PUS_SERVICE_3_MAX_STRUCTURES 8u
#endif

/** @brief Maximum registered parameters per ST[20] context. Override with -DPUS_SERVICE_20_MAX_PARAMS=N. */
#ifndef PUS_SERVICE_20_MAX_PARAMS
#define PUS_SERVICE_20_MAX_PARAMS 16u
#endif

#endif /* PUS_CONFIG_H */
