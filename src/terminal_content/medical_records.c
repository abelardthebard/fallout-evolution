#include "global.h"
#include "bg.h"
#include "data.h"
#include "event_data.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "naming_screen.h"
#include "overworld.h"
#include "palette.h"
#include "random.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "pipboy_theme.h"
#include "player_appearance.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/flags.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/vars.h"
#include "terminal.h"

#include "terminal_internal.h"

static void BuildFieldRow(u8 *out, u8 cap, const u8 *label, const u8 *value)
{
    u8 *end;
    u8 *p;

    if (cap == 0)
        return;

    end = out + cap - 1;
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

#define TERMINAL_MED_RECORDS_LABEL_MAX     8
#define TERMINAL_MED_RECORDS_VALUE_MAX     9
#define TERMINAL_MED_RECORDS_ROW_BUF_SIZE  (TERMINAL_MED_RECORDS_LABEL_MAX + TERMINAL_MED_RECORDS_VALUE_MAX + 1)

static const u8 sText_TerminalMedicalRecords_Header[]    = _("VAULT 42 PATIENT RECORDS");
static const u8 sText_TerminalMedicalRecords_Home[]      = _("HOME: Unit 315");
static const u8 sText_TerminalMedicalRecords_Exit[]      = _("EXIT");

static const u8 sText_TerminalMedicalRecords_NameLbl[]   = _("NAME: ");
static const u8 sText_TerminalMedicalRecords_GenderLbl[] = _("GENDER: ");
static const u8 sText_TerminalMedicalRecords_HairLbl[]   = _("HAIR: ");
static const u8 sText_TerminalMedicalRecords_SkinLbl[]   = _("SKIN: ");
static const u8 sText_TerminalMedicalRecords_EvalLbl[]   = _("EVAL: ");

static const u8 sText_TerminalMedicalRecords_Masculine[] = _("Masculine");
static const u8 sText_TerminalMedicalRecords_Feminine[]  = _("Feminine");
static const u8 sText_TerminalMedicalRecords_Unknown[]   = _("--");

static const u8 sText_TerminalMedicalRecords_EvalBad[]   = _("BAD");
static const u8 sText_TerminalMedicalRecords_EvalPoor[]  = _("POOR");
static const u8 sText_TerminalMedicalRecords_EvalGood[]  = _("GOOD");
static const u8 sText_TerminalMedicalRecords_EvalGreat[] = _("GREAT");

static EWRAM_DATA u8 sTerminalMedicalRecords_NameRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE]   = {0};
static EWRAM_DATA u8 sTerminalMedicalRecords_GenderRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE] = {0};
static EWRAM_DATA u8 sTerminalMedicalRecords_HairRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE]   = {0};
static EWRAM_DATA u8 sTerminalMedicalRecords_SkinRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE]   = {0};
static EWRAM_DATA u8 sTerminalMedicalRecords_EvalRow[TERMINAL_MED_RECORDS_ROW_BUF_SIZE]   = {0};

static const u8 *TerminalMedicalRecords_GenderName(u8 gender)
{
    switch (gender)
    {
    case MALE:   return sText_TerminalMedicalRecords_Masculine;
    case FEMALE: return sText_TerminalMedicalRecords_Feminine;
    default:     return sText_TerminalMedicalRecords_Unknown;
    }
}

