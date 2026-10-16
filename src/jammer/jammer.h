/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   jammer.h — state machine, parallel tasks, optional WiFi flood
   ═══════════════════════════════════════════════════════════ */

#ifndef JAMMER_H
#define JAMMER_H

#include "../radio/channels.h"

enum Operation { DEACTIVE_MODE, ACTIVE_MODE };

extern volatile OperationMode current_Mode;
extern volatile Operation current;
extern bool               wifiFloodOn;

extern volatile uint32_t hopsA;
extern volatile uint32_t hopsB;
extern volatile uint32_t hopsW;

void jammerToggle();     // Button 1: ON / OFF
void jammerNextMode();   // Button 2: cycle to next mode
void jammerPrevMode();   // Button 3: cycle to previous mode

void jammerStart();

#endif // JAMMER_H
