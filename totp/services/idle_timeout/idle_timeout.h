#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef struct IdleTimeoutContext IdleTimeoutContext;

typedef bool (*IDLE_TIMEOUT_CALLBACK)(void* context);

IdleTimeoutContext* idle_timeout_alloc(uint16_t timeout_sec, IDLE_TIMEOUT_CALLBACK on_idle_callback, void* on_idle_callback_context);

void idle_timeout_start(IdleTimeoutContext* context);

void idle_timeout_stop(IdleTimeoutContext* context);

void idle_timeout_report_activity(IdleTimeoutContext* context);

void idle_timeout_free(IdleTimeoutContext* context);