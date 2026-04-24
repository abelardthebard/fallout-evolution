#include "global.h"
#include "constants/rgb.h"
#include "bg.h"
#include "decompress.h"
#include "palette.h"
#include "text_window.h"   // for gTextWindowFrame1_Gfx (window-frame tile art)
#include "pipboy_theme.h"

// -------------------------------------------------------------------------
// Per-theme frame palettes. Paired 1:1 with themes in sWindowFrames[].
// Green is public extern (gPipBoyFramePalGreen) because save_failed_screen
// hardcodes to green regardless of the player's chosen theme -- see
// include/pipboy_theme.h for why.
// -------------------------------------------------------------------------

const u16 gPipBoyFramePalGreen[]       = INCBIN_U16("graphics/text_window/pip0.gbapal");
static const u16 sPipBoyFramePalBlue[]   = INCBIN_U16("graphics/text_window/pip1.gbapal");
static const u16 sPipBoyFramePalRed[]    = INCBIN_U16("graphics/text_window/pip2.gbapal");
static const u16 sPipBoyFramePalYellow[] = INCBIN_U16("graphics/text_window/pip3.gbapal");

// Per-theme text-window palettes (slots 14 / 15). All file-static -- access
// is via GetActiveThemeTextPal() / gPipBoyThemes[].textPal.
static const u16 sPipBoyTextPalGreen[]  = INCBIN_U16("graphics/text_window/piptext_pal0.gbapal");
static const u16 sPipBoyTextPalBlue[]   = INCBIN_U16("graphics/text_window/piptext_pal1.gbapal");
static const u16 sPipBoyTextPalRed[]    = INCBIN_U16("graphics/text_window/piptext_pal2.gbapal");
static const u16 sPipBoyTextPalYellow[] = INCBIN_U16("graphics/text_window/piptext_pal3.gbapal");

// -------------------------------------------------------------------------
// Public theme table -- indexed by THEME_*.
// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------
// 8-shade Pip-Boy ramps, bright -> dark per theme. The "green ramp" is the
// authored baseline; palette files on disk contain these exact values, so
// PipBoy_ApplyThemeToPalettes can scan any loaded palette buffer and swap
// greens for the active theme's equivalent shade.
// -------------------------------------------------------------------------

static const u16 sPipBoyGreenRamp[PIPBOY_RAMP_SIZE] = {
    RGB(24, 31, 15), // #C0F878
    RGB(19, 29, 12), // #98E860
    RGB(14, 27,  8), // #70D840
    RGB( 9, 25,  4), // #48C820
    RGB( 5, 21,  2), // #28A810
    RGB( 3, 16,  2), // #188010
    RGB( 2, 12,  1), // #106008
    RGB( 1,  8,  1), // #084008
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

// -------------------------------------------------------------------------
// Per-theme spotlight gradient palettes (8 colors each). Loaded into the
// slot-3 gradient reservation by PipBoy_LoadThemeGradient; indexed directly
// by the Birch-intro spotlight fade task via gPipBoyGradients[theme][offset].
// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------
// Shared spotlight assets: tile art + tilemap + per-theme 16-color backdrop
// palettes. Originally authored for the Birch intro; now consumed by any
// themed screen that wants the "player stands on a glowing disc" look
// (terminal content viewer, future screens). Loaded into a caller-chosen
// BG via PipBoy_LoadSpotlight().
//
// The backdrop palettes carry the gradient at positions 1-8 (matching
// gPipBoyGradients) plus feature-independent accent colors at 0 and 9-15
// (dark frame, shadow, pin highlights). PipBoy_LoadSpotlight overlays the
// live gradient after loading the backdrop so the gradient always matches
// whatever animation offset the caller wants.
// -------------------------------------------------------------------------

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

void PipBoy_LoadSpotlight(u8 charBase, u8 mapBase)
{
    DecompressDataWithHeaderVram(sPipBoySpotlightGfx, (void *)BG_CHAR_ADDR(charBase));
    DecompressDataWithHeaderVram(sPipBoySpotlightMap, (void *)BG_SCREEN_ADDR(mapBase));
    LoadPalette(sPipBoyBackdropPals[GetActiveTheme()], BG_PLTT_ID(3), PLTT_SIZE_4BPP);
    PipBoy_LoadThemeGradient(BG_PLTT_ID(3));
}

// -------------------------------------------------------------------------
// Theme-indexed window-frame table. Tile art is shared across all themes;
// only the palette differs.
// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------
// Active-theme queries
// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------
// Theme application
// -------------------------------------------------------------------------

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
