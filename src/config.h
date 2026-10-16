/* ═══════════════════════════════════════════════════════════
   ███████╗██████╗ ███████╗ ██████╗████████╗███████╗██████╗
   ██╔════╝██╔══██╗██╔════╝██╔════╝╚══██╔══╝██╔════╝██╔══██╗
   ███████╗██████╔╝█████╗  ██║        ██║   █████╗  ██████╔╝
   ╚════██║██╔═══╝ ██╔══╝  ██║        ██║   ██╔══╝  ██╔══██╗
   ███████║██║     ███████╗╚██████╗   ██║   ███████╗██║  ██║
   ╚══════╝╚═╝     ╚══════╝ ╚═════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝

   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   config.h — board pins & build constants (ESP32-WROOM-32, 38-pin)
   ═══════════════════════════════════════════════════════════ */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ──────────────────────────────────────────────
//  Build info — single source of branding
// ──────────────────────────────────────────────
#define FW_NAME     "SpecterOS"
#define FW_VERSION  "v2"

// Legacy aliases (kept so every module shares one name/version)
#define OS_NAME     FW_NAME
#define OS_VERSION  FW_VERSION

// ──────────────────────────────────────────────
//  Features
// ──────────────────────────────────────────────
// Use the ESP32's own Wi-Fi radio as a 3rd transmitter (beacon flood).
// OFF by default: it adds a large TX current spike on the 3.3 V rail that
// can brown-out the board when two NRF24+PA/LNA modules are also running.
// Set to 1 only if the radios have a solid external 3.3 V supply.
#define ENABLE_WIFI_FLOOD  0

// Per-channel dwell (microseconds). The NRF24 PLL needs ~130 us to relock
// after a channel change; without a dwell the carrier never settles and
// radiates little real power. ~200 us lets it lock and transmit at full
// power before the next hop. Lower = wider/faster sweep, less power/channel.
#define NRF_DWELL_US       200

// ──────────────────────────────────────────────
//  OTA — wireless firmware updates
//  Active only while IDLE (standby). Wi-Fi is brought down during jamming
//  to save power and free the ESP32 radio, so flash firmware from standby.
// ──────────────────────────────────────────────
#define ENABLE_OTA         0

#define OTA_AP_MODE        1                // 1 = device hosts its own Wi-Fi AP; 0 = join an existing network
#define OTA_HOSTNAME       "specteros"      // appears as a network port in the Arduino IDE
#define OTA_AP_SSID        "SpecterOS-OTA"  // AP name          (OTA_AP_MODE = 1)
#define OTA_AP_PASS        "specter2024"    // AP password, >= 8 chars
#define OTA_STA_SSID       "your-wifi"      // network to join  (OTA_AP_MODE = 0)
#define OTA_STA_PASS       "your-password"
#define OTA_PASSWORD       "specter"        // required to push firmware

// OTA and the Wi-Fi flood both own the single ESP32 Wi-Fi radio — only one.
#if ENABLE_OTA && ENABLE_WIFI_FLOOD
#error "Enable only one of ENABLE_OTA / ENABLE_WIFI_FLOOD (both use the ESP32 Wi-Fi radio)."
#endif

// ──────────────────────────────────────────────
//  Pin Definitions — ESP32-WROOM-32 (38-pin DevKit, dual SPI: VSPI + HSPI)
//  Boot-safe layout: all CS lines idle HIGH (matches strapping),
//  no input-only (34/35/36/39) or flash (6-11) pins used.
// ──────────────────────────────────────────────

// Built-in LED — heartbeat while jamming
#define PIN_LED         2

// NRF24L01 #1 — on VSPI (CE / CSN)
#define NRF_CE_PIN_A    4
#define NRF_CSN_PIN_A   5

// VSPI Bus (RadioA)
//   SCK  → GPIO 18
//   MISO → GPIO 19
//   MOSI → GPIO 23

// NRF24L01 #2 — on HSPI (separate bus, no contention)
#define NRF_CE_PIN_B    16
#define NRF_CSN_PIN_B   17

// HSPI Bus (RadioB)
#define HSPI_SCK_PIN    14
#define HSPI_MISO_PIN   12
#define HSPI_MOSI_PIN   13

// ──────────────────────────────────────────────
//  0.96" OLED — SSD1306 128x64 I2C (4-pin)
//    VCC → 3.3V    GND → GND
//    SDA → GPIO 21    SCL → GPIO 22   (default Wire pins)
// ──────────────────────────────────────────────
#define OLED_SDA_PIN    21
#define OLED_SCL_PIN    22

// ──────────────────────────────────────────────
//  Buttons — active-low, INPUT_PULLUP
//  (wire each button's other leg → GND)
// ──────────────────────────────────────────────
#define BTN_ACTIVATE_PIN  25   // Toggle ON / OFF
#define BTN_MODE_PIN      26   // Cycle to next mode
#define BTN_PREV_PIN      27   // Cycle to previous mode

#endif // CONFIG_H
