#ifndef GUARD_TERMINAL_UI_H
#define GUARD_TERMINAL_UI_H

// BG / window slot assignments shared by all terminal screens.
#define T_BG_TEXT            0   // top-priority BG: text windows
#define T_BG_IMAGE           1   // background BG: decorative chrome
#define T_WIN_TEXT           0

// BG / window templates are private to terminal_ui.c; terminal screens
// drive initialization through TerminalUI_InitBgsAndWindows below.
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
