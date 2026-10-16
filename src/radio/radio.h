/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   radio.h — NRF24L01 hardware abstraction & carrier control
   ═══════════════════════════════════════════════════════════ */

#ifndef RADIO_H
#define RADIO_H

#include <RF24.h>
#include <SPI.h>

extern RF24 RadioA;
extern RF24 RadioB;

extern volatile bool radioA_ok;
extern volatile bool radioB_ok;

void radioInit();
void radioNeutral();

void radioActivate();
void radioDeactivate();

// Per-radio hop functions (thread-safe, each touches only its own SPI bus)
void radioHopA();
void radioHopB();

#endif // RADIO_H
