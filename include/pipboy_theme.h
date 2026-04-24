#ifndef GUARD_PIPBOY_THEME_H
#define GUARD_PIPBOY_THEME_H

#include "constants/pipboy_theme.h"

// -------------------------------------------------------------------------
// Types
// -------------------------------------------------------------------------

// Paired tileset + palette for a themed window frame (one entry per theme
// in the theme-indexed sWindowFrames[] table in pipboy_theme.c).
struct TilesPal
{
    const u8 *tiles;
    const u16 *pal;
};

// Per-theme appearance data. Indexed by THEME_GREEN / BLUE / RED / YELLOW.
struct PipTheme
{
    const u16 *framePal;        // window-frame palette (goes to slot 2 / 7)
    const u16 *textPal;         // text-window palette (goes to slot 14 / 15)
    u16 mainMenuFg;             // main-menu foreground RGB
    u16 mainMenuShadow;         // main-menu shadow RGB
};

// -------------------------------------------------------------------------
// Public data
// -------------------------------------------------------------------------

// Theme table. Indexed by THEME_*. Read-only.
extern const struct PipTheme gPipBoyThemes[THEME_COUNT];

// 8-color Pip-Boy gradient for each theme. Indexed by THEME_*.
// Consumers: PipBoy_LoadThemeGradient() and the Birch-intro spotlight fade.
extern const u16 *const gPipBoyGradients[THEME_COUNT];

// Green-theme window-frame palette. Exposed because save_failed_screen.c
// hardcodes to the green frame (intentional — save failure is not allowed
// to depend on a possibly-corrupt save-block's theme setting).
extern const u16 gPipBoyFramePalGreen[];

// -------------------------------------------------------------------------
// Active-theme queries. Every call reads gSaveBlock2Ptr fresh, so features
// that call these inside their CB2 setup automatically pick up live theme
// changes (see design/palette_slot_contract.md).
// -------------------------------------------------------------------------

u8 GetActiveTheme(void);
const u16 *GetActiveThemeTextPal(void);
const u16 *GetActiveThemeFramePal(void);

// Theme-indexed frame (tile + palette) lookup.
const struct TilesPal *GetWindowFrameTilesPal(u8 id);

// -------------------------------------------------------------------------
// Theme application
// -------------------------------------------------------------------------

// Scan a palette-buffer range (both unfaded and faded buffers) and remap
// any color matching the green ramp to its active-theme equivalent.
// Works on palette *buffer* indices (0..511 for BG+OBJ), not slot numbers.
void PipBoy_ApplyThemeToPalettes(u16 bgStart, u16 bgCount, u16 objStart, u16 objCount);

// Load the active theme's 8-shade gradient into palette positions 1..8
// starting at destPalBase (typically BG_PLTT_ID(3) per the slot contract).
// Callers ensure the tilemap for the feature references palette slot 3.
void PipBoy_LoadThemeGradient(u16 destPalBase);

// Load the shared spotlight graphic (tile art + tilemap + theme backdrop
// palette at slot 3) onto the BG whose charBase / mapBase are supplied.
// The caller is responsible for configuring that BG's template to match,
// showing the BG layer, and picking a charBase / mapBase that doesn't
// collide with other BGs in use on the same screen.
void PipBoy_LoadSpotlight(u8 charBase, u8 mapBase);

#endif // GUARD_PIPBOY_THEME_H
