/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer

   Dual-radio architecture:
     NRF24L01 #1 (VSPI)  — burst hopping on Core 0
     NRF24L01 #2 (HSPI)  — burst hopping on Core 1
     ESP32 WiFi radio     — optional beacon flood (ENABLE_WIFI_FLOOD)

   Hardware:
     ESP32-WROOM-32 (38-pin DevKit)
     NRF24L01+PA+LNA #1  CE=4  CSN=5   (VSPI: SCK=18 MISO=19 MOSI=23)
     NRF24L01+PA+LNA #2  CE=16 CSN=17  (HSPI: SCK=14 MISO=12 MOSI=13)
     SSD1306 OLED  SDA=21 SCL=22
     BTN Activate=25  BTN Next=26  BTN Prev=27
     Built-in LED  GPIO 2
   ═══════════════════════════════════════════════════════════ */

#include <esp_bt.h>

#include "src/config.h"
#include "src/radio/channels.h"
#include "src/radio/radio.h"
#include "src/jammer/jammer.h"
#include "src/interface/ui.h"
#include "src/interface/cli.h"
#include "src/net/ota.h"

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  uiInit();

  esp_bt_controller_deinit();
  esp_bt_mem_release(ESP_BT_MODE_BTDM);

  radioInit();
  radioNeutral();

  jammerStart();
  otaInit();

  cliBanner();
  uiInvalidate();
}

void loop() {
  cliTick();
  uiTick();
  otaTick();
}
