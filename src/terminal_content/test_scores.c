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

// ============================================================================
// CLASSROOM G.O.A.T. RESULTS TERMINAL
// ----------------------------------------------------------------------------
// Main page: a scrollable, alphabetically-sorted student roster (10 NPCs +
// player). Player is inserted at runtime in the right alphabetical slot,
// tie-broken by last initial (player is always "G.").
// Each row opens a shared StudentDetails sub-page whose contents are
// populated from sNpcStudents[] or the player struct.
// ============================================================================

#define NUM_TEST_SCORE_NPCS      10
#define PLAYER_STUDENT_IDX       NUM_TEST_SCORE_NPCS   // 10
#define TEST_SCORES_MAX_ROWS     14                    // 11 students + blank + EXIT + buffer
#define STUDENT_DETAILS_MAX_ROWS 18                    // name + blank + asn + 3 desc + blank + 7 stats + blank + BACK

struct StudentScore
{
    const u8 *displayName;       // "Elmer B." — used in the list AND the sub-page NAME row
    const u8 *firstName;         // "Elmer"   — used for sort comparison vs the player's first name
    const u8 *lastInitial;       // "B"        — single-char string for tie-break vs player's "G"
    const u8 *assignment;        // "Surplus", "Vault Maintenance", etc.
    const u8 *descLine1;         // required (1+ description lines)
    const u8 *descLine2;         // NULL if not used
    const u8 *descLine3;         // NULL if not used
    u8 strength, perception, endurance, charisma, intelligence, agility, luck;
};

// ----- NPC display + sort strings (TBD = fill in as user provides data) -----

static const u8 sText_Elmer_Display[]      = _("Elmer B.");
static const u8 sText_Elmer_FirstName[]    = _("Elmer");
static const u8 sText_Elmer_LastInitial[]  = _("B");
static const u8 sText_Elmer_Assignment[]   = _("Electrician");
static const u8 sText_Elmer_Desc1[]        = _("Maintains lighting, wiring, and");
static const u8 sText_Elmer_Desc2[]        = _("circuit integrity in the vault.");

static const u8 sText_Dorris_Display[]       = _("Dorris T.");
static const u8 sText_Dorris_FirstName[]     = _("Dorris");
static const u8 sText_Dorris_LastInitial[]   = _("T");
static const u8 sText_Dorris_Assignment[]    = _("Barber");
static const u8 sText_Dorris_Desc1[]         = _("Cuts and styles hair vault-wide.");
static const u8 sText_Dorris_Desc2[]         = _("The shop is a social hub.");

static const u8 sText_Clifford_Display[]      = _("Clifford R.");
static const u8 sText_Clifford_FirstName[]    = _("Clifford");
static const u8 sText_Clifford_LastInitial[]  = _("R");
static const u8 sText_Clifford_Assignment[]   = _("Janitor");
static const u8 sText_Clifford_Desc1[]        = _("Daily sanitation of corridors,");
static const u8 sText_Clifford_Desc2[]        = _("common areas, and quarters.");

static const u8 sText_Herschel_Display[]     = _("Herschel F.");
static const u8 sText_Herschel_FirstName[]   = _("Herschel");
static const u8 sText_Herschel_LastInitial[] = _("F");
static const u8 sText_Herschel_Assignment[]  = _("Archivist");
static const u8 sText_Herschel_Desc1[]       = _("Catalogs vault records:");
static const u8 sText_Herschel_Desc2[]       = _("births, deaths, work logs.");

static const u8 sText_Loretta_Display[]        = _("Loretta D.");
static const u8 sText_Loretta_FirstName[]      = _("Loretta");
static const u8 sText_Loretta_LastInitial[]    = _("D");
static const u8 sText_Loretta_Assignment[]     = _("Morale Officer");
static const u8 sText_Loretta_Desc1[]          = _("Organizes events, holiday");
static const u8 sText_Loretta_Desc2[]          = _("observances, and contests.");

