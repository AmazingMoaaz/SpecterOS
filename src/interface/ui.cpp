/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   ui.cpp — OLED UI with live stats, button controls & LED
   ═══════════════════════════════════════════════════════════ */

#include "ui.h"
#include "../config.h"
#include "../jammer/jammer.h"
#include "../radio/radio.h"

#include <Wire.h>
#include <U8g2lib.h>

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

static bool uiDirty = true;

// ── Hop rate tracking ───────────────────────────────────────
static uint32_t lastHopsA = 0, lastHopsB = 0, lastHopsW = 0;
static unsigned long lastStatTime = 0;
static uint32_t hpsA = 0, hpsB = 0, hpsW = 0;
static uint32_t hpsTotal = 0;

static void updateStats() {
  unsigned long now = millis();
  if (now - lastStatTime >= 1000) {
    hpsA = hopsA - lastHopsA;
    hpsB = hopsB - lastHopsB;
    hpsW = hopsW - lastHopsW;
    hpsTotal = hpsA + hpsB + hpsW;
    lastHopsA = hopsA;
    lastHopsB = hopsB;
    lastHopsW = hopsW;
    lastStatTime = now;
    if (current == ACTIVE_MODE) uiDirty = true;
  }
}

// ── Drawing helpers ─────────────────────────────────────────
static void drawCenteredStr(const char* s, int y) {
  int w = u8g2.getStrWidth(s);
  u8g2.drawStr((128 - w) / 2, y, s);
}

static void formatRate(char* buf, size_t sz, uint32_t hps) {
  if (hps >= 1000000)
    snprintf(buf, sz, "%lu.%luM", hps / 1000000, (hps % 1000000) / 100000);
  else if (hps >= 1000)
    snprintf(buf, sz, "%lu.%luK", hps / 1000, (hps % 1000) / 100);
  else
    snprintf(buf, sz, "%lu", hps);
}

static void drawTower(int cx, int baseY, int waves) {
  int tipY = baseY - 20;
  u8g2.drawLine(cx - 7, baseY, cx, tipY);
  u8g2.drawLine(cx + 7, baseY, cx, tipY);
  u8g2.drawLine(cx - 4, baseY - 9, cx + 4, baseY - 9);
  u8g2.drawDisc(cx, tipY, 2);
  for (int i = 1; i <= waves; i++) {
    int r = 3 + i * 4;
    u8g2.drawCircle(cx, tipY, r, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  }
}

// ── Boot splash ─────────────────────────────────────────────
void uiSplash() {
  for (int w = 8; w <= 136; w += 12) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    drawCenteredStr(OS_NAME, 38);
    u8g2.setDrawColor(0);
    u8g2.drawBox(w, 0, 128 - w, 64);
    u8g2.setDrawColor(1);
    u8g2.sendBuffer();
    delay(35);
  }

  for (int r = 0; r <= 3; r++) {
    u8g2.clearBuffer();
    drawTower(22, 48, r);
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(46, 30, OS_NAME);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(46, 46, "version 2");
    u8g2.sendBuffer();
    delay(170);
  }
  delay(200);

  const char* steps[] = { "init VSPI radio", "init HSPI radio",
                          "free bluetooth", "arm carriers",
                          "dual-core start", "system ready" };
  const int nSteps = sizeof(steps) / sizeof(steps[0]);
  for (int i = 0; i < nSteps; i++) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, OS_NAME " boot");
    u8g2.drawHLine(0, 13, 128);
    u8g2.setFont(u8g2_font_5x8_tf);
    for (int j = 0; j <= i; j++) {
      int y = 24 + j * 7;
      u8g2.drawStr(0, y, ">");
      u8g2.drawStr(8, y, steps[j]);
      if (j < i) u8g2.drawStr(112, y, "ok");
    }
    int pct = (i + 1) * 100 / nSteps;
    u8g2.drawFrame(0, 58, 128, 6);
    u8g2.drawBox(2, 60, (124 * pct) / 100, 2);
    u8g2.sendBuffer();
    delay(250);
  }
  delay(100);

  for (int f = 0; f < 4; f++) {
    u8g2.clearBuffer();
    bool inv = (f % 2 == 0);
    if (inv) u8g2.drawBox(0, 0, 128, 64);
    u8g2.setDrawColor(inv ? 0 : 1);
    u8g2.setFont(u8g2_font_ncenB14_tr);
    drawCenteredStr("READY", 40);
    u8g2.setDrawColor(1);
    u8g2.sendBuffer();
    delay(110);
  }
}

