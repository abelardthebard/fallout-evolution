#ifndef GUARD_TEXT_WINDOW_H
#define GUARD_TEXT_WINDOW_H

#define WINDOW_FRAMES_COUNT 4
#define THEME_COUNT 4
#define PIPBOY_RAMP_SIZE 8

enum {
    THEME_GREEN,
    THEME_BLUE,
    THEME_RED,
    THEME_YELLOW,
};

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

extern const u8 gTextWindowFrame1_Gfx[];
extern const u16 gTextWindowFrame1_Pal[];
extern const struct PipTheme gPipThemes[THEME_COUNT];
extern const u16 gPipBoyGreenRamp[PIPBOY_RAMP_SIZE];
extern const u16 gPipBoyThemeRamps[THEME_COUNT][PIPBOY_RAMP_SIZE];
void PipBoy_ApplyThemeToPalettes(u16 bgStart, u16 bgCount, u16 objStart, u16 objCount);
extern const u16 gHairPalettes[][3];
extern const u16 gSkinPalettes[][3];
void ApplyPlayerAppearancePalette(u8 paletteSlot);

const struct TilesPal *GetWindowFrameTilesPal(u8 id);
void LoadMessageBoxGfx(u8 windowId, u16 destOffset, u8 palOffset);
void LoadStdWindowGfx(u8 windowId, u16 tileStart, u8 palette);
void LoadSignBoxGfx(u8 windowId, u16 destOffset, u8 palOffset);
void LoadWindowGfx(u8 windowId, u8 frameId, u16 destOffset, u8 palOffset);
void LoadUserWindowBorderGfx(u8 windowId, u16 destOffset, u8 palOffset);
void LoadUserWindowBorderGfx_(u8 windowId, u16 destOffset, u8 palOffset);
void LoadUserWindowBorderGfxOnBg(u8 bg, u16 destOffset, u8 palOffset);
void DrawTextBorderOuter(u8 windowId, u16 tileNum, u8 palNum);
void DrawTextBorderInner(u8 windowId, u16 tileNum, u8 palNum);
void rbox_fill_rectangle(u8 windowId);
const u16 *GetTextWindowPalette(u8 id);
const u16 *GetOverworldTextboxPalettePtr(void);
u8 GetActiveTheme(void);
const u16 *GetActiveThemeTextPal(void);
const u16 *GetActiveThemeFramePal(void);
void LoadSignPostWindowFrameGfx(void);
void LoadDexNavWindowGfx(u8 windowId, u16 destOffset, u8 palOffset);

#endif // GUARD_TEXT_WINDOW_H
