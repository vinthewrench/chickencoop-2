/*
 * main_host.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: Host console entry point
 *
 * Design intent:
 *  - Host executes the SAME scheduler logic as firmware
 *  - Host provides visibility and diagnostics only
 *  - No hardware emulation, no shortcuts
 *
 * Execution model:
 *  - Fast loop ticks devices continuously
 *  - Scheduler evaluation is triggered by:
 *      * minute boundary
 *      * OR schedule change (ETag)
 *  - Schedule application is idempotent
 *
 * Updated: 2026-01-08
 */

#include "console/console.h"
#include "uptime.h"
#include "devices/devices.h"

#include "scheduler.h"        /* date + solar cache, ETag */
#include "state_reducer.h"    /* declarative → device intent */
#include "schedule_apply.h"   /* device intent → execution */

#include "rtc.h"
#include "solar.h"
#include "config.h"
#include "time_dst.h"
#include "system_sleep.h"

int main(void)
{
    /* ----------------------------------------------------------
     * System initialization
     * ---------------------------------------------------------- */

    scheduler_init();     /* clears cached day, solar, ETag */
    uptime_init();        /* millisecond / second timebase */
    device_init();        /* initialize all registered devices */
    console_init();       /* interactive console */

    /* ----------------------------------------------------------
     * Scheduler loop state
     * ---------------------------------------------------------- */

    uint16_t last_minute = 0xFFFF;   /* forces first evaluation */
    uint32_t last_etag   = 0;        /* last-seen schedule ETag */

    int last_y  = -1;
    int last_mo = -1;
    int last_d  = -1;

    /* Cached solar context for TODAY */
    struct solar_times sol;
    bool have_sol = false;

    /* ----------------------------------------------------------
     * Main loop
     * ---------------------------------------------------------- */
     for(;;) {

        /* Service console input */
        console_poll();

        /* Tick all devices continuously (non-blocking) */
        device_tick(uptime_millis());

        /* ------------------------------------------------------
         * Read current time
         * ------------------------------------------------------ */
        int y, mo, d, h, m, s;
        rtc_get_time(&y, &mo, &d, &h, &m, &s);

        uint16_t now_minute = (uint16_t)(h * 60 + m);
        uint32_t cur_etag   = schedule_etag();

        /* ------------------------------------------------------
         * Decide whether scheduler must run
         *
         * Triggers:
         *  - minute boundary crossed
         *  - OR schedule changed (events / solar invalidated)
         * ------------------------------------------------------ */
        bool minute_changed  = (now_minute != last_minute);
        bool schedule_dirty  = (cur_etag != last_etag);

        if (!minute_changed && !schedule_dirty)
            continue;

        last_minute = now_minute;
        last_etag   = cur_etag;

        /* ------------------------------------------------------
         * Recompute solar ONCE per calendar day
         *
         * Scheduler does NOT do this.
         * Host owns:
         *  - lat/lon
         *  - timezone
         *  - DST policy
         * ------------------------------------------------------ */
        if (y != last_y || mo != last_mo || d != last_d) {

            have_sol = false;

            /* Only attempt solar if location is configured */
            if (g_cfg.latitude_e4 || g_cfg.longitude_e4) {

                double lat = (double)g_cfg.latitude_e4  / 10000.0;
                double lon = (double)g_cfg.longitude_e4 / 10000.0;

                int tz = g_cfg.tz;
                if (g_cfg.honor_dst && is_us_dst(y, mo, d, h))
                    tz += 1;

                have_sol = solar_compute(
                    y, mo, d,
                    lat,
                    lon,
                    (int8_t)tz,
                    &sol
                );
            }

            /* Inform scheduler of new day context */
            scheduler_update_day(
                y, mo, d,
                have_sol ? &sol : NULL,
                have_sol
            );

            last_y  = y;
            last_mo = mo;
            last_d  = d;
        }

        /* ------------------------------------------------------
         * APPLY SCHEDULE
         *
         * This runs when:
         *  - minute advances
         *  - OR events / solar changed
         *
         * Steps:
         *  1) Reduce declarative events → expected device state
         *  2) Apply that state idempotently
         * ------------------------------------------------------ */
        {
            struct reduced_state rs;

            size_t used = 0;
            const Event *events = config_events_get(&used);

            if (events && used > 0) {

                state_reducer_run(
                    events,
                    MAX_EVENTS,
                    have_sol ? &sol : NULL,
                    now_minute,
                    &rs
                );

                schedule_apply(&rs);
            }
        }

        /* ------------------------------------------------------
         * Optional: compute next event and “sleep”
         * (host prints intent only)
         * ------------------------------------------------------ */
        uint16_t next_min;
        if (scheduler_next_event_minute(&next_min)) {
            system_sleep_until(next_min);
        }
    }

    return 0;
}
