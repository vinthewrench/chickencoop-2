/*
 * config_sw.h
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

// src/config_sw.h
#pragma once
#include <stdbool.h>

/*
 * CONFIG slide switch state.
 *
 * Semantics:
 * - Sampled once per boot (firmware), cached in RAM.
 * - Host build may stub/override.
 * - No debounce required (slide switch).
 */
bool config_sw_state(void);
