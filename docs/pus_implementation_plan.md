# Minimal ECSS PUS Implementation Plan

## 1. Goal

Implement a minimal, deterministic ECSS Packet Utilisation Standard (PUS)
library in embedded C.

The module shall:

- operate on caller-owned byte buffers,
- use no dynamic allocation,
- keep PUS core logic independent from CCSDS Space Packet code,
- support TC processing and TM generation,
- provide a minimal subset of PUS services listed in `docs/pus_services.md`.

The implementation shall follow the project guidance from
`.github/instructions/instruction-embedded-min.md.instructions.md`.

## 2. Design Principles

- Keep the PUS core independent from transport and packetization layers.
- Use explicit byte encoding and decoding for all wire formats.
- Do not serialize bitfield structs with `memcpy()` or `sizeof()` for protocol
  wire data.
- Keep all memory caller-owned or statically bounded by configuration macros.
- Keep APIs small and predictable.
- Use service/subtype routing for TC dispatch.
- Emit verification telemetry through Service 1 where applicable.
- Give each service with runtime state its own lightweight context struct;
  do not bloat `pus_context_t` with per-service fields.

## 3. Important TC/TM Header Distinction

PUS TC and PUS TM packets use different secondary headers. The implementation
shall keep them separate in both the API and codec logic.

Current logical types are defined in `include/pus_types.h`:

- `pus_tc_sec_header_t`
- `pus_tm_sec_header_t`

The wire codec shall use fixed encoded lengths:

```c
#define PUS_TC_SEC_HEADER_LEN 10u
#define PUS_TM_SEC_HEADER_LEN 12u
```

### 3.1 TC Secondary Header

Encoded layout:

| Offset | Size    | Field          |
| ------ | ------: | -------------- |
| 0      | 4 bits  | PUS version    |
| 0      | 4 bits  | ACK flags      |
| 1      | 1 byte  | Service type   |
| 2      | 1 byte  | Subtype        |
| 3      | 2 bytes | Source ID      |
| 5      | 4 bytes | Time           |
| 9      | 1 byte  | Spare          |

### 3.2 TM Secondary Header

Encoded layout:

| Offset | Size    | Field                |
| ------ | ------: | -------------------- |
| 0      | 4 bits  | PUS version          |
| 0      | 4 bits  | Time reference status|
| 1      | 1 byte  | Service type         |
| 2      | 1 byte  | Subtype              |
| 3      | 2 bytes | Message type counter |
| 5      | 2 bytes | Destination ID       |
| 7      | 4 bytes | Time                 |
| 11     | 1 byte  | Spare                |

## 4. Code Structure

```text
include/
  pus.h                 umbrella public header (TC/TM high-level API)
  pus_types.h           core types, TC/TM header structs, status enum
  pus_config.h          compile-time limits and feature flags
  pus_codec.h           byte-level TC/TM secondary header encode/decode
  pus_context.h         runtime context, callbacks, lifecycle API
  pus_handler.h         TC handler registry API
  pus_services.h        service and subtype numeric constants
  pus_service_1.h       request verification build/emit helpers
  pus_service_3.h       housekeeping service context and API
  pus_service_5.h       event reporting emit helpers
  pus_service_17.h      test service handler registration
  pus_service_20.h      parameter management service context and API

src/
  pus.c                 TC decode/process, TM build, TM sink
  pus_codec.c           explicit wire packing and unpacking
  pus_context.c         context initialisation
  pus_handler.c         service/subtype dispatch table
  pus_service_1.c       request verification report builders and emitters
  pus_service_3.c       housekeeping TC handlers and TM builders
  pus_service_5.c       event report TM builders
  pus_service_17.c      are-you-alive and on-board connection TC handlers
  pus_service_20.c      parameter get/set TC handlers

optional later:
  pus_ccsds_adapter.c   bridge to EmbeddedSpacePacket / CCSDS
```

## 5. Core Types

### 5.1 Packet Structs

Define explicit decoded header views. Do **not** use bitfield annotations on
fields — the codec functions are the single source of truth for bit widths.

```c
typedef struct {
    uint8_t  version;           /* PUS version (4 bits used) */
    uint8_t  time_ref_status;   /* time reference status (4 bits used) */
    uint8_t  service_type_id;
    uint8_t  subtype_id;
    uint16_t msg_type_counter;
    uint16_t destination_id;
    uint32_t time;
    uint8_t  spare;
} pus_tm_sec_header_t;

typedef struct {
    uint8_t  version;           /* PUS version (4 bits used) */
    uint8_t  ack_flags;         /* acceptance/start/progress/completion (4 bits used) */
    uint8_t  service_type_id;
    uint8_t  subtype_id;
    uint16_t source_id;
    uint32_t time;
    uint8_t  spare;
} pus_tc_sec_header_t;

typedef struct {
    pus_tc_sec_header_t  sec_header;
    const uint8_t       *payload;
    uint16_t             payload_len;
} pus_tc_packet_t;

typedef struct {
    pus_tm_sec_header_t  sec_header;
    const uint8_t       *payload;
    uint16_t             payload_len;
} pus_tm_packet_t;
```

