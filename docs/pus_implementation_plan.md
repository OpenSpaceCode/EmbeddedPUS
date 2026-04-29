# Minimal ECSS PUS Implementation Plan

## 1. Goal

Implement a minimal, deterministic ECSS Packet Utilisation Standard (PUS)
library in embedded C.

The module shall:

- operate on caller-owned byte buffers,
- avoid dynamic allocation,
- keep PUS core logic independent from CCSDS Space Packet code,
- support TC processing and TM generation,
- provide a minimal subset of PUS services listed in `docs/pus_services.md`,
- allow later integration with CCSDS Space Packet through a thin adapter layer.

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

Proposed encoded layout:

| Offset | Size | Field |
| --- | ---: | --- |
| 0 | 4 bits | PUS version |
| 0 | 4 bits | ACK flags |
| 1 | 1 byte | Service type ID |
| 2 | 1 byte | Subtype ID |
| 3 | 2 bytes | Source ID |
| 5 | 4 bytes | Time |
| 9 | 1 byte | Spare |

### 3.2 TM Secondary Header

Proposed encoded layout:

| Offset | Size | Field |
| --- | ---: | --- |
| 0 | 4 bits | PUS version |
| 0 | 4 bits | Time reference status |
| 1 | 1 byte | Service type ID |
| 2 | 1 byte | Subtype ID |
| 3 | 2 bytes | Message type counter |
| 5 | 2 bytes | Destination ID |
| 7 | 4 bytes | Time |
| 11 | 1 byte | Spare |

## 4. Proposed Code Structure

```text
include/
  pus.h                 public high-level API
  pus_types.h           core types, TC/TM headers, enums
  pus_config.h          compile-time limits and feature flags
  pus_codec.h           byte-level TC/TM header encode/decode
  pus_router.h          TC handler registry
  pus_tc.h              TC processing API
  pus_tm.h              TM generation API
  pus_services.h        service and subtype constants

src/
  pus.c                 context init and common logic
  pus_codec.c           explicit wire packing and unpacking
  pus_router.c          service/subtype dispatch
  pus_tc.c              TC validation and processing flow
  pus_tm.c              TM construction and counters
  pus_service_1.c       request verification
  pus_service_3.c       housekeeping
  pus_service_5.c       event reporting
  pus_service_17.c      test service
  pus_service_20.c      parameter management

optional later:
  pus_ccsds_adapter.c   bridge to EmbeddedSpacePacket / CCSDS
```

## 5. Core Types

Define explicit packet views for decoded TC and TM packets:

```c
typedef struct {
	pus_tc_sec_header_t sec_header;
	const uint8_t *payload;
	uint16_t payload_len;
} pus_tc_packet_t;

typedef struct {
	pus_tm_sec_header_t sec_header;
	const uint8_t *payload;
	uint16_t payload_len;
} pus_tm_packet_t;
```

Define a common status enum:

```c
typedef enum {
	PUS_STATUS_OK = 0,
	PUS_STATUS_NULL,
	PUS_STATUS_BAD_LENGTH,
	PUS_STATUS_BAD_VERSION,
	PUS_STATUS_BAD_SERVICE,
	PUS_STATUS_NO_HANDLER,
	PUS_STATUS_HANDLER_FAILED,
	PUS_STATUS_BUFFER_TOO_SMALL
} pus_status_t;
```

Define callback types:

```c
typedef struct pus_context pus_context_t;

typedef pus_status_t (*pus_tc_handler_t)(
	pus_context_t *ctx,
	const pus_tc_packet_t *tc,
	void *user
);

typedef pus_status_t (*pus_tm_sink_t)(
	void *user,
	const uint8_t *data,
	uint16_t len
);
```

## 6. Configuration

Extend `include/pus_config.h` with bounded compile-time limits:

```c
#define PUS_VERSION 1u
#define PUS_MAX_TC_HANDLERS 16u
#define PUS_MAX_TM_PAYLOAD_LEN 256u
#define PUS_MAX_TC_PAYLOAD_LEN 256u
#define PUS_ENABLE_CRC 0u
```

The implementation shall not allocate memory dynamically. Any larger mission
configuration shall be done by changing compile-time limits.

## 7. Public API

### 7.1 Lifecycle

```c
pus_status_t pus_init(pus_context_t *ctx, const pus_config_t *config);
```

The context shall contain:

- registered TC handlers,
- TM message counter,
- default source/destination identifiers,
- TM sink callback,
- user pointer for application context.

### 7.2 TC Decode

