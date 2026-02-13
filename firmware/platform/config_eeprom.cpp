/*
 * config_eeprom.cpp
 *
 * Project: Chicken Coop Controller
 * Purpose: EEPROM-backed configuration storage
 *
 * Notes:
 *  - Offline system
 *  - Deterministic behavior
 *  - EEPROM contents are untrusted
 *  - Config is self-describing (magic + version + checksum)
 *
 * Updated: 2026-01-05
 */

#include "config.h"

#include <avr/eeprom.h>
#include <stddef.h>

/* --------------------------------------------------------------------------
 * EEPROM storage
 * -------------------------------------------------------------------------- */

/* Single-slot EEPROM storage for full config */
static struct config EEMEM ee_cfg;

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

bool config_load(struct config *cfg)
{
    struct config tmp;

    /* Read raw config from EEPROM */
    eeprom_read_block(&tmp, &ee_cfg, sizeof(tmp));

    /* Validate identity */
    if (tmp.magic != CONFIG_MAGIC ||
        tmp.version != CONFIG_VERSION) {

        /* Fresh EEPROM or incompatible layout */
        config_defaults(cfg);
        return false;
    }

    /* Validate checksum */
    uint16_t stored = tmp.checksum;
    uint16_t computed = config_fletcher16(
        &tmp,
        offsetof(struct config, checksum)
    );

    if (stored != computed) {
        /* Corrupt EEPROM contents */
        config_defaults(cfg);
        return false;
    }

    /* Accept config */
    *cfg = tmp;
    return true;
}

void config_save(const struct config *cfg)
{
    struct config tmp = *cfg;

    /* Ensure identity is correct */
    tmp.magic   = CONFIG_MAGIC;
    tmp.version = CONFIG_VERSION;

    /* Compute checksum */
    tmp.checksum = 0;
    tmp.checksum = config_fletcher16(
        &tmp,
        offsetof(struct config, checksum)
    );

    /* Write atomically */
    eeprom_update_block(&tmp, &ee_cfg, sizeof(tmp));
}