// ── Live status screen ──────────────────────────────────────
static void uiDraw() {
  const bool active = (current == ACTIVE_MODE);

  u8g2.clearBuffer();

  // ── Header bar (y 0-12) ──
  u8g2.drawBox(0, 0, 128, 12);
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(2, 10, OS_NAME);
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(60, 9, OS_VERSION);

  // 3 radio status dots in header
  int dx = 74;
  // A
  if (radioA_ok) u8g2.drawDisc(dx, 6, 2);
  else           u8g2.drawCircle(dx, 6, 2);
  // B
  if (radioB_ok) u8g2.drawDisc(dx + 10, 6, 2);
  else           u8g2.drawCircle(dx + 10, 6, 2);
  // W (WiFi)
  if (wifiFloodOn && active) u8g2.drawDisc(dx + 20, 6, 2);
  else                       u8g2.drawCircle(dx + 20, 6, 2);

  // Uptime in header (right-aligned; shows hours once past 1h)
  unsigned long sec = millis() / 1000;
  char uptxt[12];
  if (sec < 3600) snprintf(uptxt, sizeof(uptxt), "%02lu:%02lu", sec / 60, sec % 60);
  else            snprintf(uptxt, sizeof(uptxt), "%luh%02lu", sec / 3600, (sec / 60) % 60);
  u8g2.drawStr(126 - u8g2.getStrWidth(uptxt), 9, uptxt);

  u8g2.setDrawColor(1);

  // ── Mode name (y 14-28) ──
  u8g2.setFont(u8g2_font_ncenB14_tr);
  drawCenteredStr(kMenuLabels[(int)current_Mode], 28);

  // ── Mode index (y 30-37) ──
  char sub[20];
  snprintf(sub, sizeof(sub), "mode %d/%d", (int)current_Mode + 1, kMenuCount);
  u8g2.setFont(u8g2_font_5x8_tf);
  drawCenteredStr(sub, 38);

  if (!active) {
    // ── STANDBY ──
    u8g2.setFont(u8g2_font_7x14B_tr);
    drawCenteredStr("STANDBY", 56);
  } else {
    // ── Hop rate (y 42-50) ──
    char rate[24];
    char rateStr[12];
    formatRate(rateStr, sizeof(rateStr), hpsTotal);
    snprintf(rate, sizeof(rate), "%s hops/s", rateStr);
    u8g2.setFont(u8g2_font_6x12_tf);
    drawCenteredStr(rate, 50);

    // ── Status bar (y 54-64) ──
    u8g2.drawBox(0, 54, 128, 10);
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_6x12_tf);
    drawCenteredStr("ACTIVE", 63);
    u8g2.setDrawColor(1);
  }

  u8g2.sendBuffer();
}

// ── Button control ──────────────────────────────────────────
struct Button {
  uint8_t pin;
  void (*action)();
  bool          lastReading;
  unsigned long lastEdge;
};

static Button buttons[] = {
  { BTN_ACTIVATE_PIN, jammerToggle,   HIGH, 0 },
  { BTN_MODE_PIN,     jammerNextMode, HIGH, 0 },
  { BTN_PREV_PIN,     jammerPrevMode, HIGH, 0 },
};
static const size_t        kButtonCount = sizeof(buttons) / sizeof(buttons[0]);
static const unsigned long kDebounceMs  = 200;

static void buttonsInit() {
  for (size_t i = 0; i < kButtonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    buttons[i].lastReading = HIGH;
  }
}

static void pollButtons() {
  unsigned long now = millis();
  for (size_t i = 0; i < kButtonCount; i++) {
    bool reading = digitalRead(buttons[i].pin);
    if (buttons[i].lastReading == HIGH && reading == LOW &&
        (now - buttons[i].lastEdge) > kDebounceMs) {
      buttons[i].lastEdge = now;
      buttons[i].action();
    }
    buttons[i].lastReading = reading;
  }
}

// ── LED heartbeat ───────────────────────────────────────────
static unsigned long ledLastToggle = 0;
static bool          ledState      = false;

static void heartbeatTick() {
  if (current != ACTIVE_MODE) {
    if (ledState) { digitalWrite(PIN_LED, LOW); ledState = false; }
    return;
  }
  unsigned long now = millis();
  if (now - ledLastToggle >= 100) {
    ledLastToggle = now;
    ledState = !ledState;
    digitalWrite(PIN_LED, ledState ? HIGH : LOW);
  }
}

// ── Public API ──────────────────────────────────────────────
void uiInit() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  buttonsInit();
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  u8g2.setBusClock(400000);
  u8g2.begin();
  uiSplash();
}

void uiInvalidate() {
  uiDirty = true;
}

void uiTick() {
  pollButtons();
  heartbeatTick();
  updateStats();

  if (uiDirty) {
    uiDraw();
    uiDirty = false;
  }
}
