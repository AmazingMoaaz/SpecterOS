/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   radio.cpp — NRF24L01 carrier mode, dual-SPI, burst hopping
   ═══════════════════════════════════════════════════════════ */

#include "radio.h"
#include "../config.h"
#include "channels.h"
#include "../jammer/jammer.h"
#include <esp_random.h>
#include "soc/gpio_reg.h"

// Fast CE toggle — direct GPIO registers instead of digitalWrite (~20x faster),
// which lets the carrier retune and hop far more often. CE pins are both < 32.
#define CE_LOW(pin)   REG_WRITE(GPIO_OUT_W1TC_REG, 1u << (pin))
#define CE_HIGH(pin)  REG_WRITE(GPIO_OUT_W1TS_REG, 1u << (pin))

static SPIClass hspi(HSPI);

RF24 RadioA(NRF_CE_PIN_A, NRF_CSN_PIN_A);
RF24 RadioB(NRF_CE_PIN_B, NRF_CSN_PIN_B);

volatile bool radioA_ok = false;
volatile bool radioB_ok = false;

static const byte* cachedTableA = nullptr;
static size_t      cachedLenA   = 0;
static OperationMode cachedModeA = (OperationMode)-1;

static const byte* cachedTableB = nullptr;
static size_t      cachedLenB   = 0;
static OperationMode cachedModeB = (OperationMode)-1;

static void configureNrf(RF24 &radio) {
  radio.setAutoAck(false);
  radio.stopListening();
  radio.setRetries(0, 0);
  radio.setPALevel(RF24_PA_MAX, true);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
}

void radioInit() {
  RadioA.begin();
  configureNrf(RadioA);

  hspi.begin(HSPI_SCK_PIN, HSPI_MISO_PIN, HSPI_MOSI_PIN);
  RadioB.begin(&hspi);
  configureNrf(RadioB);
}

void radioNeutral() {
  RadioA.stopListening();
  RadioA.setAutoAck(false);
  RadioA.setRetries(0, 0);
  RadioA.powerDown();
  digitalWrite(NRF_CE_PIN_A, LOW);

  RadioB.stopListening();
  RadioB.setAutoAck(false);
  RadioB.setRetries(0, 0);
  RadioB.powerDown();
  digitalWrite(NRF_CE_PIN_B, LOW);
}

void radioActivate() {
  radioA_ok = RadioA.begin();
  if (radioA_ok) {
    configureNrf(RadioA);
    RadioA.startConstCarrier(RF24_PA_MAX, 2);   // enter carrier mode; hop loop retunes it
    Serial.println(F("[+] RadioA  OK  (VSPI)"));
  } else {
    Serial.println(F("[!] RadioA  FAIL"));
  }

  radioB_ok = RadioB.begin(&hspi);
  if (radioB_ok) {
    configureNrf(RadioB);
    RadioB.startConstCarrier(RF24_PA_MAX, 80);  // enter carrier mode; hop loop retunes it
    Serial.println(F("[+] RadioB  OK  (HSPI)"));
  } else {
    Serial.println(F("[!] RadioB  FAIL"));
  }
}

void radioDeactivate() {
  RadioA.powerDown();
  RadioB.powerDown();
  delay(50);
  Serial.println(F("[~] Radios powered down"));
}

// ── Per-radio hop (each only touches its own SPI bus) ────────

void radioHopA() {
  OperationMode m = current_Mode;
  if (m != cachedModeA) {
    cachedTableA = channelTableFor(m, &cachedLenA);
    cachedModeA  = m;
  }
  byte ch = cachedTableA[esp_random() % cachedLenA];
  CE_LOW(NRF_CE_PIN_A);
  RadioA.setChannel(ch);
  CE_HIGH(NRF_CE_PIN_A);
}

void radioHopB() {
  OperationMode m = current_Mode;
  if (m != cachedModeB) {
    cachedTableB = channelTableFor(m, &cachedLenB);
    cachedModeB  = m;
  }
  byte ch = cachedTableB[esp_random() % cachedLenB];
  CE_LOW(NRF_CE_PIN_B);
  RadioB.setChannel(ch);
  CE_HIGH(NRF_CE_PIN_B);
}

