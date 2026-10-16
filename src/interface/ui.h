/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   ui.h — OLED status screen, button controls & LED heartbeat
   ═══════════════════════════════════════════════════════════ */

#ifndef UI_H
#define UI_H

// Bring up the OLED, button pins and status LED (call once in setup).
void uiInit();

// Show the boot splash screen.
void uiSplash();

// Per-loop UI work: poll buttons, pulse the LED, redraw if invalidated.
void uiTick();

// Request an OLED redraw on the next uiTick() (called by jammer actions).
void uiInvalidate();

#endif // UI_H