static const u8 sText_Mavis_Display[]      = _("Mavis R.");
static const u8 sText_Mavis_FirstName[]    = _("Mavis");
static const u8 sText_Mavis_LastInitial[]  = _("R");
static const u8 sText_Mavis_Assignment[]   = _("Hall Monitor");
static const u8 sText_Mavis_Desc1[]        = _("Patrols corridors and enforces");
static const u8 sText_Mavis_Desc2[]        = _("curfew. Reports violations.");

static const u8 sText_Orville_Display[]       = _("Orville W.");
static const u8 sText_Orville_FirstName[]     = _("Orville");
static const u8 sText_Orville_LastInitial[]   = _("W");
static const u8 sText_Orville_Assignment[]    = _("Chaplain");
static const u8 sText_Orville_Desc1[]         = _("Provides spiritual counseling.");
static const u8 sText_Orville_Desc2[]         = _("Leads ceremonies and services.");

static const u8 sText_Posey_Display[]       = _("Posey C.");
static const u8 sText_Posey_FirstName[]     = _("Posey");
static const u8 sText_Posey_LastInitial[]   = _("C");
static const u8 sText_Posey_Assignment[]    = _("Botanist");
static const u8 sText_Posey_Desc1[]         = _("Tends hydroponics and crops.");
static const u8 sText_Posey_Desc2[]         = _("Oversees food production.");

static const u8 sText_Susette_Display[]      = _("Susette B.");
static const u8 sText_Susette_FirstName[]    = _("Susette");
static const u8 sText_Susette_LastInitial[]  = _("B");
static const u8 sText_Susette_Assignment[]   = _("Gym Coach");
static const u8 sText_Susette_Desc1[]        = _("Leads fitness programs and");
static const u8 sText_Susette_Desc2[]        = _("sports leagues. Sets standards.");

static const u8 sText_Todd_Display[]        = _("Todd H.");
static const u8 sText_Todd_FirstName[]      = _("Todd");
static const u8 sText_Todd_LastInitial[]    = _("H");
static const u8 sText_Todd_Assignment[]     = _("Surplus");
static const u8 sText_Todd_Desc1[]          = _("See Overseer Clay.");

// Pre-sorted by first name (Clifford < Dorris < Elmer < Herschel < Loretta <
// Mavis < Orville < Posey < Susette < Todd).
static const struct StudentScore sNpcStudents[NUM_TEST_SCORE_NPCS] =
{
    { sText_Clifford_Display,  sText_Clifford_FirstName,  sText_Clifford_LastInitial,
      sText_Clifford_Assignment,  sText_Clifford_Desc1, sText_Clifford_Desc2, NULL,
      6, 3, 8, 2, 3, 4, 2 },
    { sText_Dorris_Display,   sText_Dorris_FirstName,   sText_Dorris_LastInitial,
      sText_Dorris_Assignment,   sText_Dorris_Desc1, sText_Dorris_Desc2, NULL,
      2, 6, 3, 6, 3, 7, 1 },
    { sText_Elmer_Display,  sText_Elmer_FirstName,  sText_Elmer_LastInitial,
      sText_Elmer_Assignment,  sText_Elmer_Desc1, sText_Elmer_Desc2, NULL,
      3, 6, 3, 2, 7, 5, 2 },
    { sText_Herschel_Display, sText_Herschel_FirstName, sText_Herschel_LastInitial,
      sText_Herschel_Assignment, sText_Herschel_Desc1, sText_Herschel_Desc2, NULL,
      1, 6, 3, 3, 9, 2, 4 },
    { sText_Loretta_Display,    sText_Loretta_FirstName,    sText_Loretta_LastInitial,
      sText_Loretta_Assignment,    sText_Loretta_Desc1, sText_Loretta_Desc2, NULL,
      1, 4, 3, 10, 4, 2, 4 },
    { sText_Mavis_Display,  sText_Mavis_FirstName,  sText_Mavis_LastInitial,
      sText_Mavis_Assignment,  sText_Mavis_Desc1, sText_Mavis_Desc2, NULL,
      3, 8, 4, 3, 4, 5, 1 },
    { sText_Orville_Display,   sText_Orville_FirstName,   sText_Orville_LastInitial,
      sText_Orville_Assignment,   sText_Orville_Desc1, sText_Orville_Desc2, NULL,
      1, 4, 3, 9, 8, 1, 2 },
    { sText_Posey_Display,   sText_Posey_FirstName,   sText_Posey_LastInitial,
      sText_Posey_Assignment,   sText_Posey_Desc1, sText_Posey_Desc2, NULL,
      2, 6, 5, 3, 7, 3, 2 },
    { sText_Susette_Display,  sText_Susette_FirstName,  sText_Susette_LastInitial,
      sText_Susette_Assignment,  sText_Susette_Desc1, sText_Susette_Desc2, NULL,
      6, 4, 7, 5, 2, 3, 1 },
    { sText_Todd_Display,    sText_Todd_FirstName,    sText_Todd_LastInitial,
      sText_Todd_Assignment,    sText_Todd_Desc1,    NULL, NULL,  0, 0, 0, 0, 0, 0, 0 },
};

