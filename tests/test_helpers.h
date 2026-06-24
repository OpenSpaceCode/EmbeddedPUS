#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "pus_context.h"
#include "pus_types.h"

#include <string.h>

/* Shared capture buffer for the TM sink across all test files. */
static uint8_t th_buf[512];
static uint16_t th_len = 0;

static pus_status_t th_sink(void *ud, const uint8_t *data, uint16_t len)
{
    (void)ud;
    memcpy(th_buf, data, len < sizeof(th_buf) ? len : sizeof(th_buf));
    th_len = len;
    return PUS_STATUS_OK;
}

static pus_context_t th_make_ctx(void)
{
    pus_context_t ctx;
    pus_init(&ctx);
    ctx.tm_sink = th_sink;
    return ctx;
}

#endif /* TEST_HELPERS_H */
