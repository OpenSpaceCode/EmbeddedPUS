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

#endif /* PUS_CONFIG_H */
