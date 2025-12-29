/*
 * console_io_host.cpp
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

// host/platform/console_io_host.cpp
#include "console/console_io.h"
#include <unistd.h>
#include <sys/select.h>

int console_getc(void)
{
    fd_set rfds;
    struct timeval tv = { 0, 0 };   // non-blocking poll

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    int r = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
    if (r <= 0)
        return -1;

    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return c;

    return -1;
}

void console_putc(char c)
{
    write(STDOUT_FILENO, &c, 1);
}

void console_puts(const char *s)
{
    while (*s)
        console_putc(*s++);
}
