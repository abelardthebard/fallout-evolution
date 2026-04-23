#ifndef GUARD_TERMINAL_UI_H
#define GUARD_TERMINAL_UI_H

#include "bg.h"
#include "window.h"

// BG / window slot assignments shared by all terminal screens.
#define T_BG_TEXT            0   // top-priority BG: text windows
#define T_BG_IMAGE           1   // background BG: decorative chrome
#define T_WIN_TEXT           0

// Shared resource declarations. Terminal screens reference these rather
// than declaring their own BG / window templates.
extern const struct BgTemplate gTerminalBgTemplates[2];
extern const struct WindowTemplate gTerminalWindowTemplates[2];
extern const u8 gTerminalTextColors[];          // { transparent bg, theme fg, theme shadow }
extern const u8 gTerminalTextColors_Inverted[]; // for highlighted cells

// CB2 setup helpers. Call order for a screen's initialization:
//   TerminalUI_InitBgsAndWindows();
//   TerminalUI_LoadPalettes();     // text palette + bg image palette (themed)
//   TerminalUI_LoadBgImage();      // bg image tiles + tilemap (themed via palette remap)
void TerminalUI_InitBgsAndWindows(void);
void TerminalUI_LoadPalettes(void);
void TerminalUI_LoadBgImage(void);

// Render helpers
void TerminalUI_DrawCenteredHeader(u8 winId, const u8 *text);

#endif // GUARD_TERMINAL_UI_H
