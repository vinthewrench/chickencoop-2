/*
 * main_firmware.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Firmware entry point
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - No network dependencies
 *
 * BOOT MODES (CURRENT IMPLEMENTATION):
 * -----------------------------------
 *  - CONFIG mode (service / bring-up)
 *  - RUN mode skeleton (health indication only)
 *
 * Hardware: Chicken Coop Controller V3.0
 *
 * Updated: 2026-01-05
 */

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "config_sw.h"
#include "console/console.h"
#include "uart.h"
#include "relay.h"
#include "rtc.h"
#include "uptime.h"
#include "door_led.h"
#include "door_lock.h"
#include "devices/devices.h"

int main(void)
{
    /* ------------------------------------------------------------------
     * Basic bring-up
     * ------------------------------------------------------------------ */
    uart_init();
    relay_init();
    uptime_init();

    rtc_init();        /* RTC present; policy handled elsewhere */
    door_led_init();
    lock_init();

    /* ------------------------------------------------------------------
     * Load persistent configuration (EEPROM, defaults on failure)
     * ------------------------------------------------------------------ */
    bool cfg_ok = config_load(&g_cfg);
    (void)cfg_ok;   /* intentionally unused during bring-up */

    /* ------------------------------------------------------------------
     * CONFIG mode handling (latched once per boot)
     * ------------------------------------------------------------------ */
    static bool config_consumed = false;

    if (config_sw_state() && !config_consumed) {
        config_consumed = true;

        if (!rtc_time_is_set()) {
            door_led_set(DOOR_LED_BLINK_RED);
        }

        console_init();
        while (!console_should_exit()) {
            console_poll();

            uint32_t now_ms = uptime_millis();
            door_led_tick(now_ms);
            lock_tick(now_ms);
            device_tick(now_ms);
        }
    }

    /* ------------------------------------------------------------------
     * RUN MODE (SKELETON ONLY)
     * ------------------------------------------------------------------ */

    if (!rtc_time_is_set()) {
        door_led_set(DOOR_LED_BLINK_RED);
        for (;;) {
            uint32_t now_ms = uptime_millis();
            door_led_tick(now_ms);
            lock_tick(now_ms);
            device_tick(now_ms);
        }
    }

    /* RTC valid: brief green, then idle */
    door_led_set(DOOR_LED_GREEN);
    uint32_t t0_ms = uptime_millis();
    while ((uint32_t)(uptime_millis() - t0_ms) < 1000u) {
        uint32_t now_ms = uptime_millis();
        door_led_tick(now_ms);
        lock_tick(now_ms);
        device_tick(now_ms);
    }

    door_led_set(DOOR_LED_OFF);

    for (;;) {
        uint32_t now_ms = uptime_millis();
        door_led_tick(now_ms);
        lock_tick(now_ms);
        device_tick(now_ms);
    }
}
