#include "global.h"
#include "constants/rgb.h"
#include "bg.h"
#include "decompress.h"
#include "palette.h"
#include "text_window.h"
#include "pipboy_theme.h"

const u16 gPipBoyFramePalGreen[]         = INCBIN_U16("graphics/text_window/pip0.gbapal");
static const u16 sPipBoyFramePalBlue[]   = INCBIN_U16("graphics/text_window/pip1.gbapal");
static const u16 sPipBoyFramePalRed[]    = INCBIN_U16("graphics/text_window/pip2.gbapal");
static const u16 sPipBoyFramePalYellow[] = INCBIN_U16("graphics/text_window/pip3.gbapal");

static const u16 sPipBoyTextPalGreen[]  = INCBIN_U16("graphics/text_window/piptext_pal0.gbapal");
static const u16 sPipBoyTextPalBlue[]   = INCBIN_U16("graphics/text_window/piptext_pal1.gbapal");
static const u16 sPipBoyTextPalRed[]    = INCBIN_U16("graphics/text_window/piptext_pal2.gbapal");
static const u16 sPipBoyTextPalYellow[] = INCBIN_U16("graphics/text_window/piptext_pal3.gbapal");

const struct PipTheme gPipBoyThemes[THEME_COUNT] =
{
    [THEME_GREEN] = {
        .framePal       = gPipBoyFramePalGreen,
        .textPal        = sPipBoyTextPalGreen,
        .mainMenuFg     = RGB(9, 25, 4),
        .mainMenuShadow = RGB(1, 8, 1),
    },
    [THEME_BLUE] = {
        .framePal       = sPipBoyFramePalBlue,
        .textPal        = sPipBoyTextPalBlue,
        .mainMenuFg     = RGB(9, 19, 28),
        .mainMenuShadow = RGB(3, 5, 10),
    },
    [THEME_RED] = {
        .framePal       = sPipBoyFramePalRed,
        .textPal        = sPipBoyTextPalRed,
        .mainMenuFg     = RGB(29, 8, 6),
        .mainMenuShadow = RGB(10, 2, 1),
    },
    [THEME_YELLOW] = {
        .framePal       = sPipBoyFramePalYellow,
        .textPal        = sPipBoyTextPalYellow,
        .mainMenuFg     = RGB(27, 25, 5),
        .mainMenuShadow = RGB(7, 6, 0),
    },
};

static const u16 sPipBoyGreenRamp[PIPBOY_RAMP_SIZE] = {
    RGB(24, 31, 15),
    RGB(19, 29, 12),
    RGB(14, 27,  8),
    RGB( 9, 25,  4),
    RGB( 5, 21,  2),
    RGB( 3, 16,  2),
    RGB( 2, 12,  1),
    RGB( 1,  8,  1),
};

