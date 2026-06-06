#ifndef GUARD_EMERGENCY_LIGHTING_H
#define GUARD_EMERGENCY_LIGHTING_H

// Field specials (driven from scripts via `special` + `waitstate`).
// Each runs the power-flicker task, then toggles FLAG_EMERGENCY_LIGHTING and
// refreshes the overworld palettes before handing control back to the script.
void StartPowerOutage(void);   // flicker to black, then emergency-red ON
void StartPowerRestore(void);  // flicker, then emergency-red OFF

#endif // GUARD_EMERGENCY_LIGHTING_H
