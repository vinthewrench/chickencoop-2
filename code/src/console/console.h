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

extern bool want_exit;

void console_init(void);
void console_poll(void);

bool console_should_exit(void);

void console_suspend_timeout(void);
void console_resume_timeout(void);
