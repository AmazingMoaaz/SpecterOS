/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   cli.cpp — serial command interface
   ═══════════════════════════════════════════════════════════ */

#include "cli.h"
#include "../config.h"
#include "../jammer/jammer.h"

void cliStatus() {
  Serial.println(F("──────────────────────────────────────"));
  Serial.print  (F("  Mode   : ")); Serial.println(kMenuLabels[(int)current_Mode]);
  Serial.print  (F("  Status : ")); Serial.println(current == ACTIVE_MODE ? "ACTIVE" : "OFF");
  Serial.println(F("──────────────────────────────────────"));
}

void cliHelp() {
  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════╗"));
  Serial.println(F("║   " FW_NAME " " FW_VERSION "  —  COMMANDS             ║"));
  Serial.println(F("╠══════════════════════════════════════════╣"));
  Serial.println(F("║  t  — Toggle ON / OFF                    ║"));
  Serial.println(F("║  n  — Next mode                          ║"));
  Serial.println(F("║  p  — Previous mode                      ║"));
  Serial.println(F("║  s  — Show status                        ║"));
  Serial.println(F("║  ?  — This help screen                   ║"));
  Serial.println(F("╚══════════════════════════════════════════╝"));
  Serial.println();
}

void cliBanner() {
  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════════╗"));
  Serial.println(F("║        S P E C T E R O S   " FW_VERSION "             ║"));
  Serial.println(F("║  2x NRF24+PA+LNA  (VSPI + HSPI)         ║"));
  Serial.println(F("║  Dual-Core  •  Burst Hop  •  2 Radios   ║"));
  Serial.println(F("╚══════════════════════════════════════════════╝"));
  cliHelp();
  cliStatus();
}

void cliTick() {
  if (!Serial.available()) return;
  switch (Serial.read()) {
    case 't': case 'T': jammerToggle();   break;
    case 'n': case 'N': jammerNextMode(); break;
    case 'p': case 'P': jammerPrevMode(); break;
    case 's': case 'S': cliStatus();      break;
    case '?':           cliHelp();        break;
  }
}
