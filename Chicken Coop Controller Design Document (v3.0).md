<style>
@media print {
  body { font-family: Georgia, serif; line-height: 1.5; margin: 2cm; }
 /*h1 { page-break-before: always; } *//* New sections on new pages */
  pre, code { background: #f4f4f4; padding: 10px; }
  hr { page-break-after: always; visibility: hidden; }  /* Use --- for manual breaks */
}
</style>

# Standalone Chicken Coop Controller Design Document (v3.0)

**Date:** Jan 5, 2026  

**Sources of Truth:**  
- KiCad schematic (`chickencoop3.kicad_sch`)  
- Locked MCU pin assignments   

## 1. Purpose and Scope (Goal)

The controller automates chicken coop door operation in off-grid or remote setups by opening the door at **sunrise** and closing/securing it at **sunset**. This protects chickens from nocturnal predators while allowing free-range access during daylight hours.

Core design goals:  
- Fully **offline** — no internet, cloud, GPS, or external sensors.  
- One-time manual setup of date, time, latitude, and longitude by the farmer via serial console.  
- Months-long operation on a single 12V motorcycle-size SLA/AGM battery (typically 7–12 Ah).  
- Deterministic and fully inspectable behavior — all states observable via LED, multimeter, or serial console only.  
- Minimal components for maximum reliability and lowest possible power consumption.  

The system assumes a fixed geographic location and uses local standard time (no automatic DST handling).

## 2. How It Works (High-Level Overview)

1. **Initial Setup**  
   The farmer connects a serial console (e.g., via FTDI adapter) and uses simple text commands to set current date/time, latitude, longitude, and timezone offset. These values are stored persistently in the ATmega32U4 EEPROM.

2. **Normal Operation (RUN Mode)**  
   - The MCU spends >99% of its time in power-down sleep.  
   - It wakes via RTC alarm interrupt or manual door button press.  
   - On wake: reads current date/time from the PCF8523T RTC.  
   - Computes sunrise and sunset times for the current date using the stored location and the ported buelowp/sunset algorithm.  
   - If current time matches sunrise → opens door (extend actuator).  
   - If current time matches sunset → closes door (retract actuator), then pulses lock actuator.  
   - Programs the next RTC alarm (sunrise or sunset) and returns to sleep.  

3. **Manual Override**  
   Pressing the illuminated door button wakes the MCU and toggles the door state (open ↔ close + lock/unlock as needed).

4. **Status Indication**  
   The full-face illumination of the button actuator provides clear visual feedback (motion, locked, config needed, fault).

5. **Power Management**  
   12V battery powers actuators directly. The Pololu D24V22F5 switching regulator provides efficient 5V for logic with very low quiescent current.

Door position is inferred mechanically from the PA-14 actuator’s internal limit switches combined with firmware-enforced maximum runtimes. No external sensors are used.

Firmware is written in AVR C++, interrupt-driven, no RTOS, total size comfortably under 32 KB flash.

## 3. System Overview – Major Components

- **MCU**: ATmega32U4RC-AU (TQFP-44, internal RC oscillator)  
- **RTC**: PCF8523T (SOIC-8, I2C, alarm interrupt, CR2032 backup, 32.768 kHz crystal AB26T-32.768kHz)  
- **Door Actuator**: PA-14 linear actuator (12 V, internal non-adjustable limit switches)  
- **Lock Actuator**: Generic 12 V automotive door lock actuator (time-pulsed only)  
- **Motor Drivers**: 2 × VNH7100BASTR (INA/INB/EN control)  
- **5 V Regulator**: Pololu D24V22F5 (step-down switching regulator, 5.3–36 V input, 5 V @ up to 2.5 A output)  
- **Battery**: 12 V motorcycle-size sealed lead-acid or AGM  
- **User Interface / Status Indicator**: FL12DRG5 (EOZ) illuminated momentary pushbutton  
  - Integrates SPST-NO switch contact and full-face bi-color (red/green) LED at 5 V.  
  - Panel-mount (12 mm cutout), IP67 sealed, off-board via cable.

## 4. Power Architecture

- **Direct 12 V paths**: Battery → actuators (via motor drivers).  
- **5 V rail**: Pololu D24V22F5  
  - Input range: 5.3–36 V (ideal for 12 V SLA/AGM with charging margin).  
  - Output: 5 V @ up to 2.5 A.  
  - Quiescent current: ~1 mA (enabled).  
  - EN pin: Left unconnected (internal 270 kΩ pull-up to VIN keeps regulator enabled).  
  - Built-in protections: reverse-voltage, over-current, thermal shutdown.  

- **Sleep optimization**: Regulator remains enabled (EN floating). MCU enters power-down mode, achieving overall sleep current dominated by regulator's ~1 mA (acceptable for months-long battery life on 7–12 Ah capacity).  

- **Additional Recommended Components** (around D24V22F5):  
  - Input: 33–47 µF electrolytic (≥50 V) + 0.1–10 µF ceramic between VIN and GND (close to pins) for noise suppression on long battery leads.  
  - Output: 10–47 µF low-ESR capacitor (≥10 V) between VOUT and GND for load stability.  
  - PG pin: Optional 10–100 kΩ pull-up to +5 V if monitoring Power Good.  

- **Protection**:  
  - 5 A resettable polyfuse in series with battery positive.  
  - Reverse polarity protection built into D24V22F5.  

Target total sleep current: ~1 mA (yielding months of operation on a 7–12 Ah battery).

## 5. MCU Details and Locked Pin Assignments

**MCU Part Number**: ATmega32U4 (RC variant for internal oscillator)  
**Package**: 44-pin TQFP  
**Voltage**: 5 V  
**Source of truth for pinout**: Microchip Atmel-7766 Figure 1-1  

**MCU Specs**  
- Flash: 32 KB  
- SRAM: 2.5 KB  
- EEPROM: 1 KB  
- Clock: Internal RC  
- Voltage: 5 V  

**Locked Design Decisions**  
- Programming: AVR-ISP-6 only  
- USB: Not used (connector not populated, D+/D- NC)  
- RTC wake: External interrupt  
- Serial console: UART (USART1)  
- Door and lock drivers: 2 × VNH7100BASTR  
- All references use **physical pin numbers**  

**Boot/USB/Recovery**  
- Reflash method: ISP only (no DFU reliance)  
- HWB (PE2, Pin 33): Pulled HIGH (10 kΩ) to prevent accidental bootloader entry  

**Power Pins**  
- Pin 2: UVcc (USB supply – not used)  
- Pin 5: UGnd  
- Pin 6: UCap (USB regulator cap)  
- Pin 7: VBus (USB sense – NC)  
- Pin 14: VCC  
- Pin 15: GND  
- Pin 23: GND  
- Pin 24: AVCC  
- Pin 34: VCC  
- Pin 35: GND  
- Pin 42: AREF  
- Pin 43: GND  
- Pin 44: AVCC  
- Pin 13: RESET  

**Decoupling (Required)**  
- 0.1 µF ceramic at each supply pin group (VCC/AVCC/UVCC) placed close to MCU.  
- At least one local bulk capacitor (4.7–10 µF) near MCU supply.  

**Pin Assignments**

### ISP (AVR-ISP-6)
| Signal | Physical Pin | Port/Function |
|--------|--------------|---------------|
| SCK    | 9            | PB1           |
| MOSI   | 10           | PB2           |
| MISO   | 11           | PB3           |
| RESET  | 13           | RESET         |
| VCC    | 14           | VCC           |
| GND    | 15           | GND           |

### I2C (PCF8523T RTC)
| Signal | Physical Pin | Port/Function | Notes |
|--------|--------------|---------------|-------|
| SCL    | 18           | PD0           | 4.7 kΩ pull-up to +5 V |
| SDA    | 19           | PD1           | 4.7 kΩ pull-up to +5 V |

**I2C Pull-ups**  
If the PCF8523T module has no built-in pull-ups, add 4.7 kΩ to +5 V on SCL/SDA.

### RTC Wake Interrupt
| Signal   | Physical Pin | Port/Function          | Notes                     |
|----------|--------------|------------------------|---------------------------|
| RTC_INT  | 12            | PB7                   | 10 kΩ pull-up to VCC      |

### Serial Console (e.g., SparkFun DEV-15096 compatible)
| Physical Pin | Port/Function | SparkFun Pin | Notes                  |
|--------------|---------------|--------------|------------------------|
| 20           | PD2 / RXD1    | 3 (TXO)      | Optional 1 kΩ series   |
| 21           | PD3 / TXD1    | 2 (RXI)      | Optional 1 kΩ series   |
| GND          | GND           | 6            |                        |

No DTR, CTS, or VCC connection.

### Configuration Switch

| Signal   | Physical Pin | Port/Function          | Notes                     |
|------------|--------------|------------------------|---------------------------|
| CONGIG_SW  | 31            | PC6                   | 10 kΩ pull-up to VCC      |

- SPDT switch (one throw to GND, other NC)  
- Internal pull-up enabled  
- Sampled once at boot

### Door Pushbutton and Integrated Status LED (FL12DRG5)
**Part**: FL12DRG5 (EOZ illuminated momentary pushbutton, bi-color red/green, 5 V fixed LED).    
 - Test shows that @5v - 240Ω series,  Red  = ~11mA, Green = ~9ma


**Implementation**  
- The FL12DRG5 is mounted on the enclosure panel and connected to the PCB via a **4-pin cable and connector**.  
- This single part provides both the manual door override button **and** the sole status indicator (full-face illuminated actuator).  
- No separate status LED or dedicated LED driver is used.

**Connections** (via 4-pin connector/cable):
| Signal              | MCU Pin (Physical) | Port | Notes                                      |
|---------------------|--------------------|------|--------------------------------------------|
| Switch signal (NO)  | Pin 28             | PB4  | Active low to GND, 220 Ω series + 0.1 µF debounce on PCB |
| Common GND          | —                  | GND  | Shared for switch and LED cathode          |
| Red LED anode       | Pin 29             | PB5  | Direct drive (built-in limiting resistors) |
| Green LED anode     | Pin 30             | PB6  | Direct drive (built-in limiting resistors) |

**Button Function**  
- Momentary press wakes MCU (PCINT7) and toggles door state (open ↔ close + lock/unlock as needed).

**LED Behavior** (full-face illumination of button actuator)  
Priority (highest first):

| Priority | Condition                  | LED Pattern (Button Face)                     | Meaning                          |
|----------|----------------------------|----------------------------------------------|----------------------------------|
| 1        | Fault                      | Solid RED                                    | Actuator timeout, invalid state, etc. |
| 2        | Config required            | Solid YELLOW (red + green)                   | No valid location/date in EEPROM |
| 3        | Door motion                | Blink @ 2 Hz:<br>• **RED** = closing<br>• **GREEN** = opening | Clear directional feedback       |
| 4        | Door closed + locked       | RED pulse every 15 s (20–30 ms on)           | Nighttime confirmation           |
| 5        | Idle (door open, daytime)  | LEDs completely OFF                          | Power saving                     |

This integrated approach reduces component count, wiring, and power draw while providing highly visible, directional status feedback directly on the button the farmer presses.

### VNH7100BASTR Motor Drivers
**Lock Driver**
| Signal   | Physical Pin | Port |
|----------|--------------|------|
| LOCK_INA | 41           | PF0  |
| LOCK_INB | 40           | PF1  |
| LOCK_EN  | 39           | PF4  |

**Door Driver**
| Signal   | Physical Pin | Port |
|----------|--------------|------|
| DOOR_INA | 38           | PF5  |
| DOOR_INB | 37           | PF6  |
| DOOR_EN  | 36           | PF7  |

**Relay Drivers (Optional)**
| Signal        | Physical Pin | Port |
|---------------|--------------|------|
| RELAY_1_SET   | 22           | PD5  |
| RELAY_1_RESET | 25           | PD4  |
| RELAY_2_SET   | 26           | PD6  |
| RELAY_2_RESET | 27           | PD7  |

Notes:  
- PF pins require AVCC powered.  
- JTAG disabled (fuses) to free PF4–PF7.  
- Default firmware state: EN = 0, INA/INB = 0.

### Complete Pin Assignment Table
| Pin # | Port / Signal | Assigned Function                  |
|-------|---------------|------------------------------------|
| 1     | PE6           | Unused                             |
| 2     | UVcc          | USB supply (not used)              |
| 3     | D-            | USB (NC)                           |
| 4     | D+            | USB (NC)                           |
| 5     | UGnd          | USB ground                         |
| 6     | UCap          | USB regulator cap                  |
| 7     | VBus          | USB sense (NC)                     |
| 8     | PB0           | Unused                             |
| 9     | PB1           | ISP SCK                            |
| 10    | PB2           | ISP MOSI                           |
| 11    | PB3           | ISP MISO                           |
| 12    | PB7           | RTC_INT (INT6)                     |
| 13    | RESET         | RESET                              |
| 14    | VCC           | +5 V                               |
| 15    | GND           | Ground                             |
| 16    | XTAL2         | Unused (RC clock)                  |
| 17    | XTAL1         | Unused (RC clock)                  |
| 18    | PD0           | I2C SCL                            |
| 19    | PD1           | I2C SDA                            |
| 20    | PD2           | UART RXD1                          |
| 21    | PD3           | UART TXD1                          |
| 22    | PD5           | RELAY_1_RESET                      |
| 23    | GND           | Ground                             |
| 24    | AVCC          | Analog supply                      |
| 25    | PD4           | RELAY_1_SET                        |
| 26    | PD6           | RELAY_2_SET                        |
| 27    | PD7           | RELAY_2_RESET                      |
| 28    | PB4           | DOOR_SW (FL12DRG5 switch)          |
| 29    | PB5           | Red LED anode (FL12DRG5)           |
| 30    | PB6           | Green LED anode (FL12DRG5)         |
| 31    | PC6           | CONFIG_SW                          |
| 32    | PC7           | Unused                             |
| 33    | PE2           | HWB (pulled high)                  |
| 34    | VCC           | +5 V                               |
| 35    | GND           | Ground                             |
| 36    | PF7           | DOOR_EN                            |
| 37    | PF6           | DOOR_INB                           |
| 38    | PF5           | DOOR_INA                           |
| 39    | PF4           | LOCK_EN                            |
| 40    | PF1           | LOCK_INB                           |
| 41    | PF0           | LOCK_INA                           |
| 42    | AREF          | Analog reference                   |
| 43    | GND           | Ground                             |
| 44    | AVCC          | Analog supply                      |

## 6. Sunrise/Sunset Computation

Algorithm ported from **buelowp/sunset** (https://github.com/buelowp/sunset – Mike Chirico’s sunrise.c).

- Inputs: year, month, day, latitude (signed decimal degrees), longitude (signed), manual timezone offset (hours).  
- Core calculations: Julian day → solar declination → equation of time → hour angle → local sunrise/sunset times.  
- Output: minutes past midnight for sunrise and sunset.  
- Porting notes for AVR:  
  - Use double-precision floating point (soft-float supported by avr-gcc).  
  - Or convert to fixed-point for smaller code/size if needed.  
  - Accuracy ±2–5 minutes (more than sufficient for coop use).  
  - Handles edge cases (polar day/night returns invalid → no event).  

## 7. Persistent Storage

Latitude, longitude, timezone offset, and initial date/time are stored in the **ATmega32U4’s 1 KB EEPROM** for persistence across full power loss.

## 8. RTC and Timekeeping

- PCF8523T runs continuously on CR2032 backup when main battery is removed.  
- Sole source of truth for date/time.  
- Firmware sets single alarm interrupt for next sunrise or sunset event.

## 9. Actuator Control Logic

- **Door (PA-14)**: Enable driver, set INA/INB direction, run until firmware timeout (suggested 30 s max) or internal limits stop current.  
- **Lock**: Enable driver, set direction, pulse 150–400 ms (max 500 ms), disable. Never left energized.  

Default driver state: EN = 0, INA/INB = 0.

## 10. Boot and Configuration

- Boot samples CONFIG_SW (PB4).  
- Grounded → CONFIG mode (serial console active).  
- Open → RUN mode.  
- CONFIG commands (text, 115200 8N1 suggested):  
  - `set date YYYY-MM-DD`  
  - `set time HH:MM:SS`  
  - `set lat ±DD.DDDD`  
  - `set long ±DDD.DDDD`  
  - `set tz ±HH` (timezone offset)  
  - `open` / `close` / `lock` / `unlock` (manual test)  
- 60 s inactivity → auto-enter RUN mode.

## 11. Sleep Strategy

- MCU in power-down mode.  
- Wake sources: RTC INT (PE6), door button (PB7/PCINT).  
- All unnecessary peripherals disabled.  
- Regulator remains enabled (EN floating).  

## 12. Bill of Materials (Key Electrical Items)

- ATmega32U4RC-AU  
- PCF8523T  
- 2 × VNH7100BASTR  
- Pololu D24V22F5 (5 V step-down regulator)  
- AB26T-32.768kHz crystal  
- CR2032 + holder  
- FL12DRG5 (EOZ) — illuminated momentary pushbutton, bi-color red/green, 5 V  
- Passives: resistors (4.7 k, 10 k, 220 Ω, etc.), capacitors (input/output for regulator, 0.1 µF decoupling ceramics, bulk caps), polyfuse  

## 13. Explicit Exclusions

- No sensors  
- No wireless  
- No cloud/GPS  
- No automatic DST  
- No encoders/feedback  
- No heartbeat LED  
- No separate status LED or LED driver IC  

## 14. Design Philosophy

If a state or behavior is not directly observable with an LED, multimeter, or serial console — it does not belong in this system.

Minimalism, inspectability, and battery life take priority over convenience features.

## 15. Firmware Overview (AVR C++)

Firmware is written in **AVR C++** (not plain C), compiled with **avr-gcc** (g++ mode), programmed via **Atmel-ICE (ISP mode)**, and built using a **Makefile**. No Arduino framework or libraries are used.

### Key Firmware Requirements
- Ultra-low power: MCU spends >99% of time in power-down sleep.
- Interrupt-driven only: No polling loops.
- Deterministic behavior: All actions triggered by RTC alarm or button press.
- Persistent configuration: Latitude, longitude, timezone offset stored in EEPROM (with CRC validation).
- Sunrise/sunset calculation using fixed-point port of buelowp/sunset algorithm.
- Simple text-based serial console in CONFIG mode.

### Startup Sequence (main() and Initialization)
```cpp
int main(void) {
    // 1. Disable watchdog (in case of previous reset)
    wdt_disable();

    // 2. Disable all unnecessary peripherals to minimize power
    PRR = (1 << PRTWI) | (1 << PRTIM0) | (1 << PRTIM1) | (1 << PRTIM2) |
          (1 << PRADC) | (1 << PRUSART0) | (1 << PRSPI);  // Keep USART1 for console

    // 3. Configure pins
    // Inputs with pull-ups
    DDRB &= ~((1 << PB7) | (1 << PB4));  // DOOR_SW and CONFIG_SW as inputs
    PORTB |= (1 << PB7) | (1 << PB4);    // Enable internal pull-ups

    // Outputs default low/off
    DDRB |= (1 << PB5) | (1 << PB6);     // LED control (red/green)
    DDRF |= (1 << PF7) | (1 << PF6) | (1 << PF5) |   // Door driver
            (1 << PF4) | (1 << PF1) | (1 << PF0);    // Lock driver
    PORTB &= ~((1 << PB5) | (1 << PB6));  // LEDs off
    PORTF &= ~((1 << PF7) | (1 << PF6) | (1 << PF5) |
               (1 << PF4) | (1 << PF1) | (1 << PF0));  // Drivers disabled

    // 4. Sample CONFIG_SW once (latched until reboot)
    bool config_mode = (PINB & (1 << PB4)) == 0;

    // 5. Initialize UART (only if config mode, but safe to init always)
    uart_init(115200);

    // 6. Initialize I2C and RTC
    i2c_init();
    rtc_init();  // Verify communication, set 32kHz output if needed

    // 7. Enable interrupts for wake sources
    PCICR |= (1 << PCIE0);        // Enable pin change interrupt on PCINT[7:0] (PB7)
    PCMSK0 |= (1 << PCINT7);      // Mask for PB7 (door button)
    EIMSK |= (1 << INT6);         // Enable INT6 on PE6 (RTC alarm)
    EIFR |= (1 << INTF6);         // Clear any pending flag

    sei();  // Global interrupt enable

    if (config_mode) {
        enter_config_mode();  // Blocking console loop with inactivity timeout
        // On exit: falls through to RUN mode
    }

    // 8. Load settings from EEPROM (validate with CRC)
    load_settings_from_eeprom();

    // 9. Enter normal RUN mode
    run_mode_loop();  // Never returns
}
```
### Sleep and Wake Strategy
```cpp
void enter_sleep() {
    // Turn off LEDs and drivers
    PORTB &= ~((1 << PB5) | (1 << PB6));
    PORTF &= ~((1 << PF7) | (1 << PF6) | (1 << PF5) |
               (1 << PF4) | (1 << PF1) | (1 << PF0));

    // Additional power reduction
    PRR |= (1 << PRTWI) | (1 << PRUSART1);  // Disable I2C & UART after use

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();              // Ensure interrupts enabled before sleep
    sleep_cpu();        // Enter sleep — wakes on RTC INT or button PCINT
    sleep_disable();    // On wake

    // Re-enable peripherals as needed
    PRR &= ~((1 << PRTWI) | (1 << PRUSART1));
}
```
### RUN Mode Main Loop
```cpp
void run_mode_loop() {
    while (1) {
        // Read current date/time from RTC
        rtc_get_datetime(&current_time);

        // Compute sunrise/sunset for today
        compute_sunrise_sunset(current_time.year, current_time.month, current_time.day,
                               settings.lat, settings.lon, settings.tz,
                               &sunrise_min, &sunset_min);

        // Determine next event and program RTC alarm
        uint16_t next_alarm_min;
        if (should_open_now(&current_time, sunrise_min)) {
            open_door();
            next_alarm_min = sunset_min;
        } else if (should_close_now(&current_time, sunset_min)) {
            close_door_and_lock();
            next_alarm_min = sunrise_min;  // Tomorrow's sunrise
        } else {
            // Determine which is next
            next_alarm_min = closer_event(current_time.minutes_past_midnight(),
                                          sunrise_min, sunset_min);
        }

        rtc_set_alarm(next_alarm_min);

        // Update LED state (e.g., pulse if locked)
        update_status_led();

        enter_sleep();  // Sleep until next alarm or button press
    }
}
```
### Interrupt Service Routines
```cpp
ISR(INT6_vect) {
    // RTC alarm — cleared automatically by reading status register
    // Just wake — main loop handles action
}

ISR(PCINT0_vect) {
    if ((PINB & (1 << PB7)) == 0) {  // Button pressed (active low)
        // Debounce in main loop or simple delay
        manual_door_toggle();
    }
}
```
### Key Notes (Firmware)
- All heavy work (sunrise/sunset calculation, actuator drive, LED updates) is performed immediately after wake — the system returns to sleep as soon as possible.
- Serial console is active only in CONFIG mode (or optionally in background if needed for debugging).
- Faults are indicated via LED blink patterns (e.g., solid red on actuator timeout or RTC error).
- Avoid floating-point math if possible — use fixed-point arithmetic for the sunrise/sunset algorithm to reduce code size and execution time on the AVR.
- EEPROM writes are minimized: only on configuration changes (rare) to prevent wear.
- Power reduction registers (PRR) are used aggressively to disable unused peripherals before sleep.
- Interrupt service routines are kept minimal — heavy logic stays in the main loop for readability and debugging.

## 16. Technical Review (Deep Dive – v2.3)

This section provides a detailed technical review of the design as documented in v2.3. All claims are based on datasheets, calculations, and practical experience with similar systems.

### MCU and Pin Assignments
- **ATmega32U4RC-AU**: Excellent choice for low-power, offline work. Internal RC oscillator avoids external crystal power draw (~µA savings). Power-down sleep with PCINT and external interrupt enabled draws ~0.1–1 µA (datasheet Table 29-1).  
- **Pin usage**: Fully locked and conflict-free. PF4–PF7 freed by JTAG disable (fuses). Unused pins (PD4–PD7, PC6–PC7, PB0) should be configured as outputs low or inputs pull-up disabled to minimize leakage.  
- **Fuses recommended**:  
  - Low: 0xFF (8 MHz internal RC, no CLKDIV8)  
  - High: 0xD8 (JTAG disabled, boot size 0)  
  - Extended: 0xF5 (BOD ~4.3 V for 5 V stability)  
- **Risk**: No ADC used — disable in PRR to save ~100 µA potential. Internal RC calibration drift ±10% — irrelevant for timing (RTC is master).

### Power Architecture (Pololu D24V22F5)
- **Performance**: Synchronous buck, ~90–95% efficiency at 12 V → 5 V. Quiescent ~1 mA enabled (datasheet typical). EN floating = correct (internal pull-up).  
- **External caps**: Input 33–47 µF electrolytic + 0.1–10 µF ceramic recommended for long battery wires/actuator spikes. Output 10–47 µF low-ESR for transients.  
- **Sleep current**: Regulator dominates at ~1 mA → 720 mAh/month. On 7 Ah battery: ~9–10 months. On 12 Ah: 16+ months. Acceptable for goal.  
- **Protection**: Built-in reverse (-40 V), overcurrent, thermal. Polyfuse + optional SMAJ18A TVS on +12 V adds belt-and-suspenders.  
- **Alternative for lower quiescent**: TPS62825 (~4 µA) would extend life significantly, but D24V22F5 is proven and fits footprint—stick with it unless measured sleep >1.5 mA.

### RTC (PCF8523T)
- **Accuracy**: ±20 ppm crystal → ~1–2 min/year drift. Chickens don't care—sufficient.  
- **Backup**: CR2032 lasts 5–10 years at 0.5 µA.  
- **I2C**: Pull-ups 4.7 kΩ fine. Bus short traces to avoid capacitance issues.  
- **Wake**: INT1 to PE6 (INT6) perfect—level-sensitive in power-down.

### Sunrise/Sunset Algorithm
- **Port**: buelowp/sunset is solid NOAA-based. Fixed-point recommended for AVR (no FPU). Execution <10 ms. Accuracy ±2–5 min — more than enough.  
- **Edge cases**: Polar latitudes handled (return invalid → skip event).  
- **Optimization**: Precompute declination table in flash if flash space allows.


### Startup and Power-Restore Behavior

On startup or power restore, the system does not replay missed scheduled events.

Instead, the system computes the desired steady state for the current time based on the active configuration and converges to that state immediately.

This reconciliation process ensures safe, deterministic behavior and prevents unintended actuation caused by delayed or missed events.

For example:

If the configured door open time has passed but the close time has not, the door will be commanded open.

If the configured close time has passed, the door will be commanded closed, regardless of whether the open event was missed.

Past events are never retroactively executed. Only the correct state for the present time is enforced.

This behavior is by design and applies uniformly to all time-scheduled subsystems.

On startup or power restore, the system does not replay missed events.
It computes the desired steady state for the current time and converges to it.


### User Interface (FL12DRG5)
- **Integration**: Brilliant minimalism—one part for button + status. Full-face illumination highly visible.  
- **Current**: Built-in resistors ~10–20 mA/color. Patterns low-duty → average <<1 mA. Direct MCU drive safe (pins rated 20 mA recommended).  
- **Wiring**: 4-pin cable clean for panel mount. IP67 front seal perfect for coop.  
- **Risk**: Exact current unknown until measured—test sample and add series resistors if too bright/power-hungry.

### Actuator Control (VNH7100BASTR)
- **Capability**: Handles PA-14 stall (~1–2 A) and lock pulse easily. EN control allows clean off-state.  
- **Inference**: Mechanical limits + timeout = reliable without sensors.  
- **Risk**: Stuck actuator undetected—LED fault blink covers it (timeout triggers fault).

### Persistent Storage
- **EEPROM**: 1 KB plenty. Writes only on config changes → <10/year → zero wear concern.

### Sleep Strategy
- **Power-down**: Dominant draw is regulator ~1 mA. MCU/RTC/drivers negligible.  
- **Wake**: RTC INT + button PCINT — ideal. No polling.

### Firmware (AVR C++)
- **Build chain**: avr-gcc (g++), Makefile, Atmel-ICE ISP — clean, reproducible, no bloat.  
- **Structure**: Interrupt-driven, sleep-centric — correct for power.  
- **Robustness**: Add CRC on EEPROM read, timeout faults to LED.

### Overall Assessment
- **Strengths**: Minimal, inspectable, deterministic, low-power enough for goal.  
- **Weaknesses**: Regulator quiescent limits life to ~9–16 months (still fine). No over-the-air recovery if firmware bug (but ISP header allows field fix).  
- **Score**: 9/10 — production-ready after first-board test (focus on sleep current measurement and LED brightness).  

No showstoppers. Build the PCB.