### 5.2 Status Enum

```c
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
```

### 5.3 Callback Types

```c
typedef struct pus_context pus_context_t;

typedef pus_status_t (*pus_tc_handler_t)(
    pus_context_t       *ctx,
    const pus_tc_packet_t *tc,
    void                *user_data
);

typedef pus_status_t (*pus_tm_sink_t)(
    void          *user_data,
    const uint8_t *data,
    uint16_t       len
);

/* Optional application-supplied time source. Returns seconds or mission ticks. */
typedef uint32_t (*pus_time_source_t)(void *user_data);
```

### 5.4 Service-Specific Context Types

Services with runtime state use their own lightweight context structs, keeping
`pus_context_t` minimal.

```c
/* Service 3 */
typedef pus_status_t (*pus_hk_provider_t)(
    uint8_t   struct_id,
    uint8_t  *out,
    uint16_t  capacity,
    uint16_t *out_len,
    void     *user_data
);

typedef struct {
    pus_hk_provider_t hk_provider;
    pus_hk_provider_t diag_provider;
    void             *user_data;
} pus_service_3_ctx_t;

/* Service 20 */
typedef struct {
    uint16_t id;
    void    *value;
    uint8_t  size;   /* value byte width: 1, 2, or 4 */
    uint8_t  flags;  /* PUS_PARAM_FLAG_READ_ONLY */
} pus_param_entry_t;

typedef struct {
    const pus_param_entry_t *params;
    uint16_t                 count;
} pus_service_20_ctx_t;
```

## 6. Configuration

```c
#define PUS_VERSION              1u
#define PUS_MAX_TC_HANDLERS     16u
#define PUS_MAX_TM_PAYLOAD_LEN 256u
#define PUS_MAX_TC_PAYLOAD_LEN 256u
```

All limits are overridable with compiler `-D` flags. No dynamic allocation.

## 7. Public API

### 7.1 Lifecycle

```c
pus_status_t pus_init(pus_context_t *ctx);
pus_status_t pus_init_with_config(pus_context_t *ctx, const pus_config_t *config);
pus_status_t pus_set_tm_sink(pus_context_t *ctx, pus_tm_sink_t sink, void *user_data);
```

The context contains:
- registered TC handler table,
- TM message counter,
- default source/destination identifiers,
- optional TM sink callback,
- optional time source callback.

### 7.2 TC Decode

```c
pus_status_t pus_tc_decode(
    const uint8_t   *data,
    uint16_t         len,
    pus_tc_packet_t *tc
);
```

### 7.3 TM Build

```c
pus_status_t pus_tm_build(
    pus_context_t *ctx,
    pus_service_t  service,
    pus_subtype_t  subtype,
    uint16_t       destination_id,
    const uint8_t *payload,
    uint16_t       payload_len,
    uint8_t       *out,
    uint16_t       out_capacity,
    uint16_t      *out_len
);
```

When a `time_source` is set in the context, `pus_tm_build` calls it to obtain
the timestamp for each outgoing TM packet.

### 7.4 TC Processing

```c
pus_status_t pus_tc_process(
    pus_context_t *ctx,
    const uint8_t *data,
    uint16_t       len
);
```

### 7.5 TC Router

```c
pus_status_t pus_handler_register(
    pus_context_t    *ctx,
    pus_service_t     service,
    pus_subtype_t     subtype,
    pus_tc_handler_t  handler,
    void             *user_data
);
```

## 8. TC Processing Logic

1. Decode TC secondary header with `pus_tc_decode()`.
2. On decode failure, return the error status. Service 1 reports cannot be
   emitted because ACK flags are unknown.
3. If ACK acceptance bit is set, emit `TM[1,1]` acceptance success.
4. Look up handler by `(service_type_id, subtype_id)`.
5. If no handler found, always emit `TM[1,10]` routing failure and return
   `PUS_STATUS_NO_HANDLER`.
6. If ACK start bit is set, emit `TM[1,3]` start success.
7. Call the registered handler.
8. On handler success and ACK completion bit set: emit `TM[1,7]`.
9. On handler failure and ACK completion bit set: emit `TM[1,8]`.

