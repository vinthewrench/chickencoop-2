#include "system_sleep.h"
#include "console/mini_printf.h"

void system_sleep_until(uint16_t minute)
{
    unsigned h = minute / 60;
    unsigned m = minute % 60;

    mini_printf("[HOST] would sleep until %02u:%02u\n", h, m);
}
