/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   channels.h — operation modes, menu labels & NRF24 channel tables
   ═══════════════════════════════════════════════════════════ */

#ifndef CHANNELS_H
#define CHANNELS_H

#include <Arduino.h>

// ──────────────────────────────────────────────
//  Operation Modes  (index into kMenuLabels + channel tables)
// ──────────────────────────────────────────────
enum OperationMode {
  WiFi_MODULE,
  BLE_MODULE,
  Bluetooth_MODULE,
  ZIGBEE_MODULE,
  VIDEO_TX_MODULE,
  RC_MODULE,
  USB_WIRELESS_MODULE,
  NRF24_MODULE,
  FULL_BAND_MODULE,
  MODE_COUNT
};

// Human-readable labels, one per mode (excludes the sentinel).
extern const char* const kMenuLabels[];
extern const int         kMenuCount;

// ──────────────────────────────────────────────
//  Channel-table lookup
//    Returns the hop table for `mode` and writes its length to *len.
//    freq = 2400 + channel (MHz).
// ──────────────────────────────────────────────
const byte* channelTableFor(OperationMode mode, size_t* len);

#endif // CHANNELS_H
