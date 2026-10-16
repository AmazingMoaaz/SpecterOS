/* ═══════════════════════════════════════════════════════════
   SpecterOS v2 — ESP32 2.4 GHz RF Jammer
   cli.h — serial command interface (115200 baud)
   ═══════════════════════════════════════════════════════════ */

#ifndef CLI_H
#define CLI_H

void cliBanner();   // startup banner + help + status
void cliHelp();     // print the command help screen
void cliStatus();   // print the current mode / state
void cliTick();     // process one pending serial command (call each loop)

#endif // CLI_H
