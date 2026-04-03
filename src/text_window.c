#include "global.h"
#include "constants/rgb.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "palette.h"
#include "bg.h"
#include "graphics.h"
#include "menu.h"

static const u16 sStdTextWindow_Gfx[]  = INCBIN_U16("graphics/text_window/std.4bpp");

// All themes share the same tile art; only palettes differ
const u8 gTextWindowFrame1_Gfx[] = INCBIN_U8("graphics/text_window/1.4bpp");

// Per-theme frame border palettes
const u16 gTextWindowFrame1_Pal[]     = INCBIN_U16("graphics/text_window/pip0.gbapal");
static const u16 sThemeFramePal1[]    = INCBIN_U16("graphics/text_window/pip1.gbapal");
static const u16 sThemeFramePal2[]    = INCBIN_U16("graphics/text_window/pip2.gbapal");
static const u16 sThemeFramePal3[]    = INCBIN_U16("graphics/text_window/pip3.gbapal");

// Per-theme text window palettes
static const u16 sThemeTextPal0[]     = INCBIN_U16("graphics/text_window/piptext_pal0.gbapal");
static const u16 sThemeTextPal1[]     = INCBIN_U16("graphics/text_window/piptext_pal1.gbapal");
static const u16 sThemeTextPal2[]     = INCBIN_U16("graphics/text_window/piptext_pal2.gbapal");
static const u16 sThemeTextPal3[]     = INCBIN_U16("graphics/text_window/piptext_pal3.gbapal");

// Vanilla text palettes (used by signpost, naming screen, etc.)
static const u16 sTextWindowPalettes[][16] =
{
    INCBIN_U16("graphics/text_window/piptext_pal0.gbapal"),
    INCBIN_U16("graphics/text_window/text_pal1.gbapal"),
    INCBIN_U16("graphics/text_window/text_pal2.gbapal"),
    INCBIN_U16("graphics/text_window/text_pal3.gbapal"),
    INCBIN_U16("graphics/text_window/text_pal4.gbapal")
};

// Theme table — indexed by optionsWindowFrameType (0-3)
const struct PipTheme gPipThemes[THEME_COUNT] =
{
    [THEME_GREEN] = {
        .framePal     = gTextWindowFrame1_Pal,
        .textPal      = sThemeTextPal0,
        .mainMenuFg   = RGB(9, 25, 4),
        .mainMenuShadow = RGB(1, 8, 1),
    },
    [THEME_BLUE] = {
        .framePal     = sThemeFramePal1,
        .textPal      = sThemeTextPal1,
        .mainMenuFg   = RGB(9, 19, 28),
        .mainMenuShadow = RGB(3, 5, 10),
    },
    [THEME_RED] = {
        .framePal     = sThemeFramePal2,
        .textPal      = sThemeTextPal2,
        .mainMenuFg   = RGB(29, 8, 6),
        .mainMenuShadow = RGB(10, 2, 1),
    },
    [THEME_YELLOW] = {
        .framePal     = sThemeFramePal3,
        .textPal      = sThemeTextPal3,
        .mainMenuFg   = RGB(27, 25, 5),
        .mainMenuShadow = RGB(7, 6, 0),
    },
};

// PipBoy theme color ramps — 8 shades bright→dark per theme
// Green ramp is the base; palettes on disk use these values
const u16 gPipBoyGreenRamp[PIPBOY_RAMP_SIZE] = {
    RGB(24, 31, 15), // #C0F878
    RGB(19, 29, 12), // #98E860
    RGB(14, 27,  8), // #70D840
    RGB( 9, 25,  4), // #48C820
    RGB( 5, 21,  2), // #28A810
    RGB( 3, 16,  2), // #188010
    RGB( 2, 12,  1), // #106008
    RGB( 1,  8,  1), // #084008
};

