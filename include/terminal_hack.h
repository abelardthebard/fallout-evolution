#ifndef GUARD_TERMINAL_HACK_H
#define GUARD_TERMINAL_HACK_H

#include "constants/terminal_hack.h"

// Script contract:
//   setvar VAR_0x8000, <tier>
//   setvar VAR_0x8001, <flag_id>
//   special StartTerminalHack
//   waitstate
//   compare VAR_RESULT, 1    @ 1 = cracked, 0 = failed
//
// IsTerminalLockedOut (via specialvar): reads VAR_0x8000 = flag_id.
void StartTerminalHack(void);
void IsTerminalLockedOut(void);

bool8 IsTerminalWordBurned(u16 wordId);
void BurnTerminalWord(u16 wordId);

// Call from TryStartStepBasedScript on each real step.
void UpdateTerminalLockoutCounter(void);

#endif // GUARD_TERMINAL_HACK_H