// ----- Player static data -----
static const u8 sText_Player_LastInitial[]  = _("G");
static const u8 sText_Player_LastSuffix[]   = _(" G.");
static const u8 sText_Player_Assignment[]   = _("Surplus");
static const u8 sText_Player_Desc1[]        = _("See Overseer Clay.");

// Player display name is built at prepare time: "<PlayerName> G."
#define PLAYER_DISPLAY_NAME_BUF_SIZE  (PLAYER_NAME_LENGTH + 5)
static EWRAM_DATA u8 sPlayerDisplayName[PLAYER_DISPLAY_NAME_BUF_SIZE] = {0};

// ----- Sub-page row label strings -----
static const u8 sText_TestScores_Header[]   = _("VAULT 42 G.O.A.T. RESULTS");
static const u8 sText_TestScores_Exit[]     = _("EXIT");
static const u8 sText_TestScores_Back[]     = _("BACK");
static const u8 sText_Details_NameLbl[]     = _("NAME: ");
static const u8 sText_Details_AsnLbl[]      = _("JOB: ");
static const u8 sText_Details_StrLbl[]      = _("STRENGTH: ");
static const u8 sText_Details_PerLbl[]      = _("PERCEPTION: ");
static const u8 sText_Details_EndLbl[]      = _("ENDURANCE: ");
static const u8 sText_Details_ChaLbl[]      = _("CHARISMA: ");
static const u8 sText_Details_IntLbl[]      = _("INTELLIGENCE: ");
static const u8 sText_Details_AgiLbl[]      = _("AGILITY: ");
static const u8 sText_Details_LckLbl[]      = _("LUCK: ");

// ----- Sub-page row buffers (built each time prepare runs) -----
#define DETAIL_ROW_BUF_SIZE 32
static EWRAM_DATA u8 sDetails_NameRow[DETAIL_ROW_BUF_SIZE] = {0};
static EWRAM_DATA u8 sDetails_AsnRow[DETAIL_ROW_BUF_SIZE]  = {0};
static EWRAM_DATA u8 sDetails_StrRow[DETAIL_ROW_BUF_SIZE]  = {0};
static EWRAM_DATA u8 sDetails_PerRow[DETAIL_ROW_BUF_SIZE]  = {0};
static EWRAM_DATA u8 sDetails_EndRow[DETAIL_ROW_BUF_SIZE]  = {0};
static EWRAM_DATA u8 sDetails_ChaRow[DETAIL_ROW_BUF_SIZE]  = {0};
static EWRAM_DATA u8 sDetails_IntRow[DETAIL_ROW_BUF_SIZE]  = {0};
static EWRAM_DATA u8 sDetails_AgiRow[DETAIL_ROW_BUF_SIZE]  = {0};
static EWRAM_DATA u8 sDetails_LckRow[DETAIL_ROW_BUF_SIZE]  = {0};

// ----- Page state -----
static EWRAM_DATA u8 sCurrentStudentIdx = 0;
static EWRAM_DATA u8 sTestScores_SavedCursor = 0;
static EWRAM_DATA u8 sTestScores_SavedScroll = 0;
static EWRAM_DATA struct TerminalItem sTerminalTestScores_Items[TEST_SCORES_MAX_ROWS]       = {0};
static EWRAM_DATA struct TerminalItem sTerminalStudentDetails_Items[STUDENT_DETAILS_MAX_ROWS] = {0};
static EWRAM_DATA u8 sRowToStudentIdx[TEST_SCORES_MAX_ROWS] = {0};
static EWRAM_DATA struct StudentScore sPlayerStudentData = {0};

