#ifndef GUARD_TERMINAL_UI_H
#define GUARD_TERMINAL_UI_H

// BG / window slot assignments shared by all terminal screens.
#define T_BG_TEXT            0   // top-priority BG: text windows
#define T_BG_SPOTLIGHT       1   // themed Pip-Boy spotlight (PipBoy_LoadSpotlight)
#define T_BG_IMAGE           2   // decorative terminal chrome (behind spotlight)
#define T_WIN_TEXT           0

// BG / window templates are private to terminal_ui.c; terminal screens
// drive initialization through TerminalUI_InitBgsAndWindows below.
extern const u8 gTerminalTextColors[];          // { transparent bg, theme fg, theme shadow }
extern const u8 gTerminalTextColors_Inverted[]; // for highlighted cells

// CB2 setup helpers. Call order for a screen's initialization:
//   TerminalUI_InitBgsAndWindows();
//   TerminalUI_LoadPalettes();     // text palette + bg image palette (themed)
//   TerminalUI_LoadBgImage();      // bg image tiles + tilemap (themed via palette remap)
//   TerminalUI_LoadSpotlight();    // optional: themed Pip-Boy spotlight on BG 1
void TerminalUI_InitBgsAndWindows(void);
void TerminalUI_LoadPalettes(void);
void TerminalUI_LoadBgImage(void);
void TerminalUI_LoadSpotlight(void);

// Caller-specified spotlight placement: WIN0 clip rect + BG 1 scroll offsets.
// The clip rect masks where BG 1 (spotlight) renders; outside the rect, the
// spotlight is suppressed so chrome (BG 2) shows instead. Scroll offsets
// translate the Birch tilemap's native coords into the terminal's screen.
struct SpotlightLayout
{
    u8  clipLeft;
    u8  clipTop;
    u8  clipWidth;
    u8  clipHeight;
    u16 scrollX;      // written to BG1HOFS (9-bit pixel offset, wraps at 512)
    u16 scrollY;      // written to BG1VOFS
};

// Configure hardware windowing and BG 1 scroll so the spotlight clips to
// the supplied rect. Enables DISPCNT_WIN0_ON. Call this from a page's
// createSprites hook after TerminalUI_LoadSpotlight() has populated BG 1.
void TerminalUI_ShowSpotlight(const struct SpotlightLayout *layout);

// Reverse TerminalUI_ShowSpotlight: disable WIN0, zero the window/scroll
// registers. Called from TerminalContent_Teardown; harmless if the page
// never called ShowSpotlight.
void TerminalUI_HideSpotlight(void);

// Render helpers
void TerminalUI_DrawCenteredHeader(u8 winId, const u8 *text);

#endif // GUARD_TERMINAL_UI_H
