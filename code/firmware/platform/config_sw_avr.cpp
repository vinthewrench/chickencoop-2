/*
 * config_sw_avr.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: CONFIG slide switch (boot-time only)
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *  - CONFIG is sampled once per boot and cached
 *
 * Hardware assumptions (LOCKED v2.5):
 *  - CONFIG slide switch connects PC6 to GND when ON (active-low)
 *  - Internal pull-up enabled
 *  - CONFIG is NOT a wake source
 *
 * Updated: 2026-01-02
 */

// firmware/platform/config_sw_avr.cpp
#include "config_sw.h"
#include <avr/io.h>

/*
 * If wiring or polarity ever changes, this is the only file
 * that should need to be modified.
 */
#define CONFIG_SW_BIT PC6

static bool read_hw_state_once(void)
{
    /* PC6 input */
    DDRC  &= (uint8_t)~_BV(CONFIG_SW_BIT);
    /* enable internal pull-up */
    PORTC |= _BV(CONFIG_SW_BIT);

    /* active-low */
    return (PINC & _BV(CONFIG_SW_BIT)) ? false : true;
}

bool config_sw_state(void)
{
    static int8_t cached = -1;

    if (cached < 0) {
        cached = read_hw_state_once() ? 1 : 0;
    }

    return cached ? true : false;
}
