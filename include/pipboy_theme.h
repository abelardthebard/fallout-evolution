#ifndef GUARD_PIPBOY_THEME_H
#define GUARD_PIPBOY_THEME_H

#include "constants/pipboy_theme.h"

struct TilesPal
{
    const u8 *tiles;
    const u16 *pal;
};

struct PipTheme
{
    const u16 *framePal;
    const u16 *textPal;
    u16 mainMenuFg;
    u16 mainMenuShadow;
};

extern const struct PipTheme gPipBoyThemes[THEME_COUNT];
extern const u16 *const gPipBoyGradients[THEME_COUNT];

// save_failed_screen hardcodes to green; theme setting may be corrupt.
extern const u16 gPipBoyFramePalGreen[];

u8 GetActiveTheme(void);
const u16 *GetActiveThemeTextPal(void);
const u16 *GetActiveThemeFramePal(void);
const struct TilesPal *GetWindowFrameTilesPal(u8 id);

void PipBoy_ApplyThemeToPalettes(u16 bgStart, u16 bgCount, u16 objStart, u16 objCount);
void PipBoy_LoadThemeGradient(u16 destPalBase);
void PipBoy_LoadSpotlight(u8 charBase, u8 mapBase);
void PipBoy_LoadTerminalSpotlight(u8 charBase, u8 mapBase);

#endif // GUARD_PIPBOY_THEME_H
