# EmbeddedPUS
Minimal ECSS Packet Utilisation Standard (PUS) implementation in C, designed for small-scale academic space missions.

## Standards Compliance

- **ECSS-E-ST-70-41C**: Packet Utilisation Standard (PUS)

## Features

### Implemented Services

| Service | Name | Subtypes |
|---------|------|----------|
| ST[01] | Request Verification | Acceptance, Start, Completion (success & failure), Routing failure; Progress reports must be emitted manually by the handler |
| ST[03] | Housekeeping | TM[3,25] HK report, TM[3,26] diagnostic report |
| ST[05] | Event Reporting | Info, Low/Medium/High severity |
| ST[17] | Test | TC[17,1] are-you-alive, TC[17,3] on-board connection test |
| ST[20] | Parameter Management | TC[20,1] request report, TC[20,3] set values, TM[20,2] value report |

### Design Principles

- **No heap** — all memory is caller-supplied or statically declared
- **No global state** — everything lives in a `pus_context_t` you own
- **Composable** — each service has its own context struct; mix and match what you need
- **Portable C** — no platform-specific dependencies

## Project Structure

```
EmbeddedPUS/
├── include/             # Public API headers
├── src/                 # Library source files
├── example/
│   └── obc_example.c   # CubeSat OBC usage example
├── tests/
│   ├── cunit.h          # Minimal test framework
│   └── unit_tests.c     # Test entry point
├── tools/
│   └── coverage_html.sh # Coverage report generator
├── build/               # Build artifacts (generated)
├── Makefile
└── README.md
```

## Building

```bash
make          # build and run tests
make example  # build and run the OBC example
make clean    # remove build artifacts
```

### Coverage Report

Requires `gcovr`:

```bash
sudo apt install gcovr
make coverage-html
# Output: build/coverage/index.html
```

## Quick Start

```c
#include "pus.h"
#include "pus_service_17.h"
#include "pus_service_3.h"
#include "pus_service_5.h"
#include "pus_service_20.h"

/* 1. Allocate contexts statically */
static pus_context_t       g_pus;
static pus_service_3_ctx_t g_s3;

/* 2. TM sink — called for every outgoing packet */
static pus_status_t uart_send(void *ud, const uint8_t *data, uint16_t len)
{
    (void)ud;
    /* write data[0..len] to your UART / radio */
    return PUS_STATUS_OK;
}

/* 3. HK provider — fill the housekeeping buffer on demand */
static pus_status_t hk_provider(uint16_t sid, uint8_t *buf,
                                  uint16_t cap, uint16_t *out_len, void *ud)
{
    (void)sid; (void)cap; (void)ud;
    buf[0] = read_temperature();
    *out_len = 1u;
    return PUS_STATUS_OK;
}

void app_init(void)
{
    /* 4. Initialise */
    pus_config_t cfg = {0};
    cfg.default_source_id = 0x0001;
    cfg.tm_sink           = uart_send;
    pus_init_with_config(&g_pus, &cfg);

    /* 5. Register service handlers */
    pus_service_17_register_handlers(&g_pus);   /* auto-respond to ping */

    pus_service_3_init(&g_s3);
    pus_service_3_register_hk(&g_s3, 0x0001, hk_provider, NULL);
}

void app_on_tc_received(const uint8_t *raw, uint16_t len)
{
    /* 6. Decode, route, verify — all in one call */
    pus_tc_process(&g_pus, raw, len);
}

void app_periodic(void)
{
    /* 7. Emit periodic housekeeping */
    pus_service_3_emit_hk(&g_pus, &g_s3, 0x0001);

    /* 8. Emit an event */
    pus_service_5_emit(&g_pus, PUS_SUBTYPE_EVENT_INFO, 0x0101, NULL, 0);
}
```

See [example/obc_example.c](example/obc_example.c) for a more complete scenario covering all five services.

## API Reference

### Lifecycle

```c
pus_status_t pus_init(pus_context_t *ctx);
pus_status_t pus_init_with_config(pus_context_t *ctx, const pus_config_t *config);
pus_status_t pus_set_tm_sink(pus_context_t *ctx, pus_tm_sink_t sink, void *user_data);
```

### TC / TM

```c
/* Decode a raw TC buffer into a packet view (zero-copy) */
pus_status_t pus_tc_decode(const uint8_t *data, uint16_t len, pus_tc_packet_t *tc);

/* Decode, route, and process a TC; emits ST[01] verification automatically */
pus_status_t pus_tc_process(pus_context_t *ctx, const uint8_t *data, uint16_t len);

/* Build and emit a TM packet; calls the configured TM sink */
pus_status_t pus_tm_build(pus_context_t *ctx, pus_service_t service,
                           pus_subtype_t subtype, uint16_t destination_id,
                           const uint8_t *payload, uint16_t payload_len,
                           uint8_t *out, uint16_t out_capacity, uint16_t *out_len);
```

### Handler Registration

```c
/* Register a TC handler callback for (service, subtype) */
pus_status_t pus_handler_register(pus_context_t *ctx,
                                   pus_service_t service, pus_subtype_t subtype,
                                   pus_tc_handler_t handler, void *user_data);
```

## Memory

- **Code size**: ~7.7 kB (text segment, all services, unstripped)
- **Per-context RAM**: `sizeof(pus_context_t)` — handler table + counters
- **No heap**: zero dynamic allocation; all buffers are caller-supplied

## Limitations

- No CCSDS Space Packet primary header (library operates on the PUS secondary header and payload layer only)
- Single-threaded: `pus_context_t` is not thread-safe; protect with a mutex if used from multiple tasks
- Fixed-capacity tables: handler slots and service contexts are bounded by `PUS_MAX_TC_HANDLERS`, `PUS_SERVICE_3_MAX_STRUCTURES`, `PUS_SERVICE_20_MAX_PARAMS` (all overridable at compile time via `-D`)

## References

- ECSS-E-ST-70-41C — *Telemetry and Telecommand Packet Utilization*, European Cooperation for Space Standardization

## License

See [LICENSE](LICENSE).
