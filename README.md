<div align="center">

<img src="assets/logo.svg" alt="SpecterOS" width="560">

# SpecterOS v2

**ESP32 2.4 GHz RF jammer** — a dual-radio, dual-core burst-hopping engine driven by 3 buttons and a live OLED.

![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-E7352C?logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-framework-00979D?logo=arduino&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-11-00599C?logo=cplusplus&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-ready-FF7F00?logo=platformio&logoColor=white)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-dual--core-1db954)
![Version](https://img.shields.io/badge/version-2.0-22d3ee)
![Band](https://img.shields.io/badge/band-2.4_GHz-a855f7)

</div>

> The Arduino sketch/folder is named `SpecterJam` (Arduino requires the `.ino` to match its folder); the firmware brand is **SpecterOS v2**.

> ⚠️ **Legal notice** — Transmitting on these bands can be illegal and disruptive. Use only in a shielded / RF-isolated environment, or where you are explicitly authorized.

---

## ✨ What's new in v2

- **Dual-SPI, dual-core** — each NRF24 owns a separate SPI bus (VSPI / HSPI) and a dedicated core, so the radios never contend.
- **Optional Wi-Fi flood** — the ESP32's own radio can act as a 3rd transmitter (beacon flood), but it's **off by default** (`ENABLE_WIFI_FLOOD` in `config.h`) because it can brown-out a board that powers the PA/LNA radios off the ESP32's 3.3 V pin.
- **Simple button UI** — replaced the old 5-way D-pad with 3 buttons: toggle, next mode, previous mode.
- **Burst hopping** with the CE-toggle carrier retune that actually moves the carrier each hop.
- **Clean, grouped source tree** under `src/`.

## 🧩 Hardware

| | |
|---|---|
| **MCU** | ESP32-WROOM-32 (38-pin DevKit) |
| **Radios** | 2× NRF24L01(+PA/LNA) — 3.3 V, 10 µF cap across VCC/GND each |
| **Display** | SSD1306 128×64 I²C OLED (4-pin) |
| **Input** | 2 momentary buttons (active-low) |

### Pin map

| Function | GPIO |
|---|---|
| NRF24 #1 (VSPI) CE / CSN | 4 / 5 |
| VSPI SCK / MISO / MOSI | 18 / 19 / 23 |
| NRF24 #2 (HSPI) CE / CSN | 16 / 17 |
| HSPI SCK / MISO / MOSI | 14 / 12 / 13 |
| OLED SDA / SCL | 21 / 22 |
| Button — Activate (ON/OFF) | 25 |
| Button — Next mode | 26 |
| Button — Previous mode | 27 |
| Built-in LED | 2 |

Buttons use `INPUT_PULLUP` — wire each to **GND**, no resistors. OLED VCC → 3.3 V.

## 🎮 Controls

| Button | Serial | Action |
|---|---|---|
| **Activate** | `t` | Toggle ACTIVE / idle |
| **Next** | `n` | Next mode |
| **Prev** | `p` | Previous mode |
| — | `s` | Print status |
| — | `?` | Help |

**Modes** (cycled in order): `WiFi` · `BLE` · `Bluetooth` · `Zigbee` · `Video TX` · `RC` · `USB Wireless` · `NRF24` · `Full Band`.

## ⚙️ How it works

Two FreeRTOS tasks run in parallel while ACTIVE (a third is added only if the Wi-Fi flood is enabled):

| Task | Core | Role |
|---|---|---|
| `jamTaskA` | 0 | NRF24 #1 (VSPI) sweeps the **lower** half of the mode's channel table |
| `jamTaskB` | 1 | NRF24 #2 (HSPI) sweeps the **upper** half, on a separate bus |
| `wifiJamTask` | 0 | *(optional)* ESP32 Wi-Fi floods randomized beacons, hopping channels 1–13 |

Each hop toggles CE around `setChannel()` to retune the constant carrier, then **dwells `NRF_DWELL_US` (~200 µs)** so the NRF24 PLL actually locks and radiates full power before moving on — a settled carrier delivers far more energy than raw hop count suggests. The two radios cover complementary halves of the band so they're never on the same channel. The OLED redraws only on state change, never inside the hop loop.

## 🗂️ Project structure

```text
SpecterJam/
├── SpecterJam.ino            setup() / loop() orchestration
├── README.md
├── assets/
│   ├── logo.svg              project wordmark / banner
│   └── ESP-32.jpg            board reference photo
└── src/
    ├── config.h              pins & build constants (shared)
    ├── radio/                ── RF layer
    │   ├── channels.h/.cpp    operation modes, menu labels, channel tables
    │   └── radio.h/.cpp       NRF24 HAL — carrier + per-radio hop (dual SPI)
    ├── jammer/               ── logic layer
    │   └── jammer.h/.cpp      state machine, dual-core tasks, Wi-Fi beacon flood
    ├── interface/            ── I/O layer
    │   ├── ui.h/.cpp          OLED status screen, button input, LED heartbeat
    │   └── cli.h/.cpp         serial command interface
    └── net/                  ── network layer
        └── ota.h/.cpp         wireless firmware updates (standby only)
```

Arduino compiles `SpecterJam.ino` at the sketch root plus everything under `src/` recursively (all nested folders), so this grouped layout builds with no extra configuration.

Data flow: `cli` / `ui` (buttons) → `jammer` actions → `radio` control; `jammer` calls `uiInvalidate()` to refresh the screen.

## 🔧 Building

**Arduino IDE** — open `SpecterJam.ino`, select **ESP32 Dev Module**, install libraries **RF24** (nRF24/RF24) and **U8g2** (olikraus).

**PlatformIO**

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    nrf24/RF24@^1.4.8
    olikraus/U8g2@^2.35.19
```

(`ArduinoOTA` and `WiFi` for OTA are part of the ESP32 core — no extra library needed.)

## 📡 OTA — wireless firmware updates

Once the first build is flashed over USB, later updates can go over the air. OTA is **active only while idle** — the moment jamming starts, Wi-Fi is torn down (saves power, frees the ESP32 radio) and comes back when you stop.

Default is **AP mode** (self-contained, no network needed):

1. Leave the device in standby (not jamming). It hosts Wi-Fi AP **`SpecterOS-OTA`** (password `specter2024`).
2. Connect your computer to that AP.
3. In Arduino IDE the device appears as a network port **`specteros`** (or flash to `192.168.4.1`); OTA upload password is `specter`.

To join your own network instead, set `OTA_AP_MODE 0` and your `OTA_STA_SSID` / `OTA_STA_PASS` in `config.h`. All OTA settings (SSID, passwords, hostname) live in the **OTA** section of `config.h`; set `ENABLE_OTA 0` to disable it entirely (e.g. for lowest-power field use).

> OTA and the Wi-Fi beacon flood both use the single ESP32 Wi-Fi radio, so only one can be enabled — the build enforces this with a compile-time error.
