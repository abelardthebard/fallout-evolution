#ifndef GUARD_TERMINAL_H
#define GUARD_TERMINAL_H

#include "constants/terminal_hack.h"
#include "constants/terminal_content.h"

// Script: VAR_0x8000=tier, VAR_0x8001=flagId, VAR_0x8002=content_id (chained on win).
void StartTerminalHack(void);

// Called via `specialvar VAR_RESULT, IsTerminalLockedOut` -- script reads
// the return value, NOT VAR_RESULT side-effects. Script: VAR_0x8000=flagId.
bool8 IsTerminalLockedOut(void);

// Script: VAR_0x8000=content_id.
void ShowTerminalContent(void);

// Step-driven from TryStartStepBasedScript; decrements VAR_TERMINAL_LOCKOUT_STEPS.
void UpdateTerminalLockoutCounter(void);

bool8 IsTerminalWordBurned(u16 wordId);
void BurnTerminalWord(u16 wordId);

#endif // GUARD_TERMINAL_H
