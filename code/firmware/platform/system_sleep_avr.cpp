/*
 * system_sleep_avr.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Low-power sleep implementation for AVR firmware
 *
 * Design:
 *  - No policy
 *  - No scheduling
 *  - No RTC interaction
 *  - No logging
 *
 * Responsibility:
 *  - Enter deepest allowed sleep mode
 *  - Resume execution on ANY enabled interrupt
 *
 * Wake sources are configured elsewhere:
 *  - RTC alarm INT (PCF8523)
 *  - Door button
 *  - Config switch
 *  - Any other enabled PCINT / EXTINT
 *
 * Updated: 2026-01-08
 */

#include "system_sleep.h"

#include <avr/sleep.h>
#include <avr/interrupt.h>

/*
 * Enter low-power sleep until an interrupt occurs.
 *
 * Notes:
 *  - Caller MUST have already:
 *      - Armed RTC alarm if time-based wake is desired
 *      - Enabled relevant interrupt sources
 *  - This function performs no checks and no setup
 *  - Execution resumes immediately after wake
 */
void system_sleep_until(uint16_t minute)
{
    (void)minute;  /* minute is advisory; AVR sleep is interrupt-driven */

    /* Select deepest sleep mode allowed */
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    sleep_enable();

    /*
     * Ensure interrupts are enabled before sleeping.
     * If an interrupt is already pending, sleep_cpu()
     * will return immediately.
     */
    sei();

    /* CPU sleeps here */
    sleep_cpu();

    /* Execution resumes here after wake */
    sleep_disable();
}
