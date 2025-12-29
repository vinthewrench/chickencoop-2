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
 *  - Magic + version guard against garbage and layout changes
 *
 * Updated: 2025-12-29
 */

#include "config.h"
#include <avr/eeprom.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * EEPROM layout
 * -------------------------------------------------------------------------- */

#define CONFIG_MAGIC   0x434F4F50UL  /* 'COOP' */
#define CONFIG_VERSION 1

struct config_blob {
    uint32_t      magic;
    uint8_t       version;
    struct config cfg;
};

/* Single-slot EEPROM storage */
static struct config_blob EEMEM ee_cfg_blob;

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void config_load(struct config *cfg)
{
    struct config_blob blob;

    eeprom_read_block(&blob, &ee_cfg_blob, sizeof(blob));

    /* Validate EEPROM contents */
    if (blob.magic != CONFIG_MAGIC ||
        blob.version != CONFIG_VERSION) {

        /* Fresh or invalid EEPROM */
        config_defaults(cfg);
        return;
    }

    *cfg = blob.cfg;
}

void config_save(const struct config *cfg)
{
    struct config_blob blob;

    blob.magic   = CONFIG_MAGIC;
    blob.version = CONFIG_VERSION;
    blob.cfg     = *cfg;

    eeprom_update_block(&blob, &ee_cfg_blob, sizeof(blob));
}
