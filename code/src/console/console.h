/*
 * console.h
 *
 * Project: Chicken Coop Controller
 * Purpose: Source file
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *
 * Updated: 2025-12-29
 */

// src/console/console.h
#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef HOST_BUILD
static const uint32_t CONFIG_TIMEOUT_SEC = 60;
#else
static const uint32_t CONFIG_TIMEOUT_SEC = 300;
#endif

extern bool want_exit;
extern bool console_timeout_enabled;

void console_init(void);
void console_poll(void);
bool console_should_exit(void);
