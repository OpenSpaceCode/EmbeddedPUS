#ifndef PUS_CONFIG_H
#define PUS_CONFIG_H

#ifndef PUS_VERSION
/** @brief PUS version value written into every outgoing TM packet. */
#define PUS_VERSION 1u
#endif

#ifndef PUS_MAX_TC_HANDLERS
/** @brief Maximum registered TC handler entries per context. */
#define PUS_MAX_TC_HANDLERS 16u
#endif

#ifndef PUS_MAX_TM_PAYLOAD_LEN
/** @brief Maximum TM application payload length in bytes. */
#define PUS_MAX_TM_PAYLOAD_LEN 256u
#endif

#ifndef PUS_MAX_TC_PAYLOAD_LEN
/** @brief Maximum TC application payload length in bytes. */
#define PUS_MAX_TC_PAYLOAD_LEN 256u
#endif

#ifndef PUS_SERVICE_3_MAX_STRUCTURES
/** @brief Maximum number of HK+diagnostic structures per Service 3 context. */
#define PUS_SERVICE_3_MAX_STRUCTURES 8u
#endif

#ifndef PUS_SERVICE_20_MAX_PARAMS
/** @brief Maximum number of registered parameters per Service 20 context. */
#define PUS_SERVICE_20_MAX_PARAMS 16u
#endif

#endif /* PUS_CONFIG_H */