// ----- Forward declarations -----
static void TerminalTestScores_Prepare(void);
static void TerminalStudentDetails_Prepare(void);
static void TestScores_OpenStudentDetails(void);
static void StudentDetails_BackToList(void);

static const struct TerminalPage sTerminalStudentDetails_Page =
{
    .header        = NULL,
    .items         = sTerminalStudentDetails_Items,
    .itemCount     = STUDENT_DETAILS_MAX_ROWS,
    .cols          = 1,
    .layout        = TERMINAL_LAYOUT_SINGLE,
    .prepare       = TerminalStudentDetails_Prepare,
    .createSprites = NULL,
    .onBack        = StudentDetails_BackToList,
};

const struct TerminalPage sTerminalTestScores_Page =
{
    .header        = sText_TestScores_Header,
    .items         = sTerminalTestScores_Items,
    .itemCount     = TEST_SCORES_MAX_ROWS,
    .cols          = 1,
    .layout        = TERMINAL_LAYOUT_SINGLE,
    .prepare       = TerminalTestScores_Prepare,
    .createSprites = NULL,
};

// ----- Helpers -----

static void BuildPlayerDisplayName(void)
{
    StringCopy(sPlayerDisplayName, gSaveBlock2Ptr->playerName);
    StringAppend(sPlayerDisplayName, sText_Player_LastSuffix);
}

static const struct StudentScore *GetStudentData(u8 idx)
{
    if (idx == PLAYER_STUDENT_IDX)
    {
        sPlayerStudentData.displayName     = sPlayerDisplayName;
        sPlayerStudentData.firstName       = gSaveBlock2Ptr->playerName;
        sPlayerStudentData.lastInitial     = sText_Player_LastInitial;
        sPlayerStudentData.assignment      = sText_Player_Assignment;
        sPlayerStudentData.descLine1       = sText_Player_Desc1;
        sPlayerStudentData.descLine2       = NULL;
        sPlayerStudentData.descLine3       = NULL;
        sPlayerStudentData.strength = sPlayerStudentData.perception = sPlayerStudentData.endurance =
        sPlayerStudentData.charisma = sPlayerStudentData.intelligence = sPlayerStudentData.agility =
        sPlayerStudentData.luck = 0;
        return &sPlayerStudentData;
    }
    return &sNpcStudents[idx];
}

static void BuildStatRow(u8 *buf, const u8 *label, u8 value)
{
    u8 *end = StringCopy(buf, label);
    ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 2);
}

// ----- Main list prepare: build sorted student roster -----

static void TerminalTestScores_Prepare(void)
{
    u8 i = 0;
    u8 n;
    bool8 playerInserted = FALSE;

    BuildPlayerDisplayName();

    for (n = 0; n < NUM_TEST_SCORE_NPCS; n++)
    {
        if (!playerInserted)
        {
            s8 cmp = StringCompare(gSaveBlock2Ptr->playerName, sNpcStudents[n].firstName);
            // Player goes before this NPC if: first name < OR (equal first names AND player last initial < NPC last initial)
            if (cmp < 0
             || (cmp == 0 && StringCompare(sText_Player_LastInitial, sNpcStudents[n].lastInitial) < 0))
            {
                sTerminalTestScores_Items[i].type       = TERMINAL_ITEM_SELECTABLE;
                sTerminalTestScores_Items[i].text       = sPlayerDisplayName;
                sTerminalTestScores_Items[i].onActivate = TestScores_OpenStudentDetails;
                sRowToStudentIdx[i] = PLAYER_STUDENT_IDX;
                i++;
                playerInserted = TRUE;
            }
        }
        sTerminalTestScores_Items[i].type       = TERMINAL_ITEM_SELECTABLE;
        sTerminalTestScores_Items[i].text       = sNpcStudents[n].displayName;
        sTerminalTestScores_Items[i].onActivate = TestScores_OpenStudentDetails;
        sRowToStudentIdx[i] = n;
        i++;
    }

    if (!playerInserted)
    {
        sTerminalTestScores_Items[i].type       = TERMINAL_ITEM_SELECTABLE;
        sTerminalTestScores_Items[i].text       = sPlayerDisplayName;
        sTerminalTestScores_Items[i].onActivate = TestScores_OpenStudentDetails;
        sRowToStudentIdx[i] = PLAYER_STUDENT_IDX;
        i++;
    }

    sTerminalTestScores_Items[i].type = TERMINAL_ITEM_BLANK;
    i++;

    sTerminalTestScores_Items[i].type       = TERMINAL_ITEM_SELECTABLE;
    sTerminalTestScores_Items[i].text       = sText_TestScores_Exit;
    sTerminalTestScores_Items[i].onActivate = TerminalContent_ExitCallback;
    i++;

    sTC->activeItemCount = i;
}

