#include "global.h"
#include "player_appearance.h"
#include "string_util.h"
#include "terminal_content.h"
#include "terminal_layout.h"
#include "terminal_ui.h"
#include "constants/characters.h"

// -------------------------------------------------------------------------
// Row builder -- bounds-checked, never writes past cap-1. Caller supplies
// the destination buffer and its full capacity (including the EOS byte).
// -------------------------------------------------------------------------

static void BuildFieldRow(u8 *out, u8 cap, const u8 *label, const u8 *value)
{
    u8 *end;
    u8 *p;

    if (cap == 0)
        return;

    end = out + cap - 1;   // reserve the final slot for EOS
    p = out;

    while (p < end && *label != EOS)
        *p++ = *label++;

    if (value != NULL)
    {
        while (p < end && *value != EOS)
            *p++ = *value++;
    }

    *p = EOS;
}

// -------------------------------------------------------------------------
// TERMINAL_CONTENT_MEDICAL_RECORDS
// -------------------------------------------------------------------------
//
// Opens directly on the player's medical record. Body rows are
// SELECTABLE + TERMINAL_ACTION_NONE so the cursor scrolls through them
// like list items (activating them is a no-op today; actions can be
// attached later). EXIT carries TERMINAL_ACTION_EXIT.
//
// Dynamic text comes from gSaveBlock2Ptr and is written into the row
// buffers below by TerminalMedicalRecords_Prepare(), called once at
// page entry (see ShowTerminalContent).

// Longest label is "GENDER: " (8 bytes). Longest value is "Masculine"
// (9 bytes). + EOS = 18. Buffer size is derived so additions to either
// side update it symbolically.
#define TERMINAL_MED_RECORDS_LABEL_MAX     8
#define TERMINAL_MED_RECORDS_VALUE_MAX     9
#define TERMINAL_MED_RECORDS_ROW_BUF_SIZE  (TERMINAL_MED_RECORDS_LABEL_MAX + TERMINAL_MED_RECORDS_VALUE_MAX + 1)

static const u8 sText_TerminalMedicalRecords_Header[]    = _("VAULT 42 PATIENT RECORDS");
static const u8 sText_TerminalMedicalRecords_Home[]      = _("HOME: Unit 315");
static const u8 sText_TerminalMedicalRecords_Eval[]      = _("EVAL:");
static const u8 sText_TerminalMedicalRecords_Exit[]      = _("EXIT");

static const u8 sText_TerminalMedicalRecords_NameLbl[]   = _("NAME: ");
static const u8 sText_TerminalMedicalRecords_GenderLbl[] = _("GENDER: ");
static const u8 sText_TerminalMedicalRecords_HairLbl[]   = _("HAIR: ");
static const u8 sText_TerminalMedicalRecords_SkinLbl[]   = _("SKIN: ");

static const u8 sText_TerminalMedicalRecords_Masculine[] = _("Masculine");
static const u8 sText_TerminalMedicalRecords_Feminine[]  = _("Feminine");
static const u8 sText_TerminalMedicalRecords_Unknown[]   = _("--");

static EWRAM_DATA u8 sTerminalMedicalRecords_NameRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE]   = {0};
static EWRAM_DATA u8 sTerminalMedicalRecords_GenderRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE] = {0};
static EWRAM_DATA u8 sTerminalMedicalRecords_HairRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE]   = {0};
static EWRAM_DATA u8 sTerminalMedicalRecords_SkinRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE]   = {0};

static const u8 *TerminalMedicalRecords_GenderName(u8 gender)
{
    switch (gender)
    {
    case MALE:   return sText_TerminalMedicalRecords_Masculine;
    case FEMALE: return sText_TerminalMedicalRecords_Feminine;
    default:     return sText_TerminalMedicalRecords_Unknown;
    }
}

static void TerminalMedicalRecords_Prepare(void)
{
    const u8 *hair = GetPlayerHairColorName(gSaveBlock2Ptr->hairColor);
    const u8 *skin = GetPlayerSkinToneName(gSaveBlock2Ptr->skinTone);

    BuildFieldRow(sTerminalMedicalRecords_NameRow, TERMINAL_MED_RECORDS_ROW_BUF_SIZE,
        sText_TerminalMedicalRecords_NameLbl, gSaveBlock2Ptr->playerName);

    BuildFieldRow(sTerminalMedicalRecords_GenderRow, TERMINAL_MED_RECORDS_ROW_BUF_SIZE,
        sText_TerminalMedicalRecords_GenderLbl,
        TerminalMedicalRecords_GenderName(gSaveBlock2Ptr->playerGender));

    BuildFieldRow(sTerminalMedicalRecords_HairRow, TERMINAL_MED_RECORDS_ROW_BUF_SIZE,
        sText_TerminalMedicalRecords_HairLbl,
        (hair != NULL) ? hair : sText_TerminalMedicalRecords_Unknown);

    BuildFieldRow(sTerminalMedicalRecords_SkinRow, TERMINAL_MED_RECORDS_ROW_BUF_SIZE,
        sText_TerminalMedicalRecords_SkinLbl,
        (skin != NULL) ? skin : sText_TerminalMedicalRecords_Unknown);
}

