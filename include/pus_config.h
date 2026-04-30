#ifndef PUS_CONFIG_H
#define PUS_CONFIG_H

/**
 * @file pus_config.h
 * @brief Compile-time configuration defaults for EmbeddedPUS.
 *
 * Define these macros before including the public headers, or pass equivalent
 * -D options from the build system, to customize the library for a target.
 */

#ifndef PUS_VERSION
/** @brief PUS version value accepted by decoders and written by builders. */
#define PUS_VERSION 1u
#endif

#ifndef PUS_ENABLE_SPACE_PACKET_ADAPTER
/**
 * @brief Enable integration helpers for CCSDS SpacePacket when set nonzero.
 *
 * When enabled, pus.h includes the external space packet header and exposes the
 * adapter conversion functions.
 */
#define PUS_ENABLE_SPACE_PACKET_ADAPTER 0u
#endif

#ifndef PUS_MAX_TC_HANDLERS
/** @brief Maximum number of service/subtype handler entries per context. */
#define PUS_MAX_TC_HANDLERS 16u
#endif

#endif /* PUS_CONFIG_H */
