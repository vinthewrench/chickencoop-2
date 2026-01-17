/*
 * lock.h
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

// src/lock.h
#pragma once
#include <stdbool.h>

void lock_engage(void);
void lock_release(void);
bool lock_is_engaged(void);
