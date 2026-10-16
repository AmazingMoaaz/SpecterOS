/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   ota.cpp — ArduinoOTA over Wi-Fi, active only while in standby
   ═══════════════════════════════════════════════════════════ */

#include "ota.h"
#include "../config.h"

#if ENABLE_OTA

#include <WiFi.h>
#include <ArduinoOTA.h>
#include "../jammer/jammer.h"   // for `current` / ACTIVE_MODE

static bool wifiUp = false;

static void wifiStart() {
  if (wifiUp) return;

#if OTA_AP_MODE
  WiFi.mode(WIFI_AP);
  WiFi.softAP(OTA_AP_SSID, OTA_AP_PASS);
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin(OTA_STA_SSID, OTA_STA_PASS);
  for (uint32_t t0 = millis(); WiFi.status() != WL_CONNECTED && millis() - t0 < 8000; )
    delay(100);
#endif

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();
  wifiUp = true;

  Serial.print(F("[OTA] ready @ "));
#if OTA_AP_MODE
  Serial.print(WiFi.softAPIP());
  Serial.print(F("  (AP: "));
  Serial.print(OTA_AP_SSID);
  Serial.println(F(")"));
#else
  Serial.println(WiFi.localIP());
#endif
}

static void wifiStop() {
  if (!wifiUp) return;
  ArduinoOTA.end();
  WiFi.mode(WIFI_OFF);
  wifiUp = false;
  Serial.println(F("[OTA] off (jamming)"));
}

void otaInit() {
  wifiStart();
}

void otaTick() {
  if (current == ACTIVE_MODE) {
    wifiStop();          // no Wi-Fi while the radios run full tilt
    return;
  }
  if (!wifiUp) wifiStart();
  ArduinoOTA.handle();
}

#else  // ENABLE_OTA == 0

void otaInit() {}
void otaTick() {}

#endif
