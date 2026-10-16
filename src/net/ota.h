/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   ota.h — wireless firmware updates (standby only)
   ═══════════════════════════════════════════════════════════ */

#ifndef OTA_H
#define OTA_H

void otaInit();   // call once in setup()
void otaTick();   // call every loop(): OTA is served while idle, Wi-Fi is
                  // dropped while jamming (saves power, frees the ESP32 radio)

#endif // OTA_H