static const u8 *TerminalMedicalRecords_EvalLabel(void)
{
    u8 a = gSaveBlock2Ptr->introAnswers;
    u8 score = ((a & INTRO_ANSWER_REMEMBERED_BASICS)    ? 1 : 0)
             + ((a & INTRO_ANSWER_GUESSED_YEAR_RIGHT)   ? 1 : 0)
             + ((a & INTRO_ANSWER_NAMED_TODD_CORRECTLY) ? 1 : 0);
    switch (score)
    {
    case 0:  return sText_TerminalMedicalRecords_EvalBad;
    case 1:  return sText_TerminalMedicalRecords_EvalPoor;
    case 2:  return sText_TerminalMedicalRecords_EvalGood;
    default: return sText_TerminalMedicalRecords_EvalGreat;
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

    BuildFieldRow(sTerminalMedicalRecords_EvalRow, TERMINAL_MED_RECORDS_ROW_BUF_SIZE,
        sText_TerminalMedicalRecords_EvalLbl, TerminalMedicalRecords_EvalLabel());
}

enum
{
    MED_ROW_NAME,
    MED_ROW_HOME,
    MED_ROW_GENDER,
    MED_ROW_HAIR,
    MED_ROW_SKIN,
    MED_ROW_EVAL,
    MED_ROW_BLANK,
    MED_ROW_EXIT,
};

#define HAIR_PICKER_COLS  2

static EWRAM_DATA struct TerminalItem sHairPickerItems[PLAYER_HAIR_OPTION_COUNT] = {0};
static EWRAM_DATA u8 sHairPicker_Original = 0;

static void HairPicker_ClosePicker(void)
{
    TerminalContent_SetCursorMoveHook(NULL);
    TerminalContent_SetCancelCallback(NULL);
    TerminalContent_RestorePageItems(MED_ROW_HAIR);
}

static void HairPicker_Preview(u8 newCursorIdx)
{
    if (newCursorIdx >= PLAYER_HAIR_OPTION_COUNT)
        return;
    if (sTC->playerSpriteId == MAX_SPRITES)
        return;
    PreviewPlayerHairPalette(gSprites[sTC->playerSpriteId].oam.paletteNum, newCursorIdx);
}

static void HairPicker_Cancel(void)
{
    PlaySE(SE_SELECT);
    if (sTC->playerSpriteId != MAX_SPRITES)
        PreviewPlayerHairPalette(gSprites[sTC->playerSpriteId].oam.paletteNum, sHairPicker_Original);
    HairPicker_ClosePicker();
}

static void HairPicker_Commit(void)
{
    u8 idx = sTC->cursorItemIdx;
    if (idx >= PLAYER_HAIR_OPTION_COUNT)
        return;
    SetPlayerHairColor(idx);
    PlaySE(SE_SUCCESS);
    TerminalMedicalRecords_Prepare();
    HairPicker_ClosePicker();
}

static void HairPicker_Open(void)
{
    u8 i;

    PlaySE(SE_SELECT);
    sHairPicker_Original = gSaveBlock2Ptr->hairColor;

    for (i = 0; i < PLAYER_HAIR_OPTION_COUNT; i++)
    {
        sHairPickerItems[i].type       = TERMINAL_ITEM_SELECTABLE;
        sHairPickerItems[i].text       = GetPlayerHairColorName(i);
        sHairPickerItems[i].sprite     = NULL;
        sHairPickerItems[i].onActivate = HairPicker_Commit;
    }

    TerminalContent_SetCursorMoveHook(HairPicker_Preview);
    TerminalContent_SetCancelCallback(HairPicker_Cancel);
    TerminalContent_SetActiveItems(sHairPickerItems, PLAYER_HAIR_OPTION_COUNT,
                                   HAIR_PICKER_COLS, sHairPicker_Original);
}

#define SKIN_PICKER_COLS  2

static EWRAM_DATA struct TerminalItem sSkinPickerItems[PLAYER_SKIN_OPTION_COUNT] = {0};
static EWRAM_DATA u8 sSkinPicker_Original = 0;

static void SkinPicker_ClosePicker(void)
{
    TerminalContent_SetCursorMoveHook(NULL);
    TerminalContent_SetCancelCallback(NULL);
    TerminalContent_RestorePageItems(MED_ROW_SKIN);
}

static void SkinPicker_Preview(u8 newCursorIdx)
{
    if (newCursorIdx >= PLAYER_SKIN_OPTION_COUNT)
        return;
    if (sTC->playerSpriteId == MAX_SPRITES)
        return;
    PreviewPlayerSkinPalette(gSprites[sTC->playerSpriteId].oam.paletteNum, newCursorIdx);
}

static void SkinPicker_Cancel(void)
{
    PlaySE(SE_SELECT);
    if (sTC->playerSpriteId != MAX_SPRITES)
        PreviewPlayerSkinPalette(gSprites[sTC->playerSpriteId].oam.paletteNum, sSkinPicker_Original);
    SkinPicker_ClosePicker();
}

static void SkinPicker_Commit(void)
{
    u8 idx = sTC->cursorItemIdx;
    if (idx >= PLAYER_SKIN_OPTION_COUNT)
        return;
    SetPlayerSkinTone(idx);
    PlaySE(SE_SUCCESS);
    TerminalMedicalRecords_Prepare();
    SkinPicker_ClosePicker();
}

static void SkinPicker_Open(void)
{
    u8 i;

    PlaySE(SE_SELECT);
    sSkinPicker_Original = gSaveBlock2Ptr->skinTone;

    for (i = 0; i < PLAYER_SKIN_OPTION_COUNT; i++)
    {
        sSkinPickerItems[i].type       = TERMINAL_ITEM_SELECTABLE;
        sSkinPickerItems[i].text       = GetPlayerSkinToneName(i);
        sSkinPickerItems[i].sprite     = NULL;
        sSkinPickerItems[i].onActivate = SkinPicker_Commit;
    }

    TerminalContent_SetCursorMoveHook(SkinPicker_Preview);
    TerminalContent_SetCancelCallback(SkinPicker_Cancel);
    TerminalContent_SetActiveItems(sSkinPickerItems, PLAYER_SKIN_OPTION_COUNT,
                                   SKIN_PICKER_COLS, sSkinPicker_Original);
}

#define GENDER_PICKER_COLS  1

static EWRAM_DATA struct TerminalItem sGenderPickerItems[GENDER_COUNT] = {0};
static EWRAM_DATA u8 sGenderPicker_Original = 0;

static void GenderPicker_ClosePicker(void)
{
    TerminalContent_SetCursorMoveHook(NULL);
    TerminalContent_SetCancelCallback(NULL);
    TerminalContent_RestorePageItems(MED_ROW_GENDER);
}

static void GenderPicker_Preview(u8 newCursorIdx)
{
    if (newCursorIdx >= GENDER_COUNT)
        return;
    TerminalContent_RecreatePlayerSprite(newCursorIdx);
}

static void GenderPicker_Cancel(void)
{
    PlaySE(SE_SELECT);
    TerminalContent_RecreatePlayerSprite(sGenderPicker_Original);
    GenderPicker_ClosePicker();
}

static void GenderPicker_Commit(void)
{
    u8 idx = sTC->cursorItemIdx;
    if (idx >= GENDER_COUNT)
        return;
    SetPlayerGender(idx);
    RefreshPlayerAvatarGender();
    PlaySE(SE_SUCCESS);
    TerminalMedicalRecords_Prepare();
    GenderPicker_ClosePicker();
}

static void GenderPicker_Open(void)
{
    u8 i;

    PlaySE(SE_SELECT);
    sGenderPicker_Original = gSaveBlock2Ptr->playerGender;

    for (i = 0; i < GENDER_COUNT; i++)
    {
        sGenderPickerItems[i].type       = TERMINAL_ITEM_SELECTABLE;
        sGenderPickerItems[i].text       = TerminalMedicalRecords_GenderName(i);
        sGenderPickerItems[i].sprite     = NULL;
        sGenderPickerItems[i].onActivate = GenderPicker_Commit;
    }

    TerminalContent_SetCursorMoveHook(GenderPicker_Preview);
    TerminalContent_SetCancelCallback(GenderPicker_Cancel);
    TerminalContent_SetActiveItems(sGenderPickerItems, GENDER_COUNT,
                                   GENDER_PICKER_COLS, sGenderPicker_Original);
}

static EWRAM_DATA const struct TerminalPage *sNamePicker_ReturnPage = NULL;
static EWRAM_DATA u8 sNamePicker_ReturnCursor = 0;

static void CB2_TerminalContent_ReturnFromNamingScreen(void)
{
    EnterTerminalContent(sNamePicker_ReturnPage, sNamePicker_ReturnCursor);
}

static void Task_NamePicker_LaunchNamingScreen(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    DestroyTask(taskId);
    TerminalContent_Teardown();
    DoNamingScreen(NAMING_SCREEN_PLAYER, gSaveBlock2Ptr->playerName,
                   gSaveBlock2Ptr->playerGender, 0, 0,
                   CB2_TerminalContent_ReturnFromNamingScreen);
}

static void NamePicker_Open(void)
{
    PlaySE(SE_SELECT);
    sNamePicker_ReturnPage   = sTC->page;
    sNamePicker_ReturnCursor = sTC->cursorItemIdx;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[sTC->taskId].func = Task_NamePicker_LaunchNamingScreen;
}

const struct TerminalPage sTerminalMedicalRecords_Page;

static const u8 sText_TerminalHousing_Title[]     = _("UNIT 315: THE GREEN FAMILY");
static const u8 sText_TerminalHousing_Matriarch[] = _("MATRIARCH: Iris, Cook");
static const u8 sText_TerminalHousing_Patriarch[] = _("PATRIARCH: Caleb, Engineer");
static const u8 sText_TerminalHousing_Child1Lbl[] = _("CHILD 1: ");
static const u8 sText_TerminalHousing_Child1Suf[]        = _(", Pending");
static const u8 sText_TerminalHousing_Child1SufSurplus[] = _(", Surplus");
static const u8 sText_TerminalHousing_Child2[]    = _("CHILD 2: Maisie, Pending");
static const u8 sText_TerminalHousing_Desc1[]     = _("Home environment reported as");
static const u8 sText_TerminalHousing_Desc2[]     = _("“stable” and “unremarkable”.");
static const u8 sText_TerminalHousing_Back[]      = _("BACK");

// "CHILD 1: " (9) + PLAYER_NAME_LENGTH (7) + ", Pending" (9) + EOS
#define TERMINAL_HOUSING_CHILD1_BUF_SIZE  32

static EWRAM_DATA u8 sTerminalHousing_Child1Row[TERMINAL_HOUSING_CHILD1_BUF_SIZE] = {0};

enum
{
    HOUSING_ROW_TITLE,
    HOUSING_ROW_BLANK1,
    HOUSING_ROW_MATRIARCH,
    HOUSING_ROW_PATRIARCH,
    HOUSING_ROW_CHILD1,
    HOUSING_ROW_CHILD2,
    HOUSING_ROW_BLANK2,
    HOUSING_ROW_DESC1,
    HOUSING_ROW_DESC2,
    HOUSING_ROW_BLANK3,
    HOUSING_ROW_BACK,
};

static void TerminalHousing_Prepare(void)
{
    u8 *dst = sTerminalHousing_Child1Row;
    dst = StringCopy(dst, sText_TerminalHousing_Child1Lbl);
    dst = StringCopy(dst, gSaveBlock2Ptr->playerName);
    StringCopy(dst, FlagGet(FLAG_VIEWED_OWN_TEST_SCORES)
                    ? sText_TerminalHousing_Child1SufSurplus
                    : sText_TerminalHousing_Child1Suf);
}

static void Housing_BackToMedicalRecords(void)
{
    PlaySE(SE_SELECT);
    TerminalContent_SwapPage(&sTerminalMedicalRecords_Page, MED_ROW_HOME);
}

static const struct TerminalItem sTerminalHousing_Items[] =
{
    [HOUSING_ROW_TITLE]     = { .type = TERMINAL_ITEM_TEXT, .text = sText_TerminalHousing_Title     },
    [HOUSING_ROW_BLANK1]    = { .type = TERMINAL_ITEM_BLANK },
    [HOUSING_ROW_MATRIARCH] = { .type = TERMINAL_ITEM_TEXT, .text = sText_TerminalHousing_Matriarch },
    [HOUSING_ROW_PATRIARCH] = { .type = TERMINAL_ITEM_TEXT, .text = sText_TerminalHousing_Patriarch },
    [HOUSING_ROW_CHILD1]    = { .type = TERMINAL_ITEM_TEXT, .text = sTerminalHousing_Child1Row      },
    [HOUSING_ROW_CHILD2]    = { .type = TERMINAL_ITEM_TEXT, .text = sText_TerminalHousing_Child2    },
    [HOUSING_ROW_BLANK2]    = { .type = TERMINAL_ITEM_BLANK },
    [HOUSING_ROW_DESC1]     = { .type = TERMINAL_ITEM_TEXT, .text = sText_TerminalHousing_Desc1     },
    [HOUSING_ROW_DESC2]     = { .type = TERMINAL_ITEM_TEXT, .text = sText_TerminalHousing_Desc2     },
    [HOUSING_ROW_BLANK3]    = { .type = TERMINAL_ITEM_BLANK },
    [HOUSING_ROW_BACK]      = { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalHousing_Back, .onActivate = Housing_BackToMedicalRecords },
};

static const struct TerminalPage sTerminalHousing_Page =
{
    .header        = sText_TerminalMedicalRecords_Header,
    .items         = sTerminalHousing_Items,
    .itemCount     = ARRAY_COUNT(sTerminalHousing_Items),
    .cols          = 1,
    .prepare       = TerminalHousing_Prepare,
    .createSprites = NULL,
    .onBack        = Housing_BackToMedicalRecords,
};

static void Housing_Open(void)
{
    PlaySE(SE_SELECT);
    TerminalContent_SwapPage(&sTerminalHousing_Page, HOUSING_ROW_BACK);
}

static const u8 sText_TerminalEval_Title1[]    = _("S.P.E.C.I.A.L STATS:");
static const u8 sText_TerminalEval_Title2[]    = _("Pending G.O.A.T. results");

static const u8 sText_TerminalEval_S1Right1[]  = _("Recalled Vault 42 and post-war");
static const u8 sText_TerminalEval_S1Right2[]  = _("conditions without prompting.");
static const u8 sText_TerminalEval_S1Wrong1[]  = _("Required briefing on Vault 42");
static const u8 sText_TerminalEval_S1Wrong2[]  = _("and post-war conditions.");

static const u8 sText_TerminalEval_S2Right1[]  = _("Identified current year as 2291.");
static const u8 sText_TerminalEval_S2Right2[]  = _("Hippocampus functional.");
static const u8 sText_TerminalEval_S2Wrong1[]  = _("Misidentified current year.");
static const u8 sText_TerminalEval_S2Wrong2[]  = _("Cause of error undetermined:");
static const u8 sText_TerminalEval_S2Wrong3[]  = _("concussion, ignorance, or defiance.");

static const u8 sText_TerminalEval_S3Locked1[] = _("Stated own name and described");
static const u8 sText_TerminalEval_S3Locked2[] = _("self accurately.");

static const u8 sText_TerminalEval_S4Right1[]  = _("Correctly referred to attending");
static const u8 sText_TerminalEval_S4Right2[]  = _("party as 'Todd'. Social recall intact.");
static const u8 sText_TerminalEval_S4Wrong1[]  = _("Referred to attending party by");
static const u8 sText_TerminalEval_S4Wrong2[]  = _("incorrect name. Suggests left");
static const u8 sText_TerminalEval_S4Wrong3[]  = _("temporal lobe impairment.");

static const u8 sText_TerminalEval_Back[]      = _("BACK");

static const u8 sText_TerminalEval_StrZero[]   = _("STRENGTH: 0");
static const u8 sText_TerminalEval_PerZero[]   = _("PERCEPTION: 0");
static const u8 sText_TerminalEval_EndZero[]   = _("ENDURANCE: 0");
static const u8 sText_TerminalEval_ChaZero[]   = _("CHARISMA: 0");
static const u8 sText_TerminalEval_IntZero[]   = _("INTELLIGENCE: 0");
static const u8 sText_TerminalEval_AgiZero[]   = _("AGILITY: 0");
static const u8 sText_TerminalEval_LckZero[]   = _("LUCK: 0");

#define EVAL_MAX_ROWS  18

static EWRAM_DATA struct TerminalItem sTerminalEval_Items[EVAL_MAX_ROWS] = {0};

static void Eval_BackToMedicalRecords(void)
{
    PlaySE(SE_SELECT);
    TerminalContent_SwapPage(&sTerminalMedicalRecords_Page, MED_ROW_EVAL);
}

static void EvalPushText(u8 *idx, const u8 *text)
{
    sTerminalEval_Items[*idx].type       = TERMINAL_ITEM_TEXT;
    sTerminalEval_Items[*idx].text       = text;
    sTerminalEval_Items[*idx].sprite     = NULL;
    sTerminalEval_Items[*idx].onActivate = NULL;
    (*idx)++;
}

static void EvalPushBlank(u8 *idx)
{
    sTerminalEval_Items[*idx].type       = TERMINAL_ITEM_BLANK;
    sTerminalEval_Items[*idx].text       = NULL;
    sTerminalEval_Items[*idx].sprite     = NULL;
    sTerminalEval_Items[*idx].onActivate = NULL;
    (*idx)++;
}

static void EvalPushBack(u8 *idx)
{
    sTerminalEval_Items[*idx].type       = TERMINAL_ITEM_SELECTABLE;
    sTerminalEval_Items[*idx].text       = sText_TerminalEval_Back;
    sTerminalEval_Items[*idx].sprite     = NULL;
    sTerminalEval_Items[*idx].onActivate = Eval_BackToMedicalRecords;
    (*idx)++;
}

static void TerminalEval_Prepare(void)
{
    u8 a = gSaveBlock2Ptr->introAnswers;
    u8 i = 0;

    if (FlagGet(FLAG_VIEWED_OWN_TEST_SCORES))
    {
        EvalPushText(&i, sText_TerminalEval_Title1);
        EvalPushBlank(&i);
        EvalPushText(&i, sText_TerminalEval_StrZero);
        EvalPushText(&i, sText_TerminalEval_PerZero);
        EvalPushText(&i, sText_TerminalEval_EndZero);
        EvalPushText(&i, sText_TerminalEval_ChaZero);
        EvalPushText(&i, sText_TerminalEval_IntZero);
        EvalPushText(&i, sText_TerminalEval_AgiZero);
        EvalPushText(&i, sText_TerminalEval_LckZero);
        EvalPushBlank(&i);
        EvalPushBack(&i);
        AGB_ASSERT(i <= EVAL_MAX_ROWS);
        sTC->activeItemCount = i;
        return;
    }

    EvalPushText(&i, sText_TerminalEval_Title1);
    EvalPushText(&i, sText_TerminalEval_Title2);
    EvalPushBlank(&i);

    if (a & INTRO_ANSWER_REMEMBERED_BASICS)
    {
        EvalPushText(&i, sText_TerminalEval_S1Right1);
        EvalPushText(&i, sText_TerminalEval_S1Right2);
    }
    else
    {
        EvalPushText(&i, sText_TerminalEval_S1Wrong1);
        EvalPushText(&i, sText_TerminalEval_S1Wrong2);
    }
    EvalPushBlank(&i);

    if (a & INTRO_ANSWER_GUESSED_YEAR_RIGHT)
    {
        EvalPushText(&i, sText_TerminalEval_S2Right1);
        EvalPushText(&i, sText_TerminalEval_S2Right2);
    }
    else
    {
        EvalPushText(&i, sText_TerminalEval_S2Wrong1);
        EvalPushText(&i, sText_TerminalEval_S2Wrong2);
        EvalPushText(&i, sText_TerminalEval_S2Wrong3);
    }
    EvalPushBlank(&i);

    EvalPushText(&i, sText_TerminalEval_S3Locked1);
    EvalPushText(&i, sText_TerminalEval_S3Locked2);
    EvalPushBlank(&i);

    if (a & INTRO_ANSWER_NAMED_TODD_CORRECTLY)
    {
        EvalPushText(&i, sText_TerminalEval_S4Right1);
        EvalPushText(&i, sText_TerminalEval_S4Right2);
    }
    else
    {
        EvalPushText(&i, sText_TerminalEval_S4Wrong1);
        EvalPushText(&i, sText_TerminalEval_S4Wrong2);
        EvalPushText(&i, sText_TerminalEval_S4Wrong3);
    }
    EvalPushBlank(&i);

    EvalPushBack(&i);

    AGB_ASSERT(i <= EVAL_MAX_ROWS);
    sTC->activeItemCount = i;
}

static const struct TerminalPage sTerminalEval_Page =
{
    .header        = sText_TerminalMedicalRecords_Header,
    .items         = sTerminalEval_Items,
    .itemCount     = EVAL_MAX_ROWS,
    .cols          = 1,
    .prepare       = TerminalEval_Prepare,
    .createSprites = NULL,
    .onBack        = Eval_BackToMedicalRecords,
};

static void Eval_Open(void)
{
    PlaySE(SE_SELECT);
    TerminalContent_SwapPage(&sTerminalEval_Page, 0);
}

static const struct TerminalItem sTerminalMedicalRecords_Items[] =
{
    [MED_ROW_NAME]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_NameRow,   .onActivate = NamePicker_Open },
    [MED_ROW_HOME]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Home, .onActivate = Housing_Open },
    [MED_ROW_GENDER] = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_GenderRow, .onActivate = GenderPicker_Open },
    [MED_ROW_HAIR]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_HairRow,   .onActivate = HairPicker_Open },
    [MED_ROW_SKIN]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_SkinRow,   .onActivate = SkinPicker_Open },
    [MED_ROW_EVAL]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_EvalRow,   .onActivate = Eval_Open },
    [MED_ROW_BLANK]  = { .type = TERMINAL_ITEM_BLANK },
    [MED_ROW_EXIT]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Exit, .onActivate = TerminalContent_ExitCallback },
};

