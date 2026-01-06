/*
 * console_io.h
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

#pragma once
int  console_getc(void);
void console_putc(char c);
void console_puts(const char *s);


void console_terminal_init(void);
