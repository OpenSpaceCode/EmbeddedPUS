#ifndef PUS_SERVICE_3_H
#define PUS_SERVICE_3_H

#include <stdint.h>
#include "pus_config.h"
#include "pus_context.h"
#include "pus_types.h"

#define PUS_SERVICE_3_KIND_HK   0u
#define PUS_SERVICE_3_KIND_DIAG 1u

/**
 * Provider callback: write parameter data for the given SID into buf.
 * Called by emit functions; must not include the SID itself.
 */
typedef pus_status_t (*pus_hk_provider_t)(
	uint16_t  sid,
	uint8_t  *buf,
	uint16_t  capacity,
	uint16_t *out_len,
	void     *user_data);

typedef struct {
	uint16_t          sid;
	uint8_t           kind;
	pus_hk_provider_t provider;
	void             *user_data;
	uint8_t           is_used;
} pus_hk_entry_t;

typedef struct {
	pus_hk_entry_t structures[PUS_SERVICE_3_MAX_STRUCTURES];
} pus_service_3_ctx_t;

pus_status_t pus_service_3_init(pus_service_3_ctx_t *s3);

/** Register a housekeeping structure provider for the given SID. */
pus_status_t pus_service_3_register_hk(
	pus_service_3_ctx_t *s3,
	uint16_t             sid,
	pus_hk_provider_t    provider,
	void                *user_data);

/** Register a diagnostic structure provider for the given SID. */
pus_status_t pus_service_3_register_diag(
	pus_service_3_ctx_t *s3,
	uint16_t             sid,
	pus_hk_provider_t    provider,
	void                *user_data);

/** Build and emit TM[3,25] for a registered HK structure SID. */
pus_status_t pus_service_3_emit_hk(
	pus_context_t       *ctx,
	pus_service_3_ctx_t *s3,
	uint16_t             sid);

/** Build and emit TM[3,26] for a registered diagnostic structure SID. */
pus_status_t pus_service_3_emit_diag(
	pus_context_t       *ctx,
	pus_service_3_ctx_t *s3,
	uint16_t             sid);

#endif /* PUS_SERVICE_3_H */
