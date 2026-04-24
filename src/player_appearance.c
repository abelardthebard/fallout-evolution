#include "global.h"
#include "constants/global.h"   // for GENDER_COUNT
#include "constants/rgb.h"
#include "palette.h"
#include "player_appearance.h"

// Display names. One string per option, referenced by both the menu grids
// in the Birch intro (main_menu.c) and the info-row labels in the terminal
// content viewer (terminal_content_pages.c).
static const u8 sText_Hair_Blonde[] = _("Blond");
static const u8 sText_Hair_Sandy[]  = _("Sandy");
static const u8 sText_Hair_Brown[]  = _("Brown");
static const u8 sText_Hair_Umber[]  = _("Umber");
static const u8 sText_Hair_Ginger[] = _("Ginger");
static const u8 sText_Hair_Red[]    = _("Red");
static const u8 sText_Hair_Black[]  = _("Black");
static const u8 sText_Hair_Raven[]  = _("Raven");
static const u8 sText_Hair_White[]  = _("White");
static const u8 sText_Hair_Ash[]    = _("Ash");

static const u8 sText_Skin_Fair[]  = _("Fair");
static const u8 sText_Skin_Light[] = _("Light");
static const u8 sText_Skin_Mid[]   = _("Mid");
static const u8 sText_Skin_Tan[]   = _("Tan");
static const u8 sText_Skin_Dark[]  = _("Dark");
static const u8 sText_Skin_Deep[]  = _("Deep");

// Hair options: 3-color ramps (highlight, base, shadow) rendered into OBJ
// palette positions 4..6 of the assigned sprite's palette slot.
const struct AppearanceOption gPlayerHairOptions[] =
{
    { { RGB(29, 27, 18), RGB(25, 22, 13), RGB(16, 13,  7) }, sText_Hair_Blonde },
    { { RGB(25, 22, 13), RGB(16, 13,  7), RGB(11,  9,  4) }, sText_Hair_Sandy  },
    { { RGB(14, 12, 11), RGB(10,  8,  7), RGB( 7,  6,  5) }, sText_Hair_Brown  },
    { { RGB(10,  8,  7), RGB( 7,  6,  5), RGB( 5,  3,  3) }, sText_Hair_Umber  },
    { { RGB(27, 19, 12), RGB(22, 14,  7), RGB(16,  9,  4) }, sText_Hair_Ginger },
    { { RGB(22, 14,  7), RGB(16,  9,  4), RGB( 9,  5,  2) }, sText_Hair_Red    },
    { { RGB( 8,  9, 11), RGB( 5,  6,  8), RGB( 3,  4,  6) }, sText_Hair_Black  },
    { { RGB( 5,  6,  8), RGB( 3,  4,  6), RGB( 2,  3,  5) }, sText_Hair_Raven  },
    { { RGB(31, 31, 31), RGB(19, 19, 19), RGB(12, 12, 12) }, sText_Hair_White  },
    { { RGB(25, 25, 25), RGB(12, 12, 12), RGB( 7,  7,  7) }, sText_Hair_Ash    },
};

// Skin options: 3-color ramps rendered into OBJ palette positions 1..3.
const struct AppearanceOption gPlayerSkinTones[] =
{
    { { RGB(30, 26, 24), RGB(27, 22, 20), RGB(20, 15, 13) }, sText_Skin_Fair  },
    { { RGB(27, 22, 20), RGB(24, 18, 16), RGB(16, 12, 10) }, sText_Skin_Light },
    { { RGB(24, 18, 16), RGB(20, 15, 13), RGB(12,  9,  7) }, sText_Skin_Mid   },
    { { RGB(20, 15, 13), RGB(16, 12, 10), RGB( 9,  7,  5) }, sText_Skin_Tan   },
    { { RGB(16, 12, 10), RGB(12,  9,  7), RGB( 5,  3,  3) }, sText_Skin_Dark  },
    { { RGB(12,  9,  7), RGB( 9,  7,  5), RGB( 5,  3,  3) }, sText_Skin_Deep  },
};

const u8 gPlayerHairOptionCount = ARRAY_COUNT(gPlayerHairOptions);
const u8 gPlayerSkinToneCount   = ARRAY_COUNT(gPlayerSkinTones);

// Compile-time guarantees the authored table sizes match the public
// PLAYER_*_OPTION_COUNT macros exposed in the header. If anyone adds or
// removes an option from a table without updating the macro (or vice
// versa), build fails here with a clear marker.
STATIC_ASSERT(ARRAY_COUNT(gPlayerHairOptions) == PLAYER_HAIR_OPTION_COUNT,
              gPlayerHairOptions_countMismatch);
STATIC_ASSERT(ARRAY_COUNT(gPlayerSkinTones)   == PLAYER_SKIN_OPTION_COUNT,
              gPlayerSkinTones_countMismatch);

#define HAIR_PALETTE_START  4
#define SKIN_PALETTE_START  1

const u8 *GetPlayerHairColorName(u8 idx)
{
    if (idx >= gPlayerHairOptionCount)
        return NULL;
    return gPlayerHairOptions[idx].name;
}

const u8 *GetPlayerSkinToneName(u8 idx)
{
    if (idx >= gPlayerSkinToneCount)
        return NULL;
    return gPlayerSkinTones[idx].name;
}

void ApplyPlayerAppearancePalette(u8 paletteSlot)
{
    u8 hair = gSaveBlock2Ptr->hairColor;
    u8 skin = gSaveBlock2Ptr->skinTone;

    if (hair >= gPlayerHairOptionCount)
        hair = 0;
    if (skin >= gPlayerSkinToneCount)
        skin = 0;

    LoadPalette(gPlayerHairOptions[hair].colors,
        OBJ_PLTT_ID(paletteSlot) + HAIR_PALETTE_START, PLTT_SIZEOF(3));
    LoadPalette(gPlayerSkinTones[skin].colors,
        OBJ_PLTT_ID(paletteSlot) + SKIN_PALETTE_START, PLTT_SIZEOF(3));
}

void PreviewPlayerHairPalette(u8 paletteSlot, u8 hairIdx)
{
    AGB_ASSERT(hairIdx < gPlayerHairOptionCount);
    if (hairIdx >= gPlayerHairOptionCount)
        return;
    LoadPalette(gPlayerHairOptions[hairIdx].colors,
        OBJ_PLTT_ID(paletteSlot) + HAIR_PALETTE_START, PLTT_SIZEOF(3));
}

void PreviewPlayerSkinPalette(u8 paletteSlot, u8 skinIdx)
{
    AGB_ASSERT(skinIdx < gPlayerSkinToneCount);
    if (skinIdx >= gPlayerSkinToneCount)
        return;
    LoadPalette(gPlayerSkinTones[skinIdx].colors,
        OBJ_PLTT_ID(paletteSlot) + SKIN_PALETTE_START, PLTT_SIZEOF(3));
}

void SetPlayerGender(u8 gender)
{
    AGB_ASSERT(gender < GENDER_COUNT);
    if (gender >= GENDER_COUNT)
        return;
    gSaveBlock2Ptr->playerGender = gender;
}

void SetPlayerHairColor(u8 idx)
{
    AGB_ASSERT(idx < gPlayerHairOptionCount);
    if (idx >= gPlayerHairOptionCount)
        return;
    gSaveBlock2Ptr->hairColor = idx;
}

void SetPlayerSkinTone(u8 idx)
{
    AGB_ASSERT(idx < gPlayerSkinToneCount);
    if (idx >= gPlayerSkinToneCount)
        return;
    gSaveBlock2Ptr->skinTone = idx;
}