```c
pus_status_t pus_tc_decode(
	const uint8_t *data,
	uint16_t len,
	pus_tc_packet_t *tc
);
```

Responsibilities:

- validate pointers,
- check minimum TC secondary header length,
- decode the TC secondary header,
- validate PUS version,
- expose payload pointer and payload length.

### 7.3 TM Build

```c
pus_status_t pus_tm_build(
	pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype,
	uint16_t destination_id,
	const uint8_t *payload,
	uint16_t payload_len,
	uint8_t *out,
	uint16_t out_capacity,
	uint16_t *out_len
);
```

Responsibilities:

- validate arguments,
- fill a TM secondary header,
- increment message counter,
- encode the TM secondary header,
- append payload,
- optionally append CRC in a later phase.

### 7.4 TC Processing

```c
pus_status_t pus_tc_process(
	pus_context_t *ctx,
	const uint8_t *data,
	uint16_t len
);
```

Responsibilities:

- decode the TC,
- validate service/subtype,
- emit Service 1 verification telemetry based on ACK flags,
- route the TC to the registered handler,
- report routing or execution failures.

### 7.5 TC Router

```c
pus_status_t pus_router_register(
	pus_context_t *ctx,
	pus_service_t service,
	pus_subtype_t subtype,
	pus_tc_handler_t handler,
	void *user
);
```

The router shall use a fixed-size handler table configured by
`PUS_MAX_TC_HANDLERS`.

## 8. TC Processing Logic

1. Decode TC secondary header with `pus_tc_decode()`.
2. Validate:
   - buffer length,
   - PUS version,
   - service type,
   - subtype,
   - payload length.
3. If requested by ACK flags, emit:
   - `TM[1,1]` on successful acceptance,
   - `TM[1,2]` on failed acceptance.
4. Route by `(service_type_id, subtype_id)`.
5. If no handler exists, emit `TM[1,10]` failed routing verification.
6. If a handler exists:
   - emit `TM[1,3]` start success if requested,
   - call the handler,
   - emit `TM[1,7]` completion success on success,
   - emit `TM[1,8]` completion failure on failure.

## 9. TM Generation Logic

1. Application or PUS service requests a TM packet.
2. `pus_tm_build()` validates the output buffer.
3. The function fills a `pus_tm_sec_header_t`.
4. The TM codec encodes the secondary header explicitly.
5. Payload is copied after the encoded TM secondary header.
6. The message type counter is incremented.
7. The resulting byte buffer is returned to the caller or passed to the TM sink.

## 10. Service Implementation Plan

### 10.1 Service 1 - Request Verification

Implement helper functions for:

- `TM[1,1]` successful acceptance verification report,
- `TM[1,2]` failed acceptance verification report,
- `TM[1,3]` successful start of execution verification report,
- `TM[1,4]` failed start of execution verification report,
- `TM[1,5]` successful progress of execution verification report,
- `TM[1,6]` failed progress of execution verification report,
- `TM[1,7]` successful completion of execution verification report,
- `TM[1,8]` failed completion of execution verification report,
- `TM[1,10]` failed routing verification report.

Minimal payload shall identify the related TC and include a failure code when
applicable.

### 10.2 Service 17 - Test

Implement this first as the minimal end-to-end service:

- `TC[17,1]` perform are-you-alive connection test,
- `TM[17,2]` are-you-alive connection test report,
- `TC[17,3]` perform on-board connection test,
- `TM[17,4]` on-board connection test report.

This service is useful for validating TC decode, routing, TM generation, and
Service 1 verification.

### 10.3 Service 5 - Event Reporting

Provide application APIs for:

- `TM[5,1]` informative event report,
- `TM[5,2]` low severity anomaly report,
- `TM[5,3]` medium severity anomaly report,
- `TM[5,4]` high severity anomaly report.

Use a compact event payload:

- event ID,
- optional parameter bytes.

### 10.4 Service 3 - Housekeeping

Support:

- `TM[3,25]` housekeeping parameter report,
- `TM[3,26]` diagnostic parameter report,
- `TC[3,27]` one-shot housekeeping report request,
- `TC[3,28]` one-shot diagnostic report request.

Use registered provider callbacks so the PUS core does not own application data.

### 10.5 Service 20 - On-board Parameter Management

Support:

- `TC[20,1]` report parameter values,
- `TM[20,2]` parameter value report,
- `TC[20,3]` set parameter values.

Use a fixed parameter table supplied by the application.

## 11. CCSDS Space Packet Integration

The PUS core shall not require `sp_packet_t`.

