#include "global.h"
#include "constants/rgb.h"
#include "text.h"
#include "text_window.h"
#include "pipboy_theme.h"
#include "window.h"
#include "palette.h"
#include "bg.h"
#include "graphics.h"
#include "menu.h"

static const u16 sStdTextWindow_Gfx[]  = INCBIN_U16("graphics/text_window/std.4bpp");

// All themes share the same tile art; per-theme frame palettes live in
// pipboy_theme.c paired with this gfx via sWindowFrames[].
const u8 gTextWindowFrame1_Gfx[] = INCBIN_U8("graphics/text_window/1.4bpp");

// Vanilla text palettes (used by signpost, naming screen, etc.)
static const u16 sTextWindowPalettes[][16] =
{
    INCBIN_U16("graphics/text_window/piptext_pal0.gbapal"),
    INCBIN_U16("graphics/text_window/text_pal1.gbapal"),
    INCBIN_U16("graphics/text_window/text_pal2.gbapal"),
    INCBIN_U16("graphics/text_window/text_pal3.gbapal"),
    INCBIN_U16("graphics/text_window/text_pal4.gbapal")
};

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

// DexNav frame uses its own (non-themed) palette. The Pip-Boy themed
// frames live in pipboy_theme.c; access them via GetWindowFrameTilesPal().
static const u16 sTextWindowDexNavFrame[] = INCBIN_U16("graphics/text_window/dexnav_pal.gbapal");
static const struct TilesPal sDexNavWindowFrame = {gTextWindowFrame1_Gfx, sTextWindowDexNavFrame};

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
    const struct TilesPal *frame = GetWindowFrameTilesPal(frameId);
    LoadBgTiles(GetWindowAttribute(windowId, WINDOW_BG), frame->tiles, 0x120, destOffset);
    LoadPalette(frame->pal, palOffset, PLTT_SIZE_4BPP);
}

void LoadUserWindowBorderGfx(u8 windowId, u16 destOffset, u8 palOffset)
{
    LoadWindowGfx(windowId, GetActiveTheme(), destOffset, palOffset);
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

const u16 *GetOverworldTextboxPalettePtr(void)
{
    return GetActiveThemeTextPal();
}

// Effectively LoadUserWindowBorderGfx but specifying the bg directly instead of a window from that bg
void LoadUserWindowBorderGfxOnBg(u8 bg, u16 destOffset, u8 palOffset)
{
    const struct TilesPal *frame = GetWindowFrameTilesPal(GetActiveTheme());
    LoadBgTiles(bg, frame->tiles, 0x120, destOffset);
    LoadPalette(frame->pal, palOffset, PLTT_SIZE_4BPP);
}

void LoadDexNavWindowGfx(u8 windowId, u16 destOffset, u8 palOffset)
{
    LoadBgTiles(GetWindowAttribute(windowId, WINDOW_BG), sDexNavWindowFrame.tiles, 0x120, destOffset);
    LoadPalette(sDexNavWindowFrame.pal, palOffset, 32);
}