const u16 gPipBoyThemeRamps[THEME_COUNT][PIPBOY_RAMP_SIZE] = {
    [THEME_GREEN] = {
        RGB(24, 31, 15), RGB(19, 29, 12), RGB(14, 27,  8), RGB( 9, 25,  4),
        RGB( 5, 21,  2), RGB( 3, 16,  2), RGB( 2, 12,  1), RGB( 1,  8,  1),
    },
    [THEME_BLUE] = {
        RGB(19, 29, 31), RGB(15, 26, 31), RGB(11, 23, 29), RGB( 9, 19, 28),
        RGB( 7, 15, 24), RGB( 6, 11, 19), RGB( 4,  8, 15), RGB( 3,  5, 10),
    },
    [THEME_RED] = {
        RGB(30, 16, 15), RGB(29, 12, 12), RGB(29, 10,  9), RGB(29,  8,  6),
        RGB(24,  6,  4), RGB(19,  4,  3), RGB(15,  3,  2), RGB(10,  2,  1),
    },
    [THEME_YELLOW] = {
        RGB(29, 28, 11), RGB(27, 25,  5), RGB(25, 21,  3), RGB(22, 18,  2),
        RGB(18, 15,  1), RGB(14, 12,  0), RGB(10,  9,  0), RGB( 7,  6,  0),
    },
};

void PipBoy_ApplyThemeToPalettes(u16 bgStart, u16 bgCount, u16 objStart, u16 objCount)
{
    u8 theme = GetActiveTheme();
    u16 i, j;

    if (theme == THEME_GREEN)
        return;

    for (i = bgStart; i < bgStart + bgCount; i++)
    {
        for (j = 0; j < PIPBOY_RAMP_SIZE; j++)
        {
            if (gPlttBufferUnfaded[i] == gPipBoyGreenRamp[j])
            {
                gPlttBufferUnfaded[i] = gPipBoyThemeRamps[theme][j];
                gPlttBufferFaded[i] = gPipBoyThemeRamps[theme][j];
                break;
            }
        }
    }

    for (i = objStart; i < objStart + objCount; i++)
    {
        for (j = 0; j < PIPBOY_RAMP_SIZE; j++)
        {
            if (gPlttBufferUnfaded[i] == gPipBoyGreenRamp[j])
            {
                gPlttBufferUnfaded[i] = gPipBoyThemeRamps[theme][j];
                gPlttBufferFaded[i] = gPipBoyThemeRamps[theme][j];
                break;
            }
        }
    }
}

// Player appearance palettes — shared across intro, naming screen, and overworld
const u16 gHairPalettes[][3] = {
    { RGB(29, 27, 18), RGB(25, 22, 13), RGB(16, 13,  7) }, // Blond
    { RGB(25, 22, 13), RGB(16, 13,  7), RGB(11,  9,  4) }, // Sandy
    { RGB(14, 12, 11), RGB(10,  8,  7), RGB( 7,  6,  5) }, // Brown
    { RGB(10,  8,  7), RGB( 7,  6,  5), RGB( 5,  3,  3) }, // Umber
    { RGB(27, 19, 12), RGB(22, 14,  7), RGB(16,  9,  4) }, // Ginger
    { RGB(22, 14,  7), RGB(16,  9,  4), RGB( 9,  5,  2) }, // Red
    { RGB( 8,  9, 11), RGB( 5,  6,  8), RGB( 3,  4,  6) }, // Black
    { RGB( 5,  6,  8), RGB( 3,  4,  6), RGB( 2,  3,  5) }, // Raven
    { RGB(31, 31, 31), RGB(19, 19, 19), RGB(12, 12, 12) }, // White
    { RGB(25, 25, 25), RGB(12, 12, 12), RGB( 7,  7,  7) }, // Ash
};

const u16 gSkinPalettes[][3] = {
    { RGB(30, 26, 24), RGB(27, 22, 20), RGB(20, 15, 13) }, // Fair
    { RGB(27, 22, 20), RGB(24, 18, 16), RGB(16, 12, 10) }, // Light
    { RGB(24, 18, 16), RGB(20, 15, 13), RGB(12,  9,  7) }, // Mid
    { RGB(20, 15, 13), RGB(16, 12, 10), RGB( 9,  7,  5) }, // Tan
    { RGB(16, 12, 10), RGB(12,  9,  7), RGB( 5,  3,  3) }, // Dark
    { RGB(12,  9,  7), RGB( 9,  7,  5), RGB( 5,  3,  3) }, // Deep
};

#define HAIR_PALETTE_START 4  // OBJ palette indices 4-6
#define SKIN_PALETTE_START 1  // OBJ palette indices 1-3