static const struct TerminalItem sTerminalMedicalRecords_Items[] =
{
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_NameRow,       .action = TERMINAL_ACTION_NONE },
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Home,     .action = TERMINAL_ACTION_NONE },
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_GenderRow,     .action = TERMINAL_ACTION_NONE },
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_HairRow,       .action = TERMINAL_ACTION_NONE },
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_SkinRow,       .action = TERMINAL_ACTION_NONE },
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Eval,     .action = TERMINAL_ACTION_NONE },
    { .type = TERMINAL_ITEM_BLANK },
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Exit,     .action = TERMINAL_ACTION_EXIT },
};

// Sprite position: centered in the terminal's log column (right-side
// region of the safe area). Math derives entirely from layout constants.
#define MEDICAL_RECORDS_SPRITE_X   (T_LOG_X + T_LOG_RESERVED_W / 2)  // 148 + 36 = 184
#define MEDICAL_RECORDS_SPRITE_Y   (T_LOG_Y + T_BODY_H / 2)          // 29 + 60 = 89

// Extend the WIN0 clip left of the log-column edge so the spotlight has
// more breathing room. Up to T_LOG_PAD_X (8) pixels uses only the gap
// between the text and log columns with no impact on text rendering;
// beyond that, the spotlight starts rendering behind the text rows (BG 0
// text stays on top since its priority is lower than BG 1 spotlight).
#define MEDICAL_RECORDS_WIN_EXTEND_LEFT   8

// Birch tilemap anchor points -- measured values derived from inspecting
// map.bin + shadow.4bpp.
//
// BIRCH_SPRITE_X/Y: the Birch-intro sprite position used during appearance
// selection (src/main_menu.c:2216/2222, CreateTrainerSprite(..., 120, 60, ...)),
// which is the only phase where the sprite is visibly standing on the disc.
// (Later in the intro the sprite moves to x=180 AND the platform fades out,
// so that position is unrelated to the tilemap's disc location.)
//
// BIRCH_DISC_CENTER_X/Y: the disc's actual pixel-center in the Birch
// tilemap -- measured by classifying each tile's pixels as gradient vs.
// bg-art and taking the bounding-box center of the disc cells. The disc
// is centered horizontally under the sprite (same X) and vertically offset
// by +28 from the sprite center (so the sprite's feet at y+32 sit 4 px
// below disc center, the standard "standing on a floor" layout).
#define BIRCH_SPRITE_X             120
#define BIRCH_SPRITE_Y             60
#define BIRCH_DISC_CENTER_X        120
#define BIRCH_DISC_CENTER_Y        88

// BG 1 scroll that translates Birch's sprite anchor to the terminal's
// sprite position. By preserving this delta the sprite/disc relationship
// from Birch carries over intact: the disc sits under the sprite at the
// same offset, the sprite's feet land on the disc's floor at the same
// 4-pixel offset below disc center. BG scroll semantics: positive
// HOFS/VOFS scrolls the view right/down in BG space, shifting content
// left/up on screen -- so the register value is (Birch coord - terminal
// coord), with negatives wrapping via the u16 cast.
#define MEDICAL_RECORDS_BG1_HOFS   ((u16)(BIRCH_SPRITE_X - MEDICAL_RECORDS_SPRITE_X))
#define MEDICAL_RECORDS_BG1_VOFS   ((u16)(BIRCH_SPRITE_Y - MEDICAL_RECORDS_SPRITE_Y))

// Spotlight placement for this page. All fields derive from the layout
// constants + Birch's native sprite coords so nothing here is a magic
// number except MEDICAL_RECORDS_WIN_EXTEND_LEFT (empirical breathing-room
// extension into T_LOG_PAD_X) and the +26 sprite-X bias (empirical --
// Birch's disc visual center is offset from its sprite center by that
// amount in the tile art we inherit).
static const struct SpotlightLayout sMedicalRecords_Spotlight =
{
    .clipLeft   = T_LOG_X - MEDICAL_RECORDS_WIN_EXTEND_LEFT,
    .clipTop    = T_LOG_Y,
    .clipWidth  = T_LOG_RESERVED_W + MEDICAL_RECORDS_WIN_EXTEND_LEFT,
    .clipHeight = T_BODY_H,
    .scrollX    = MEDICAL_RECORDS_BG1_HOFS,
    .scrollY    = MEDICAL_RECORDS_BG1_VOFS,
};

static void TerminalMedicalRecords_CreateSprites(void)
{
    TerminalUI_ShowSpotlight(&sMedicalRecords_Spotlight);
    TerminalContent_CreatePlayerSprite(MEDICAL_RECORDS_SPRITE_X,
                                       MEDICAL_RECORDS_SPRITE_Y);
}

static const struct TerminalPage sTerminalMedicalRecords_Page =
{
    .header        = sText_TerminalMedicalRecords_Header,
    .items         = sTerminalMedicalRecords_Items,
    .itemCount     = ARRAY_COUNT(sTerminalMedicalRecords_Items),
    .prepare       = TerminalMedicalRecords_Prepare,
    .createSprites = TerminalMedicalRecords_CreateSprites,
};

// -------------------------------------------------------------------------
// Lookup table: TERMINAL_CONTENT_* id -> page. Indexed by the enum values
// in include/constants/terminal_content.h.
// -------------------------------------------------------------------------

const struct TerminalPage *const gTerminalContents[NUM_TERMINAL_CONTENTS] =
{
    [TERMINAL_CONTENT_MEDICAL_RECORDS] = &sTerminalMedicalRecords_Page,
};