static void TestScores_OpenStudentDetails(void)
{
    sTestScores_SavedCursor = sTC->cursorItemIdx;
    sTestScores_SavedScroll = sTC->scrollOffset;
    sCurrentStudentIdx = sRowToStudentIdx[sTC->cursorItemIdx];
    if (sCurrentStudentIdx == PLAYER_STUDENT_IDX)
        FlagSet(FLAG_VIEWED_OWN_TEST_SCORES);
    PlaySE(SE_SELECT);
    TerminalContent_SwapPage(&sTerminalStudentDetails_Page, 0);
}

// ----- Sub-page prepare: per-student details -----

static void TerminalStudentDetails_Prepare(void)
{
    const struct StudentScore *s = GetStudentData(sCurrentStudentIdx);
    u8 i = 0;

    StringCopy(sDetails_NameRow, sText_Details_NameLbl);
    StringAppend(sDetails_NameRow, s->displayName);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_NameRow;
    i++;

    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_BLANK;
    i++;

    StringCopy(sDetails_AsnRow, sText_Details_AsnLbl);
    StringAppend(sDetails_AsnRow, s->assignment);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_AsnRow;
    i++;

    if (s->descLine1) { sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT; sTerminalStudentDetails_Items[i].text = s->descLine1; i++; }
    if (s->descLine2) { sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT; sTerminalStudentDetails_Items[i].text = s->descLine2; i++; }
    if (s->descLine3) { sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT; sTerminalStudentDetails_Items[i].text = s->descLine3; i++; }

    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_BLANK;
    i++;

    BuildStatRow(sDetails_StrRow, sText_Details_StrLbl, s->strength);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_StrRow;
    i++;

    BuildStatRow(sDetails_PerRow, sText_Details_PerLbl, s->perception);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_PerRow;
    i++;

    BuildStatRow(sDetails_EndRow, sText_Details_EndLbl, s->endurance);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_EndRow;
    i++;

    BuildStatRow(sDetails_ChaRow, sText_Details_ChaLbl, s->charisma);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_ChaRow;
    i++;

    BuildStatRow(sDetails_IntRow, sText_Details_IntLbl, s->intelligence);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_IntRow;
    i++;

    BuildStatRow(sDetails_AgiRow, sText_Details_AgiLbl, s->agility);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_AgiRow;
    i++;

    BuildStatRow(sDetails_LckRow, sText_Details_LckLbl, s->luck);
    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_TEXT;
    sTerminalStudentDetails_Items[i].text = sDetails_LckRow;
    i++;

    sTerminalStudentDetails_Items[i].type = TERMINAL_ITEM_BLANK;
    i++;

    sTerminalStudentDetails_Items[i].type       = TERMINAL_ITEM_SELECTABLE;
    sTerminalStudentDetails_Items[i].text       = sText_TestScores_Back;
    sTerminalStudentDetails_Items[i].onActivate = StudentDetails_BackToList;
    i++;

    sTC->activeItemCount = i;
}

static void StudentDetails_BackToList(void)
{
    PlaySE(SE_SELECT);
    TerminalContent_SwapPage(&sTerminalTestScores_Page, sTestScores_SavedCursor);
    TerminalContent_SetScroll(sTestScores_SavedScroll);
}