#define MEDICAL_RECORDS_Y_OFFSET           (-6)
#define MEDICAL_RECORDS_WIN_EXTEND_LEFT    8
#define MEDICAL_RECORDS_PLAY_X             (T_LOG_X - MEDICAL_RECORDS_WIN_EXTEND_LEFT)
#define MEDICAL_RECORDS_PLAY_W             (T_LOG_RESERVED_W + MEDICAL_RECORDS_WIN_EXTEND_LEFT)
#define MEDICAL_RECORDS_SPRITE_X           (MEDICAL_RECORDS_PLAY_X + MEDICAL_RECORDS_PLAY_W / 2)
#define MEDICAL_RECORDS_SPRITE_Y           (T_LOG_Y + T_BODY_H / 2 + MEDICAL_RECORDS_Y_OFFSET)

#define TERMINAL_SPOTLIGHT_CENTER_X        40
#define TERMINAL_SPOTLIGHT_CENTER_Y        16

#define MEDICAL_RECORDS_GLOW_VS_SPRITE_Y   28

#define MEDICAL_RECORDS_BG1_HOFS   ((u16)(TERMINAL_SPOTLIGHT_CENTER_X - MEDICAL_RECORDS_SPRITE_X))
#define MEDICAL_RECORDS_BG1_VOFS   ((u16)(TERMINAL_SPOTLIGHT_CENTER_Y \
                                          - (MEDICAL_RECORDS_SPRITE_Y + MEDICAL_RECORDS_GLOW_VS_SPRITE_Y)))

static const struct SpotlightLayout sMedicalRecords_Spotlight =
{
    .clipLeft   = MEDICAL_RECORDS_PLAY_X,
    .clipTop    = T_LOG_Y + MEDICAL_RECORDS_Y_OFFSET,
    .clipWidth  = MEDICAL_RECORDS_PLAY_W,
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

const struct TerminalPage sTerminalMedicalRecords_Page =
{
    .header        = sText_TerminalMedicalRecords_Header,
    .items         = sTerminalMedicalRecords_Items,
    .itemCount     = ARRAY_COUNT(sTerminalMedicalRecords_Items),
    .cols          = 1,
    .prepare       = TerminalMedicalRecords_Prepare,
    .createSprites = TerminalMedicalRecords_CreateSprites,
};
