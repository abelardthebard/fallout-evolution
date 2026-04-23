#include "global.h"
#include "main_menu.h"
#include "string_util.h"
#include "terminal_content.h"
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
static const u8 sText_TerminalMedicalRecords_CogScore[]  = _("COGNITIVE SCORE:");
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
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_CogScore, .action = TERMINAL_ACTION_NONE },
    { .type = TERMINAL_ITEM_BLANK },
    { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Exit,     .action = TERMINAL_ACTION_EXIT },
};

static const struct TerminalPage sTerminalMedicalRecords_Page =
{
    .header    = sText_TerminalMedicalRecords_Header,
    .items     = sTerminalMedicalRecords_Items,
    .itemCount = ARRAY_COUNT(sTerminalMedicalRecords_Items),
    .prepare   = TerminalMedicalRecords_Prepare,
};

// -------------------------------------------------------------------------
// Lookup table: TERMINAL_CONTENT_* id -> page. Indexed by the enum values
// in include/constants/terminal_content.h.
// -------------------------------------------------------------------------

const struct TerminalPage *const gTerminalContents[NUM_TERMINAL_CONTENTS] =
{
    [TERMINAL_CONTENT_MEDICAL_RECORDS] = &sTerminalMedicalRecords_Page,
};