void ApplyPlayerAppearancePalette(u8 paletteSlot)
{
    u8 hair = gSaveBlock2Ptr->hairColor;
    u8 skin = gSaveBlock2Ptr->skinTone;

    if (hair >= ARRAY_COUNT(gHairPalettes))
        hair = 0;
    if (skin >= ARRAY_COUNT(gSkinPalettes))
        skin = 0;

    LoadPalette(gHairPalettes[hair], OBJ_PLTT_ID(paletteSlot) + HAIR_PALETTE_START, PLTT_SIZEOF(3));
    LoadPalette(gSkinPalettes[skin], OBJ_PLTT_ID(paletteSlot) + SKIN_PALETTE_START, PLTT_SIZEOF(3));
}

// Frame table — all entries share tile art, palette varies per theme
static const struct TilesPal sWindowFrames[WINDOW_FRAMES_COUNT] =
{
    {gTextWindowFrame1_Gfx, gTextWindowFrame1_Pal},
    {gTextWindowFrame1_Gfx, sThemeFramePal1},
    {gTextWindowFrame1_Gfx, sThemeFramePal2},
    {gTextWindowFrame1_Gfx, sThemeFramePal3},
};

static const u16 sTextWindowDexNavFrame[] = INCBIN_U16("graphics/text_window/dexnav_pal.gbapal");
static const struct TilesPal sDexNavWindowFrame = {gTextWindowFrame1_Gfx, sTextWindowDexNavFrame};

// code
const struct TilesPal *GetWindowFrameTilesPal(u8 id)
{
    if (id >= WINDOW_FRAMES_COUNT)
        return &sWindowFrames[0];
    else
        return &sWindowFrames[id];
}

void LoadMessageBoxGfx(u8 windowId, u16 destOffset, u8 palOffset)
{
    LoadBgTiles(GetWindowAttribute(windowId, WINDOW_BG), gMessageBox_Gfx, 0x1C0, destOffset);
    LoadPalette(GetOverworldTextboxPalettePtr(), palOffset, PLTT_SIZE_4BPP);
}

void LoadStdWindowGfx(u8 windowId, u16 destOffset, u8 palOffset)
{
    LoadBgTiles(GetWindowAttribute(windowId, WINDOW_BG), sStdTextWindow_Gfx, 0x120, destOffset);
    LoadPalette(GetTextWindowPalette(3), palOffset, PLTT_SIZE_4BPP);
}

void LoadSignBoxGfx(u8 windowId, u16 destOffset, u8 palOffset)
{
    LoadBgTiles(GetWindowAttribute(windowId, WINDOW_BG), gSignpostWindow_Gfx, 0x1C0, destOffset);
    LoadPalette(GetTextWindowPalette(1), palOffset, PLTT_SIZE_4BPP);
}

void LoadUserWindowBorderGfx_(u8 windowId, u16 destOffset, u8 palOffset)
{
    LoadUserWindowBorderGfx(windowId, destOffset, palOffset);
}

void LoadWindowGfx(u8 windowId, u8 frameId, u16 destOffset, u8 palOffset)
{
    LoadBgTiles(GetWindowAttribute(windowId, WINDOW_BG), sWindowFrames[frameId].tiles, 0x120, destOffset);
    LoadPalette(sWindowFrames[frameId].pal, palOffset, PLTT_SIZE_4BPP);
}

void LoadUserWindowBorderGfx(u8 windowId, u16 destOffset, u8 palOffset)
{
    LoadWindowGfx(windowId, gSaveBlock2Ptr->optionsWindowFrameType, destOffset, palOffset);
}

void DrawTextBorderOuter(u8 windowId, u16 tileNum, u8 palNum)
{
    u8 bgLayer = GetWindowAttribute(windowId, WINDOW_BG);
    u16 tilemapLeft = GetWindowAttribute(windowId, WINDOW_TILEMAP_LEFT);
    u16 tilemapTop = GetWindowAttribute(windowId, WINDOW_TILEMAP_TOP);
    u16 width = GetWindowAttribute(windowId, WINDOW_WIDTH);
    u16 height = GetWindowAttribute(windowId, WINDOW_HEIGHT);

    FillBgTilemapBufferRect(bgLayer, tileNum + 0, tilemapLeft - 1,      tilemapTop - 1,         1,      1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 1, tilemapLeft,          tilemapTop - 1,         width,  1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 2, tilemapLeft + width,  tilemapTop - 1,         1,      1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 3, tilemapLeft - 1,      tilemapTop,             1,      height, palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 5, tilemapLeft + width,  tilemapTop,             1,      height, palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 6, tilemapLeft - 1,      tilemapTop + height,    1,      1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 7, tilemapLeft,          tilemapTop + height,    width,  1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 8, tilemapLeft + width,  tilemapTop + height,    1,      1,      palNum);
}