static const u16 sPipBoyThemeRamps[THEME_COUNT][PIPBOY_RAMP_SIZE] = {
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

static const u16 sPipBoyGradientGreen[]  = INCBIN_U16("graphics/birch_speech/bg2.gbapal");
static const u16 sPipBoyGradientBlue[]   = INCBIN_U16("graphics/birch_speech/bg2_blue.gbapal");
static const u16 sPipBoyGradientRed[]    = INCBIN_U16("graphics/birch_speech/bg2_red.gbapal");
static const u16 sPipBoyGradientYellow[] = INCBIN_U16("graphics/birch_speech/bg2_yellow.gbapal");

const u16 *const gPipBoyGradients[THEME_COUNT] = {
    [THEME_GREEN]  = sPipBoyGradientGreen,
    [THEME_BLUE]   = sPipBoyGradientBlue,
    [THEME_RED]    = sPipBoyGradientRed,
    [THEME_YELLOW] = sPipBoyGradientYellow,
};

static const u16 sPipBoyBackdropGreen[]  = INCBIN_U16("graphics/birch_speech/bg0.gbapal");
static const u16 sPipBoyBackdropBlue[]   = INCBIN_U16("graphics/birch_speech/bg0_blue.gbapal");
static const u16 sPipBoyBackdropRed[]    = INCBIN_U16("graphics/birch_speech/bg0_red.gbapal");
static const u16 sPipBoyBackdropYellow[] = INCBIN_U16("graphics/birch_speech/bg0_yellow.gbapal");

static const u16 *const sPipBoyBackdropPals[THEME_COUNT] = {
    [THEME_GREEN]  = sPipBoyBackdropGreen,
    [THEME_BLUE]   = sPipBoyBackdropBlue,
    [THEME_RED]    = sPipBoyBackdropRed,
    [THEME_YELLOW] = sPipBoyBackdropYellow,
};

static const u32 sPipBoySpotlightGfx[] = INCBIN_U32("graphics/birch_speech/shadow.4bpp.smol");
static const u32 sPipBoySpotlightMap[] = INCBIN_U32("graphics/birch_speech/map.bin.smolTM");

static const u32 sPipBoyTerminalSpotlightGfx[] = INCBIN_U32("graphics/terminal/spotlight.4bpp.smol");
static const u32 sPipBoyTerminalSpotlightMap[] = INCBIN_U32("graphics/terminal/spotlight.bin.smolTM");

static void LoadSpotlightAssets(const u32 *gfx, const u32 *map, u8 charBase, u8 mapBase)
{
    DecompressDataWithHeaderVram(gfx, (void *)BG_CHAR_ADDR(charBase));
    DecompressDataWithHeaderVram(map, (void *)BG_SCREEN_ADDR(mapBase));
    LoadPalette(sPipBoyBackdropPals[GetActiveTheme()], BG_PLTT_ID(3), PLTT_SIZE_4BPP);
    PipBoy_LoadThemeGradient(BG_PLTT_ID(3));
}

void PipBoy_LoadSpotlight(u8 charBase, u8 mapBase)
{
    LoadSpotlightAssets(sPipBoySpotlightGfx, sPipBoySpotlightMap, charBase, mapBase);
}

void PipBoy_LoadTerminalSpotlight(u8 charBase, u8 mapBase)
{
    LoadSpotlightAssets(sPipBoyTerminalSpotlightGfx, sPipBoyTerminalSpotlightMap, charBase, mapBase);
}

static const struct TilesPal sWindowFrames[THEME_COUNT] =
{
    [THEME_GREEN]  = { gTextWindowFrame1_Gfx, gPipBoyFramePalGreen   },
    [THEME_BLUE]   = { gTextWindowFrame1_Gfx, sPipBoyFramePalBlue    },
    [THEME_RED]    = { gTextWindowFrame1_Gfx, sPipBoyFramePalRed     },
    [THEME_YELLOW] = { gTextWindowFrame1_Gfx, sPipBoyFramePalYellow  },
};

const struct TilesPal *GetWindowFrameTilesPal(u8 id)
{
    if (id >= THEME_COUNT)
        return &sWindowFrames[0];
    return &sWindowFrames[id];
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
    return gPipBoyThemes[GetActiveTheme()].textPal;
}

const u16 *GetActiveThemeFramePal(void)
{
    return gPipBoyThemes[GetActiveTheme()].framePal;
}

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
            if (gPlttBufferUnfaded[i] == sPipBoyGreenRamp[j])
            {
                gPlttBufferUnfaded[i] = sPipBoyThemeRamps[theme][j];
                gPlttBufferFaded[i]   = sPipBoyThemeRamps[theme][j];
                break;
            }
        }
    }

    for (i = objStart; i < objStart + objCount; i++)
    {
        for (j = 0; j < PIPBOY_RAMP_SIZE; j++)
        {
            if (gPlttBufferUnfaded[i] == sPipBoyGreenRamp[j])
            {
                gPlttBufferUnfaded[i] = sPipBoyThemeRamps[theme][j];
                gPlttBufferFaded[i]   = sPipBoyThemeRamps[theme][j];
                break;
            }
        }
    }
}

void PipBoy_LoadThemeGradient(u16 destPalBase)
{
    LoadPalette(gPipBoyGradients[GetActiveTheme()], destPalBase + 1, PLTT_SIZEOF(8));
}