Note: failure reports (`TM[1,2]`, `TM[1,4]`, `TM[1,6]`, `TM[1,8]`) are also
available as standalone builder/emit functions for handlers that want to report
finer-grained failures.

## 9. TM Generation Logic

1. `pus_tm_build()` validates output buffer and arguments.
2. Fills `pus_tm_sec_header_t` using context counter and time source.
3. Encodes the secondary header explicitly byte-by-byte.
4. Appends payload bytes.
5. Increments the message counter.
6. If a TM sink is set, invokes it with the encoded packet.

## 10. Service Implementation Plan

### 10.1 Service 1 — Request Verification

**Simplified Request ID:** ECSS-E-ST-70-41C defines the request ID as the
CCSDS primary header identification fields (APID + sequence count). Since this
library operates below the CCSDS layer, the request ID is simplified to:
`[service_type_id (1), subtype_id (1), source_id (2)]` = 4 bytes.

Payload formats:
- Success reports: `[service(1), subtype(1), source_id(2)]` = 4 bytes
- Failure reports: `[service(1), subtype(1), source_id(2), failure_code(2)]` = 6 bytes

Reports implemented:
- `TM[1,1]` acceptance success (ACK-conditioned)
- `TM[1,2]` acceptance failure (ACK-conditioned)
- `TM[1,3]` start success (ACK-conditioned)
- `TM[1,4]` start failure (ACK-conditioned)
- `TM[1,5]` progress success (ACK-conditioned)
- `TM[1,6]` progress failure (ACK-conditioned)
- `TM[1,7]` completion success (ACK-conditioned)
- `TM[1,8]` completion failure (ACK-conditioned)
- `TM[1,10]` routing failure (always emitted)

Each report has a `build` variant (writes into caller buffer) and an `emit`
variant (writes via context TM sink).

### 10.2 Service 17 — Test

Registration:
```c
pus_status_t pus_service_17_register_handlers(pus_context_t *ctx);
```

Handlers:
- `TC[17,1]` → emit `TM[17,2]` (no payload)
- `TC[17,3]` → emit `TM[17,4]` (echo TC payload as destination ID)

### 10.3 Service 5 — Event Reporting

Pure TM-only service. Application calls:
```c
pus_status_t pus_service_5_emit_info(pus_context_t *ctx, uint16_t event_id,
    const uint8_t *data, uint16_t data_len);
pus_status_t pus_service_5_emit_low(pus_context_t *ctx, uint16_t event_id,
    const uint8_t *data, uint16_t data_len);
pus_status_t pus_service_5_emit_medium(pus_context_t *ctx, uint16_t event_id,
    const uint8_t *data, uint16_t data_len);
pus_status_t pus_service_5_emit_high(pus_context_t *ctx, uint16_t event_id,
    const uint8_t *data, uint16_t data_len);
```

TM payload: `[event_id(2), optional_data...]`

### 10.4 Service 3 — Housekeeping

Service 3 has its own `pus_service_3_ctx_t` holding provider callbacks.

```c
pus_status_t pus_service_3_init(pus_service_3_ctx_t *svc,
    pus_hk_provider_t hk_provider,
    pus_hk_provider_t diag_provider,
    void *user_data);
pus_status_t pus_service_3_register_handlers(pus_context_t *ctx,
    pus_service_3_ctx_t *svc);
```

TC payload format for `TC[3,27]` and `TC[3,28]`:
`[struct_id (2 bytes)]`

TM payload format for `TM[3,25]` and `TM[3,26]`:
`[struct_id (2 bytes), ...provider-supplied parameter bytes...]`

### 10.5 Service 20 — Parameter Management

Service 20 has its own `pus_service_20_ctx_t` holding the application parameter
table.

```c
pus_status_t pus_service_20_init(pus_service_20_ctx_t *svc,
    const pus_param_entry_t *params, uint16_t count);
pus_status_t pus_service_20_register_handlers(pus_context_t *ctx,
    pus_service_20_ctx_t *svc);
```

TC[20,1] payload: `[param_id (2 bytes), ...]` (list of parameter IDs to report)
TM[20,2] payload: `[param_id (2 bytes), value (param.size bytes), ...]`
TC[20,3] payload: `[param_id (2 bytes), value (param.size bytes), ...]`

Parameter flags:
- `PUS_PARAM_FLAG_READ_ONLY 0x01` — set requests ignored, value not written


## 14. Language and Constraints

- **Language:** C99
- **Memory:** no dynamic allocation; all buffers are caller-owned or statically
  bounded by `pus_config.h` macros
- **Determinism:** no blocking calls; all functions return synchronously
- **Portability:** no target-specific headers; big-endian encoding handled
  explicitly in the codec