void DrawTextBorderInner(u8 windowId, u16 tileNum, u8 palNum)
{
    u8 bgLayer = GetWindowAttribute(windowId, WINDOW_BG);
    u16 tilemapLeft = GetWindowAttribute(windowId, WINDOW_TILEMAP_LEFT);
    u16 tilemapTop = GetWindowAttribute(windowId, WINDOW_TILEMAP_TOP);
    u16 width = GetWindowAttribute(windowId, WINDOW_WIDTH);
    u16 height = GetWindowAttribute(windowId, WINDOW_HEIGHT);

    FillBgTilemapBufferRect(bgLayer, tileNum + 0, tilemapLeft,              tilemapTop,                 1,          1,          palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 1, tilemapLeft + 1,          tilemapTop,                 width - 2,  1,          palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 2, tilemapLeft + width - 1,  tilemapTop,                 1,          1,          palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 3, tilemapLeft,              tilemapTop + 1,             1,          height - 2, palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 5, tilemapLeft + width - 1,  tilemapTop + 1,             1,          height - 2, palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 6, tilemapLeft,              tilemapTop + height - 1,    1,          1,          palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 7, tilemapLeft + 1,          tilemapTop + height - 1,    width -     2,  1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 8, tilemapLeft + width - 1,  tilemapTop + height - 1,    1,          1,          palNum);
}

void rbox_fill_rectangle(u8 windowId)
{
    u8 bgLayer = GetWindowAttribute(windowId, WINDOW_BG);
    u16 tilemapLeft = GetWindowAttribute(windowId, WINDOW_TILEMAP_LEFT);
    u16 tilemapTop = GetWindowAttribute(windowId, WINDOW_TILEMAP_TOP);
    u16 width = GetWindowAttribute(windowId, WINDOW_WIDTH);
    u16 height = GetWindowAttribute(windowId, WINDOW_HEIGHT);

    FillBgTilemapBufferRect(bgLayer, 0, tilemapLeft - 1, tilemapTop - 1, width + 2, height + 2, 0x11);
}

const u16 *GetTextWindowPalette(u8 id)
{
    switch (id)
    {
    case 0:
        id = 0x00;
        break;
    case 1:
        id = 0x10;
        break;
    case 2:
        id = 0x20;
        break;
    case 3:
        id = 0x30;
        break;
    case 4:
    default:
        id = 0x40;
        break;
    }

    return (const u16 *)(sTextWindowPalettes) + id;
}

u8 GetActiveTheme(void)
{
    u8 theme = gSaveBlock2Ptr->optionsWindowFrameType;
    if (theme >= THEME_COUNT)
        theme = THEME_GREEN;
    return theme;
}

const u16 *GetActiveThemeTextPal(void)
{
    return gPipThemes[GetActiveTheme()].textPal;
}

const u16 *GetActiveThemeFramePal(void)
{
    u8 theme = gSaveBlock2Ptr->optionsWindowFrameType;
    if (theme >= THEME_COUNT)
        theme = THEME_GREEN;
    return gPipThemes[theme].framePal;
}

const u16 *GetOverworldTextboxPalettePtr(void)
{
    return GetActiveThemeTextPal();
}

// Effectively LoadUserWindowBorderGfx but specifying the bg directly instead of a window from that bg
void LoadUserWindowBorderGfxOnBg(u8 bg, u16 destOffset, u8 palOffset)
{
    LoadBgTiles(bg, sWindowFrames[gSaveBlock2Ptr->optionsWindowFrameType].tiles, 0x120, destOffset);
    LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, palOffset, PLTT_SIZE_4BPP);
}

void LoadDexNavWindowGfx(u8 windowId, u16 destOffset, u8 palOffset)
{
    LoadBgTiles(GetWindowAttribute(windowId, WINDOW_BG), sDexNavWindowFrame.tiles, 0x120, destOffset);
    LoadPalette(sDexNavWindowFrame.pal, palOffset, 32);
}
