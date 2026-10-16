/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   channels.cpp — channel-table definitions & mode lookup
   ═══════════════════════════════════════════════════════════ */

#include "channels.h"

const char* const kMenuLabels[] = {
  "WiFi", "BLE", "Bluetooth", "Zigbee",
  "Video TX", "RC", "USB Wireless", "NRF24", "Full Band"
};
const int kMenuCount = sizeof(kMenuLabels) / sizeof(kMenuLabels[0]);

// Concentrated on the three non-overlapping Wi-Fi channels (1/6/11), where
// real APs and clients live, instead of spreading thin across the whole band.
static const byte WiFi_channels[] = {
  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22,   // Wi-Fi ch 1  (2401-2423 MHz)
  27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47,   // Wi-Fi ch 6  (2426-2448 MHz)
  52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72    // Wi-Fi ch 11 (2451-2473 MHz)
};

static const byte ble_channels[] = {
  2, 26, 80,
  4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24,
  28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50,
  52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78
};

static const byte bluetooth_channels[] = {
  0, 1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
  32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60,
  62, 64, 66, 68, 70, 72, 74, 76, 78, 80
};

static const byte zigbee_channels[]           = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80};
static const byte videoTransmitter_channels[] = {60, 65, 70, 75, 80, 83};
static const byte rc_channels[]               = {2, 8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80};
static const byte usbWireless_channels[]      = {30, 35, 40, 45, 50, 55, 60, 65, 70};
static const byte nrf24_channels[]            = {70, 72, 74, 76, 78, 79, 80, 82, 84};

static const byte fullband_channels[] = {
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
  64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84
};

const byte* channelTableFor(OperationMode mode, size_t* len) {
  switch (mode) {
    case WiFi_MODULE:         *len = sizeof(WiFi_channels);             return WiFi_channels;
    case BLE_MODULE:          *len = sizeof(ble_channels);              return ble_channels;
    case Bluetooth_MODULE:    *len = sizeof(bluetooth_channels);        return bluetooth_channels;
    case ZIGBEE_MODULE:       *len = sizeof(zigbee_channels);           return zigbee_channels;
    case VIDEO_TX_MODULE:     *len = sizeof(videoTransmitter_channels); return videoTransmitter_channels;
    case RC_MODULE:           *len = sizeof(rc_channels);               return rc_channels;
    case USB_WIRELESS_MODULE: *len = sizeof(usbWireless_channels);      return usbWireless_channels;
    case NRF24_MODULE:        *len = sizeof(nrf24_channels);            return nrf24_channels;
    case FULL_BAND_MODULE:    *len = sizeof(fullband_channels);         return fullband_channels;
    default:                  *len = sizeof(fullband_channels);         return fullband_channels;
  }
}