Add CCSDS integration later as an adapter:

```c
pus_status_t pus_ccsds_process_space_packet(
	pus_context_t *ctx,
	const sp_packet_t *packet
);

pus_status_t pus_ccsds_build_tm_packet(
	pus_context_t *ctx,
	const pus_tm_packet_t *tm,
	sp_packet_t *packet
);
```

The adapter shall:

- validate CCSDS packet type and secondary header flag,
- pass the secondary header and payload bytes into the PUS core,
- translate CCSDS identifiers into a compact TC reference for Service 1 reports,
- build CCSDS TM packets from already encoded PUS TM data.

## 12. Implementation Phases

### Phase 1 - Type Cleanup

- Rename `serivce_type_id` to `service_type_id`.
- Add service and subtype constants in `pus_services.h`.
- Add fixed encoded secondary header length macros.
- Add `pus_status_t`, `pus_tc_packet_t`, and `pus_tm_packet_t`.

### Phase 2 - Codec

- Implement `pus_tc_sec_header_decode()`.
- Implement `pus_tm_sec_header_encode()`.
- Implement optional complementary helpers:
  - `pus_tc_sec_header_encode()`,
  - `pus_tm_sec_header_decode()`.
- Add unit tests for TC and TM header lengths and field decoding.

### Phase 3 - Context and Router

- Implement `pus_context_t`.
- Implement `pus_init()`.
- Implement fixed-size route table.
- Implement `pus_router_register()`.
- Implement internal route lookup.

### Phase 4 - TC Processing

- Implement `pus_tc_decode()`.
- Implement `pus_tc_process()`.
- Add acceptance, routing, start, and completion flow.
- Add tests for invalid version, unknown service, no handler, and handler failure.

### Phase 5 - TM Generation

- Implement `pus_tm_build()`.
- Add TM message counter handling.
- Add TM sink callback support.
- Add tests for buffer-too-small and expected encoded TM bytes.

### Phase 6 - Service 1

- Implement request verification report builders.
- Connect ACK flags to verification generation.
- Add tests for each ACK-controlled report path.

### Phase 7 - Service 17

- Register default Service 17 handlers.
- Implement `TC[17,1]` to `TM[17,2]`.
- Implement `TC[17,3]` to `TM[17,4]`.
- Add an end-to-end test.

### Phase 8 - Service 5

- Implement event report APIs.
- Add tests for each severity subtype.

### Phase 9 - Service 3

- Add housekeeping provider callback registration.
- Implement one-shot housekeeping and diagnostic reports.
- Add tests using fake provider callbacks.

### Phase 10 - Service 20

- Add parameter table configuration.
- Implement report parameter values.
- Implement set parameter values.
- Add tests for valid set, invalid parameter ID, and readback.

### Phase 11 - Optional CRC

- Add compile-time CRC enable flag.
- Implement CRC append/check helpers.
- Add tests with known vectors.

### Phase 12 - CCSDS Adapter

- Move direct `sp_packet_t` dependency out of the core.
- Implement adapter functions.
- Keep existing `pus_from_space_packet()` and `pus_to_space_packet()` only as
  compatibility wrappers if needed.

## 13. Testing Plan

Add unit tests for:

- TC secondary header decode,
- TM secondary header encode,
- TC and TM header length differences,
- invalid pointers,
- invalid PUS version,
- buffer too small,
- route registration,
- route dispatch,
- no-handler behavior,
- Service 1 verification reports,
- Service 17 ping flow,
- Service 5 event reports,
- Service 3 housekeeping callback flow,
- Service 20 parameter get/set flow.

Tests shall avoid dynamic allocation and shall use deterministic byte arrays.

## 14. Early Risks

- C bitfield layout is compiler-dependent. Use bitfields only as logical fields,
  not as serialized wire representation.
- The current `src/pus.c` depends directly on `sp_packet_t`; this should become
  an adapter dependency rather than a core dependency.
- The field name `serivce_type_id` is misspelled and should be corrected before
  expanding the API.
- TC and TM secondary headers are different and must not be handled through one
  generic serialized struct.

## 15. First Useful Milestone

The first functional milestone should be:

1. Decode a raw TC byte buffer.
2. Route `TC[17,1]`.
3. Generate `TM[1,1]` acceptance success when requested.
4. Generate `TM[17,2]` are-you-alive report.
5. Generate `TM[1,7]` completion success when requested.

This proves the minimal TC decode, verification, routing, and TM generation
pipeline without requiring CCSDS integration.
