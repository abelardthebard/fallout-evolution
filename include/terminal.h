#ifndef GUARD_TERMINAL_H
#define GUARD_TERMINAL_H

#include "constants/terminal_hack.h"
#include "constants/terminal_content.h"

void StartTerminalHack(void);
bool8 IsTerminalLockedOut(void);
void ShowTerminalContent(void);
void UpdateTerminalLockoutCounter(void);

bool8 IsTerminalWordBurned(u16 wordId);
void BurnTerminalWord(u16 wordId);

#endif // GUARD_TERMINAL_H
