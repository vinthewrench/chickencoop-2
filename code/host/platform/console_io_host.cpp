/*
 * console_io_host.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Host console I/O implementation
 *
 * Responsibilities:
 *  - Read console input from stdin
 *  - Write console output to stdout
 *  - Support non-blocking polling for interactive console use
 *  - Support indefinite blocking when console timeout is suspended
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *  - Host-only implementation (not used on firmware)
 *
 * Timeout behavior:
 *  - Normal mode: console_getc() performs a non-blocking poll
 *  - Suspended mode: console_getc() blocks indefinitely until input
 *  - Mode is controlled via console_suspend_timeout() /
 *    console_resume_timeout()
 *
 * Updated: 2025-12-29
 */

#include "console/console_io.h"
#include <unistd.h>
#include <sys/select.h>
#include <stdbool.h>

// console_io_host.cpp

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>

static bool g_console_inited = false;
static struct termios g_orig_termios;

void console_terminal_init(void)
{
    if (g_console_inited)
        return;

    g_console_inited = true;

    /* make stdin non-blocking */
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    /* optional but strongly recommended */
    if (isatty(STDIN_FILENO)) {
        struct termios t;
        tcgetattr(STDIN_FILENO, &g_orig_termios);
        t = g_orig_termios;

        t.c_lflag &= ~(ICANON | ECHO);
        t.c_cc[VMIN]  = 0;
        t.c_cc[VTIME] = 0;

        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
}

int console_getc(void)
{
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    if (n == 1)
        return c;

    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            return -1;
    }

    /* EOF or real error */
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
