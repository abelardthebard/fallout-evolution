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

#include "data/terminal_hack.h"

static const struct BgTemplate sTerminalBgTemplates[3] =
{
    {
        .bg            = T_BG_TEXT,
        .charBaseIndex = 0,
        .mapBaseIndex  = 30,
        .screenSize    = 0,
        .paletteMode   = 0,
        .priority      = 0,
        .baseTile      = 0,
    },
    {
        .bg            = T_BG_SPOTLIGHT,
        .charBaseIndex = 3,
        .mapBaseIndex  = 28,
        .screenSize    = 0,
        .paletteMode   = 0,
        .priority      = 1,
        .baseTile      = 0,
    },
    {
        .bg            = T_BG_IMAGE,
        .charBaseIndex = 2,
        .mapBaseIndex  = 31,
        .screenSize    = 0,
        .paletteMode   = 0,
        .priority      = 2,
        .baseTile      = 0,
    },
};

static const struct WindowTemplate sTerminalWindowTemplates[2] =
{
    {
        .bg          = T_BG_TEXT,
        .tilemapLeft = 0,
        .tilemapTop  = 0,
        .width       = 30,
        .height      = 20,
        .paletteNum  = 15,
        .baseBlock   = 1,
    },
    DUMMY_WIN_TEMPLATE,
};

static const u8 sTerminalTextColors[]          = { 0, 2, 3 };
static const u8 sTerminalTextColors_Inverted[] = { 2, 1, 2 };

static const u8 sText_TerminalScrollUp[]   = _("{UP_ARROW}");
static const u8 sText_TerminalScrollDown[] = _("{DOWN_ARROW}");

static const u32 sBgImageTiles[] = INCBIN_U32("graphics/terminal/background.4bpp");
static const u32 sBgImageMap[]   = INCBIN_U32("graphics/terminal/background.bin");
static const u16 sBgImagePal[]   = INCBIN_U16("graphics/terminal/background.gbapal");

// Chrome tileset must stay below tile 960 -- BG 1's tilemap lives there.
STATIC_ASSERT(sizeof(sBgImageTiles) / 32 < 960, terminalBgImage_tilesetOverlapsMap);

static void TerminalUI_InitBgsAndWindows(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sTerminalBgTemplates, ARRAY_COUNT(sTerminalBgTemplates));
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    InitWindows(sTerminalWindowTemplates);
    DeactivateAllTextPrinters();
    FillBgTilemapBufferRect_Palette0(T_BG_TEXT, 0, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(T_BG_TEXT);
}

static void TerminalUI_LoadPalettes(void)
{
    LoadPalette(GetActiveThemeTextPal(), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadPalette(sBgImagePal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
    PipBoy_ApplyThemeToPalettes(BG_PLTT_ID(0), 16, 0, 0);
}

static void TerminalUI_LoadBgImage(void)
{
    LoadBgTiles(T_BG_IMAGE, sBgImageTiles, sizeof(sBgImageTiles), 0);
    LoadBgTilemap(T_BG_IMAGE, sBgImageMap, sizeof(sBgImageMap), 0);
}

static void TerminalUI_LoadSpotlight(void)
{
    PipBoy_LoadTerminalSpotlight(3, 28);
}

void TerminalUI_ShowSpotlight(const struct SpotlightLayout *layout)
{
    SetGpuReg(REG_OFFSET_WIN0H,
              WIN_RANGE(layout->clipLeft, layout->clipLeft + layout->clipWidth));
    SetGpuReg(REG_OFFSET_WIN0V,
              WIN_RANGE(layout->clipTop,  layout->clipTop  + layout->clipHeight));
    SetGpuReg(REG_OFFSET_WININ,
              WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG2 | WININ_WIN0_OBJ);
    SetGpuReg(REG_OFFSET_WINOUT,
              WINOUT_WIN01_BG0 | WINOUT_WIN01_BG2 | WINOUT_WIN01_OBJ);
    SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON);

    SetGpuReg(REG_OFFSET_BG1HOFS, layout->scrollX);
    SetGpuReg(REG_OFFSET_BG1VOFS, layout->scrollY);
    ShowBg(T_BG_SPOTLIGHT);
}

void TerminalUI_HideSpotlight(void)
{
    HideBg(T_BG_SPOTLIGHT);
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON);
    SetGpuReg(REG_OFFSET_WIN0H,  0);
    SetGpuReg(REG_OFFSET_WIN0V,  0);
    SetGpuReg(REG_OFFSET_WININ,  0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
}

static void TerminalUI_DrawCenteredHeader(u8 winId, const u8 *text)
{
    u16 x = T_MARGIN_X + GetStringCenterAlignXOffset(FONT_NORMAL, text, T_CONTENT_W);
    AddTextPrinterParameterized3(winId, FONT_NORMAL, x, T_HEADER_Y,
        sTerminalTextColors, TEXT_SKIP_DRAW, text);
}


#define TERMINAL_TOTAL_WORDS \
    (ARRAY_COUNT(sTerminalWordsLen4)  + \
     ARRAY_COUNT(sTerminalWordsLen5)  + \
     ARRAY_COUNT(sTerminalWordsLen6)  + \
     ARRAY_COUNT(sTerminalWordsLen7)  + \
     ARRAY_COUNT(sTerminalWordsLen8))

STATIC_ASSERT(TERMINAL_TOTAL_WORDS <= TERMINAL_MAX_WORD_IDS,
              terminalWordPool_exceedsBitfieldCapacity);
STATIC_ASSERT(sizeof(((struct SaveBlock1 *)0)->terminalWordBurned) * 8 == TERMINAL_MAX_WORD_IDS,
              terminalWordBurned_sizeMustMatchMaxWordIds);

#define BOARD_ROWS          T_BODY_ROWS
#define BOARD_COLS          T_BODY_COLS

#define MAX_CANDIDATES      TERMINAL_MAX_WORD_COUNT
#define MAX_BRACKETS        8
#define MIN_BRACKET_SPAN    2
#define MAX_BRACKET_SPAN    10
#define MIN_BRACKET_GAP     4
#define LOG_WORD_BUF_SIZE   (TERMINAL_MAX_WORD_LENGTH + 1)

#define WORD_MIN_GAP        2

#define MAX_WORDS_PER_ROW \
    (((BOARD_COLS) + (WORD_MIN_GAP)) / ((TERMINAL_MIN_WORD_LENGTH) + (WORD_MIN_GAP)))

#define TIER_MAX_QUOTA(WC)            (((WC) + (BOARD_ROWS) - 1) / (BOARD_ROWS))
#define TIER_MAX_SPAN(WC, WLMAX)      (TIER_MAX_QUOTA(WC) * (WLMAX) + (TIER_MAX_QUOTA(WC) - 1) * (WORD_MIN_GAP))
#define TIER_PLACEMENT_FEASIBLE(WC, WLMAX)  (TIER_MAX_SPAN(WC, WLMAX) <= (BOARD_COLS))

STATIC_ASSERT(TIER_PLACEMENT_FEASIBLE(TIER_1_WORD_COUNT, TIER_1_WORD_LEN_MAX), tier1_placement);
STATIC_ASSERT(TIER_PLACEMENT_FEASIBLE(TIER_2_WORD_COUNT, TIER_2_WORD_LEN_MAX), tier2_placement);
STATIC_ASSERT(TIER_PLACEMENT_FEASIBLE(TIER_3_WORD_COUNT, TIER_3_WORD_LEN_MAX), tier3_placement);

#define TERMINAL_IN_PROGRESS  0
#define TERMINAL_WON          1
#define TERMINAL_LOST         2

#define MAX_LOG_ENTRIES      T_BODY_ROWS
#define LOG_LINE_BUF_SIZE    16

struct Candidate
{
    u8  row;
    u8  col;
    u16 wordPoolGlobalId;
    bool8 dudded;
};

struct BracketPair
{
    u8 row;
    u8 openerCol;
    u8 closerCol;
    u8 type;
    bool8 consumed;
};

enum TerminalLogKind
{
    LOG_KIND_GUESS = 0,
    LOG_KIND_DUD_REMOVED,
};

struct TerminalLogEntry
{
    u8 kind;
    u8 word[LOG_WORD_BUF_SIZE];   // GUESS only
    u8 likeness;                  // GUESS only
};

struct TerminalHack
{
    MainCallback exitCb;
    u16 flagId;
    u16 chainContentId;
    u8 tier;
    u8 wordLen;
    u8 wordCount;
    u8 bracketCount;
    u8 cursorCol;
    u8 cursorRow;
    u8 attemptsRemaining;
    u8 passwordCandidateIdx;
    u8 endResult;
    u8 logHead;
    u8 logCount;
    u8 board[BOARD_ROWS * BOARD_COLS];
    struct Candidate candidates[MAX_CANDIDATES];
    struct BracketPair brackets[MAX_BRACKETS];
    struct TerminalLogEntry logEntries[MAX_LOG_ENTRIES];
};

static EWRAM_DATA struct TerminalHack *sHack = NULL;

extern const u8 gFontNormalLatinGlyphWidths[];
extern const u8 gFontNarrowLatinGlyphWidths[];

static void CB2_TerminalHackSetup(void);
static void CB2_TerminalHackMain(void);
static void TerminalHack_VBlank(void);
static void TerminalHack_GenerateBoard(void);
static void TerminalHack_RenderBoard(void);
static void Task_TerminalHackMainInput(u8 taskId);
static void Task_TerminalHackEndState(u8 taskId);
static void Task_TerminalHackFadeOut(u8 taskId);
static void TerminalHack_Teardown(void);
static void AddGuessLogEntry(const struct Candidate *guess, u8 likeness);
static void AddDudRemovedLogEntry(void);

bool8 IsTerminalWordBurned(u16 wordId)
{
    AGB_ASSERT(wordId < TERMINAL_MAX_WORD_IDS);
    if (wordId >= TERMINAL_MAX_WORD_IDS)
        return FALSE;
    return (gSaveBlock1Ptr->terminalWordBurned[wordId >> 3] >> (wordId & 7)) & 1;
}

void BurnTerminalWord(u16 wordId)
{
    AGB_ASSERT(wordId < TERMINAL_MAX_WORD_IDS);
    if (wordId >= TERMINAL_MAX_WORD_IDS)
        return;
    gSaveBlock1Ptr->terminalWordBurned[wordId >> 3] |= 1 << (wordId & 7);
}

void UpdateTerminalLockoutCounter(void)
{
    u16 steps = VarGet(VAR_TERMINAL_LOCKOUT_STEPS);
    if (steps == 0)
        return;

    steps--;
    VarSet(VAR_TERMINAL_LOCKOUT_STEPS, steps);
    if (steps == 0)
        VarSet(VAR_TERMINAL_LOCKED_ID, 0);
}

void StartTerminalHack(void)
{
    sHack = AllocZeroed(sizeof(*sHack));
    sHack->tier           = gSpecialVar_0x8000;
    sHack->flagId         = gSpecialVar_0x8001;
    sHack->chainContentId = gSpecialVar_0x8002;
    sHack->exitCb         = CB2_ReturnToFieldContinueScript;

    AGB_ASSERT(sHack->tier >= TERMINAL_TIER_1 && sHack->tier < NUM_TERMINAL_TIERS);
    AGB_ASSERT(sHack->chainContentId < NUM_TERMINAL_CONTENTS);

    gSpecialVar_Result = 0;

    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    SetMainCallback2(CB2_TerminalHackSetup);
    gMain.state = 0;
}

// `specialvar` writes the return value to VAR_RESULT -- void leaves garbage in r0.
bool8 IsTerminalLockedOut(void)
{
    u16 terminalId = gSpecialVar_0x8000;
    u16 lockedId   = VarGet(VAR_TERMINAL_LOCKED_ID);
    u16 steps      = VarGet(VAR_TERMINAL_LOCKOUT_STEPS);

    return (lockedId == terminalId && steps > 0) ? TRUE : FALSE;
}

static void CB2_TerminalHackSetup(void)
{
    if (gMain.state == 0 && gPaletteFade.active)
    {
        UpdatePaletteFade();
        return;
    }

    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        CpuFill32(0, (void *)VRAM, VRAM_SIZE);
        ClearScheduledBgCopiesToVram();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        ResetPaletteFade();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 1:
        TerminalUI_InitBgsAndWindows();
        gMain.state++;
        break;
    case 2:
        TerminalUI_LoadPalettes();
        TerminalUI_LoadBgImage();
        gMain.state++;
        break;
    case 3:
        TerminalHack_GenerateBoard();
        TerminalHack_RenderBoard();
        gMain.state++;
        break;
    case 4:
        SetVBlankCallback(TerminalHack_VBlank);
        EnableInterrupts(INTR_FLAG_VBLANK);
        ShowBg(T_BG_TEXT);
        ShowBg(T_BG_IMAGE);
        HideBg(T_BG_SPOTLIGHT);
        HideBg(3);
        SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    case 5:
        CreateTask(Task_TerminalHackMainInput, 0);
        SetMainCallback2(CB2_TerminalHackMain);
        break;
    }
}

static void ShuffleU8(u8 *arr, u8 count)
{
    u8 i, j, tmp;
    for (i = count - 1; i > 0; i--)
    {
        j = Random() % (i + 1);
        tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

static void SampleCandidateWords(void)
{
    const struct TerminalWordPool *pool = &sTerminalWordPools[sHack->wordLen - TERMINAL_MIN_WORD_LENGTH];
    u16 available[TERMINAL_MAX_POOL_SAMPLE];
    u8 availCount = 0;
    u8 i;

    for (i = 0; i < pool->count; i++)
    {
        u16 globalId = pool->globalIdBase + i;
        if (!IsTerminalWordBurned(globalId))
        {
            available[availCount++] = i;
            if (availCount == ARRAY_COUNT(available))
                break;
        }
    }

    if (availCount < sHack->wordCount)
    {
        availCount = pool->count;
        for (i = 0; i < pool->count; i++)
            available[i] = i;
    }

    for (i = 0; i < sHack->wordCount; i++)
    {
        u8 j = i + (Random() % (availCount - i));
        u16 tmp = available[i];
        available[i] = available[j];
        available[j] = tmp;
    }

    for (i = 0; i < sHack->wordCount; i++)
    {
        sHack->candidates[i].wordPoolGlobalId = pool->globalIdBase + available[i];
        sHack->candidates[i].dudded = FALSE;
    }

    sHack->passwordCandidateIdx = Random() % sHack->wordCount;
}

static void FillBoardWithGarbage(void)
{
    u16 i;
    for (i = 0; i < BOARD_ROWS * BOARD_COLS; i++)
        sHack->board[i] = sTerminalGarbageChars[Random() % TERMINAL_NUM_GARBAGE_CHARS];
}

// Stars-and-bars random partition across count+1 gap slots.
static void PlaceWordsOnRow(u8 row, u8 count, u8 *outCandIdx)
{
    // {0} silences a spurious -Wmaybe-uninitialized when this inlines.
    u8 gaps[MAX_WORDS_PER_ROW + 1] = {0};
    u8 minSpan = count * sHack->wordLen;
    u8 slack;
    u8 numGaps = count + 1;
    u8 candIdx = *outCandIdx;
    u8 col;
    u8 k, s;

    if (count > 1)
        minSpan += (count - 1) * WORD_MIN_GAP;

    AGB_ASSERT(count <= MAX_WORDS_PER_ROW);
    AGB_ASSERT(minSpan <= BOARD_COLS);
    slack = BOARD_COLS - minSpan;

    for (k = 0; k < numGaps; k++)
        gaps[k] = (k > 0 && k < count) ? WORD_MIN_GAP : 0;
    for (s = 0; s < slack; s++)
        gaps[Random() % numGaps]++;

    col = gaps[0];
    for (k = 0; k < count; k++)
    {
        sHack->candidates[candIdx + k].row = row;
        sHack->candidates[candIdx + k].col = col;
        col += sHack->wordLen;
        if (k + 1 < count)
            col += gaps[k + 1];
    }
    *outCandIdx = candIdx + count;
}

static void AssignWordSlots(void)
{
    u8 rowQuotas[BOARD_ROWS];
    u8 rowOrder[BOARD_ROWS];
    u8 floor  = sHack->wordCount / BOARD_ROWS;
    u8 extras = sHack->wordCount % BOARD_ROWS;
    u8 candIdx = 0;
    u8 r;

    for (r = 0; r < BOARD_ROWS; r++)
    {
        rowOrder[r] = r;
        rowQuotas[r] = floor;
    }
    ShuffleU8(rowOrder, BOARD_ROWS);
    for (r = 0; r < extras; r++)
        rowQuotas[rowOrder[r]] += 1;

    for (r = 0; r < BOARD_ROWS; r++)
    {
        if (rowQuotas[r] > 0)
            PlaceWordsOnRow(r, rowQuotas[r], &candIdx);
    }
    AGB_ASSERT(candIdx == sHack->wordCount);
}

static void WriteWordsToBoard(void)
{
    const struct TerminalWordPool *pool = &sTerminalWordPools[sHack->wordLen - TERMINAL_MIN_WORD_LENGTH];
    u8 i, j;

    for (i = 0; i < sHack->wordCount; i++)
    {
        const struct Candidate *cand = &sHack->candidates[i];
        u16 localIdx = cand->wordPoolGlobalId - pool->globalIdBase;
        const u8 *word = pool->words + localIdx * pool->stride;
        for (j = 0; j < sHack->wordLen; j++)
            sHack->board[cand->row * BOARD_COLS + cand->col + j] = word[j];
    }
}

static bool8 IsGarbageCell(u8 row, u8 col)
{
    u8 i;
    for (i = 0; i < sHack->wordCount; i++)
    {
        const struct Candidate *c = &sHack->candidates[i];
        if (c->row == row && col >= c->col && col < c->col + sHack->wordLen)
            return FALSE;
    }
    for (i = 0; i < sHack->bracketCount; i++)
    {
        const struct BracketPair *b = &sHack->brackets[i];
        if (b->row == row && (col == b->openerCol || col == b->closerCol))
            return FALSE;
    }
    return TRUE;
}

// `existing` is flat [open1, close1, open2, close2, ...].
static bool8 IsValidBracketPlacement(u8 openerCol, u8 closerCol, const u8 *existing, u8 numExisting)
{
    u8 i;
    for (i = 0; i + 1 < numExisting; i += 2)
    {
        u8 eOpen  = existing[i];
        u8 eClose = existing[i + 1];

        if (eOpen  > openerCol && eOpen  < closerCol) return FALSE;
        if (eClose > openerCol && eClose < closerCol) return FALSE;
        if (openerCol > eOpen && openerCol < eClose) return FALSE;
        if (closerCol > eOpen && closerCol < eClose) return FALSE;

        if (closerCol < eOpen && eOpen - closerCol - 1 < MIN_BRACKET_GAP) return FALSE;
        if (openerCol > eClose && openerCol - eClose - 1 < MIN_BRACKET_GAP) return FALSE;
    }
    return TRUE;
}

static bool8 HasWordCellBetween(u8 row, u8 openerCol, u8 closerCol)
{
    u8 c, w;
    for (c = openerCol + 1; c < closerCol; c++)
    {
        for (w = 0; w < sHack->wordCount; w++)
        {
            const struct Candidate *cand = &sHack->candidates[w];
            if (cand->row == row && c >= cand->col && c < cand->col + sHack->wordLen)
                return TRUE;
        }
    }
    return FALSE;
}

static bool8 IsBracketSpanUnique(u8 span)
{
    u8 i;
    for (i = 0; i < sHack->bracketCount; i++)
    {
        const struct BracketPair *bp = &sHack->brackets[i];
        if ((u8)(bp->closerCol - bp->openerCol + 1) == span)
            return FALSE;
    }
    return TRUE;
}

static bool8 IsCandidateBracketValid(u8 row, u8 openerCol, u8 closerCol,
                                     const u8 *rowExisting, u8 numRowExisting)
{
    u8 span = closerCol - openerCol + 1;
    if (span < MIN_BRACKET_SPAN || span > MAX_BRACKET_SPAN) return FALSE;
    if (!IsBracketSpanUnique(span))                        return FALSE;
    if (!IsValidBracketPlacement(openerCol, closerCol, rowExisting, numRowExisting)) return FALSE;
    if (HasWordCellBetween(row, openerCol, closerCol))     return FALSE;
    return TRUE;
}

static bool8 TryPlaceBracketOnRow(u8 row, u8 type)
{
    u8 garbageCols[BOARD_COLS];
    u8 numGarbage = 0;
    u8 existing[MAX_BRACKETS * 2];
    u8 numExisting = 0;
    u16 validCount = 0;
    u16 pick, idx;
    u8 col, i, j;

    for (col = 0; col < BOARD_COLS; col++)
    {
        if (IsGarbageCell(row, col))
            garbageCols[numGarbage++] = col;
    }
    if (numGarbage < 2)
        return FALSE;

    for (i = 0; i < sHack->bracketCount; i++)
    {
        const struct BracketPair *b = &sHack->brackets[i];
        if (b->row == row)
        {
            existing[numExisting++] = b->openerCol;
            existing[numExisting++] = b->closerCol;
        }
    }

    for (i = 0; i + 1 < numGarbage; i++)
    {
        for (j = i + 1; j < numGarbage; j++)
        {
            if (IsCandidateBracketValid(row, garbageCols[i], garbageCols[j], existing, numExisting))
                validCount++;
        }
    }
    if (validCount == 0)
        return FALSE;

    pick = Random() % validCount;
    idx = 0;
    for (i = 0; i + 1 < numGarbage; i++)
    {
        for (j = i + 1; j < numGarbage; j++)
        {
            u8 openerCol = garbageCols[i];
            u8 closerCol = garbageCols[j];
            if (!IsCandidateBracketValid(row, openerCol, closerCol, existing, numExisting))
                continue;
            if (idx == pick)
            {
                struct BracketPair *bp = &sHack->brackets[sHack->bracketCount];
                bp->row       = row;
                bp->openerCol = openerCol;
                bp->closerCol = closerCol;
                bp->type      = type;
                bp->consumed  = FALSE;
                sHack->board[row * BOARD_COLS + openerCol] = sTerminalBracketOpeners[type];
                sHack->board[row * BOARD_COLS + closerCol] = sTerminalBracketClosers[type];
                sHack->bracketCount++;
                return TRUE;
            }
            idx++;
        }
    }
    return FALSE;
}

static void PlaceBracketPairs(u8 numPairs)
{
    u8 i;
    sHack->bracketCount = 0;

    for (i = 0; i < numPairs; i++)
    {
        u8 type = Random() % TERMINAL_NUM_BRACKET_TYPES;
        u8 rowOrder[BOARD_ROWS];
        u8 r;
        bool8 placed = FALSE;

        for (r = 0; r < BOARD_ROWS; r++)
            rowOrder[r] = r;
        ShuffleU8(rowOrder, BOARD_ROWS);

        for (r = 0; r < BOARD_ROWS; r++)
        {
            if (TryPlaceBracketOnRow(rowOrder[r], type))
            {
                placed = TRUE;
                break;
            }
        }
        if (!placed)
            break;
    }
}

static void TerminalHack_GenerateBoard(void)
{
    const struct TerminalDifficulty *tier = &sTerminalDifficulty[sHack->tier];

    sHack->wordLen     = tier->wordLenMin + (Random() % (tier->wordLenMax - tier->wordLenMin + 1));
    sHack->wordCount   = tier->wordCount;
    sHack->cursorCol   = 0;
    sHack->cursorRow   = 0;
    sHack->attemptsRemaining = TERMINAL_MAX_ATTEMPTS;

    SampleCandidateWords();
    FillBoardWithGarbage();
    AssignWordSlots();
    WriteWordsToBoard();

    PlaceBracketPairs(tier->bracketPairs);
}

static const u8 sText_AttemptsPrefix[] = _("ATTEMPTS: ");
static const u8 sText_AttemptMark[]    = _(" X");
static const u8 sText_AttemptGone[]    = _("  ");
static const u8 sText_AccessGranted[]  = _("ACCESS GRANTED - PRESS A");
static const u8 sText_AccessDenied[]   = _("ACCESS DENIED - PRESS A");
static const u8 sText_LogPrompt[]      = _(">");
static const u8 sText_LogLikeness[]    = _("LIKENESS=");
static const u8 sText_LogDudRemoved[]  = _(">DUD REMOVED");

static s8 FindLiveCandidateAt(u8 row, u8 col)
{
    u8 i;
    for (i = 0; i < sHack->wordCount; i++)
    {
        const struct Candidate *c = &sHack->candidates[i];
        if (c->dudded)
            continue;
        if (c->row == row && col >= c->col && col < c->col + sHack->wordLen)
            return (s8)i;
    }
    return -1;
}

static s8 FindLiveBracketEndpointAt(u8 row, u8 col)
{
    u8 i;
    for (i = 0; i < sHack->bracketCount; i++)
    {
        const struct BracketPair *bp = &sHack->brackets[i];
        if (bp->consumed)
            continue;
        if (bp->row == row && (col == bp->openerCol || col == bp->closerCol))
            return (s8)i;
    }
    return -1;
}

static bool8 GetWordSpanAt(u8 row, u8 col, u8 *outStartCol, u8 *outEndCol)
{
    s8 idx = FindLiveCandidateAt(row, col);
    if (idx < 0)
        return FALSE;
    *outStartCol = sHack->candidates[idx].col;
    *outEndCol = sHack->candidates[idx].col + sHack->wordLen - 1;
    return TRUE;
}

static bool8 GetEmptyBracketSpanAt(u8 row, u8 col, u8 *outStartCol, u8 *outEndCol)
{
    s8 bidx = FindLiveBracketEndpointAt(row, col);
    const struct BracketPair *bp;
    if (bidx < 0)
        return FALSE;
    bp = &sHack->brackets[bidx];
    if (bp->closerCol - bp->openerCol != 1)
        return FALSE;
    *outStartCol = bp->openerCol;
    *outEndCol = bp->closerCol;
    return TRUE;
}

static bool8 GetBlankSpanAt(u8 row, u8 col, u8 *outStartCol, u8 *outEndCol)
{
    s16 c;
    if (sHack->board[row * BOARD_COLS + col] != CHAR_SPACE)
        return FALSE;

    c = col;
    while (c > 0 && sHack->board[row * BOARD_COLS + (c - 1)] == CHAR_SPACE)
        c--;
    *outStartCol = (u8)c;

    c = col;
    while (c < BOARD_COLS - 1 && sHack->board[row * BOARD_COLS + (c + 1)] == CHAR_SPACE)
        c++;
    *outEndCol = (u8)c;
    return TRUE;
}

static void ComputeHighlightSpan(u8 *outStartCol, u8 *outEndCol)
{
    u8 col = sHack->cursorCol;
    u8 row = sHack->cursorRow;
    s8 bracketIdx;

    if (GetWordSpanAt(row, col, outStartCol, outEndCol))
        return;

    if (GetBlankSpanAt(row, col, outStartCol, outEndCol))
        return;

    bracketIdx = FindLiveBracketEndpointAt(row, col);
    if (bracketIdx >= 0)
    {
        *outStartCol = sHack->brackets[bracketIdx].openerCol;
        *outEndCol = sHack->brackets[bracketIdx].closerCol;
        return;
    }

    *outStartCol = col;
    *outEndCol = col;
}

static u8 ComputeLikeness(const struct Candidate *guess, const struct Candidate *password)
{
    const u8 *guessChars    = &sHack->board[guess->row * BOARD_COLS + guess->col];
    const u8 *passwordChars = &sHack->board[password->row * BOARD_COLS + password->col];
    u8 likeness = 0;
    u8 i;
    for (i = 0; i < sHack->wordLen; i++)
        if (guessChars[i] == passwordChars[i])
            likeness++;
    return likeness;
}

static void EraseBoardCellsWithSpace(u8 row, u8 startCol, u8 endCol)
{
    u8 col;
    for (col = startCol; col <= endCol; col++)
        sHack->board[row * BOARD_COLS + col] = CHAR_SPACE;
}

static void DudCandidate(u8 candidateIdx)
{
    struct Candidate *c = &sHack->candidates[candidateIdx];
    if (c->dudded)
        return;
    c->dudded = TRUE;
    EraseBoardCellsWithSpace(c->row, c->col, c->col + sHack->wordLen - 1);
}

static void ConsumeBracket(u8 bracketIdx)
{
    struct BracketPair *bp = &sHack->brackets[bracketIdx];
    if (bp->consumed)
        return;
    bp->consumed = TRUE;
    EraseBoardCellsWithSpace(bp->row, bp->openerCol, bp->closerCol);
}

static void ApplyDudRemovalEffect(void)
{
    u8 eligible[MAX_CANDIDATES];
    u8 eligibleCount = 0;
    u8 i;
    for (i = 0; i < sHack->wordCount; i++)
    {
        if (i == sHack->passwordCandidateIdx)
            continue;
        if (sHack->candidates[i].dudded)
            continue;
        eligible[eligibleCount++] = i;
    }
    if (eligibleCount == 0)
        return;
    DudCandidate(eligible[Random() % eligibleCount]);
    AddDudRemovedLogEntry();
}

static struct TerminalLogEntry *AcquireLogSlot(void)
{
    struct TerminalLogEntry *entry = &sHack->logEntries[sHack->logHead];
    sHack->logHead = (sHack->logHead + 1) % MAX_LOG_ENTRIES;
    if (sHack->logCount < MAX_LOG_ENTRIES)
        sHack->logCount++;
    return entry;
}

static void AddGuessLogEntry(const struct Candidate *guess, u8 likeness)
{
    struct TerminalLogEntry *entry = AcquireLogSlot();
    const u8 *src = &sHack->board[guess->row * BOARD_COLS + guess->col];
    u8 i;
    entry->kind = LOG_KIND_GUESS;
    for (i = 0; i < sHack->wordLen; i++)
        entry->word[i] = src[i];
    entry->word[sHack->wordLen] = EOS;
    entry->likeness = likeness;
}

static void AddDudRemovedLogEntry(void)
{
    struct TerminalLogEntry *entry = AcquireLogSlot();
    entry->kind = LOG_KIND_DUD_REMOVED;
}

static void EnterEndState(u8 taskId)
{
    TerminalHack_RenderBoard();
    gTasks[taskId].func = Task_TerminalHackEndState;
}

static void TryGuessWordAtCursor(u8 taskId, u8 guessIdx)
{
    const struct Candidate *guess    = &sHack->candidates[guessIdx];
    const struct Candidate *password = &sHack->candidates[sHack->passwordCandidateIdx];
    u8 likeness = ComputeLikeness(guess, password);
    AddGuessLogEntry(guess, likeness);

    if (likeness == sHack->wordLen)
    {
        BurnTerminalWord(password->wordPoolGlobalId);
        sHack->endResult = TERMINAL_WON;
        PlaySE(SE_SUCCESS);
        EnterEndState(taskId);
        return;
    }

    PlaySE(SE_FAILURE);
    DudCandidate(guessIdx);
    if (sHack->attemptsRemaining > 0)
        sHack->attemptsRemaining--;
    if (sHack->attemptsRemaining == 0)
    {
        sHack->endResult = TERMINAL_LOST;
        VarSet(VAR_TERMINAL_LOCKED_ID, sHack->flagId);
        VarSet(VAR_TERMINAL_LOCKOUT_STEPS, sTerminalDifficulty[sHack->tier].lockoutSteps);
        EnterEndState(taskId);
        return;
    }
    TerminalHack_RenderBoard();
}

static void TryActivateBracketAtCursor(u8 bracketIdx)
{
    ApplyDudRemovalEffect();
    ConsumeBracket(bracketIdx);
    PlaySE(SE_SUCCESS);
    TerminalHack_RenderBoard();
}

static void ActivateAtCursor(u8 taskId)
{
    u8 row = sHack->cursorRow;
    u8 col = sHack->cursorCol;
    s8 idx;

    idx = FindLiveCandidateAt(row, col);
    if (idx >= 0)
    {
        TryGuessWordAtCursor(taskId, (u8)idx);
        return;
    }

    idx = FindLiveBracketEndpointAt(row, col);
    if (idx >= 0)
    {
        TryActivateBracketAtCursor((u8)idx);
        return;
    }

    PlaySE(SE_FAILURE);
}

static void BuildAttemptsHeader(u8 *out)
{
    u8 *dst = StringCopy(out, sText_AttemptsPrefix);
    u8 i;
    for (i = 0; i < TERMINAL_MAX_ATTEMPTS; i++)
        dst = StringCopy(dst, (i < sHack->attemptsRemaining) ? sText_AttemptMark : sText_AttemptGone);
}

static void RenderHeader(void)
{
    if (sHack->endResult == TERMINAL_WON)
    {
        TerminalUI_DrawCenteredHeader(T_WIN_TEXT,sText_AccessGranted);
    }
    else if (sHack->endResult == TERMINAL_LOST)
    {
        TerminalUI_DrawCenteredHeader(T_WIN_TEXT,sText_AccessDenied);
    }
    else
    {
        u8 buf[24];
        BuildAttemptsHeader(buf);
        TerminalUI_DrawCenteredHeader(T_WIN_TEXT,buf);
    }
}

static u8 LogEntryRowCount(const struct TerminalLogEntry *entry)
{
    return (entry->kind == LOG_KIND_GUESS) ? 2 : 1;
}

static void RenderLogPanel(void)
{
    u8 firstIdx;
    u8 visibleRows = 0;
    u8 visibleCount = 0;
    u8 drawRow;
    s8 i;

    if (sHack->logCount == 0)
    {
        u16 yTop = T_LOG_Y + (T_BODY_ROWS - 1) * T_ROW_HEIGHT;
        AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NARROW, T_LOG_X, yTop,
            sTerminalTextColors, TEXT_SKIP_DRAW, sText_LogPrompt);
        return;
    }

    firstIdx = (sHack->logCount < MAX_LOG_ENTRIES) ? 0 : sHack->logHead;

    for (i = (s8)sHack->logCount - 1; i >= 0; i--)
    {
        u8 idx = (firstIdx + (u8)i) % MAX_LOG_ENTRIES;
        u8 rows = LogEntryRowCount(&sHack->logEntries[idx]);
        if (visibleRows + rows > T_BODY_ROWS)
            break;
        visibleRows += rows;
        visibleCount++;
    }

    drawRow = T_BODY_ROWS - visibleRows;

    for (i = (s8)sHack->logCount - (s8)visibleCount; i < (s8)sHack->logCount; i++)
    {
        u8 idx = (firstIdx + (u8)i) % MAX_LOG_ENTRIES;
        const struct TerminalLogEntry *entry = &sHack->logEntries[idx];
        u16 yTop = T_LOG_Y + drawRow * T_ROW_HEIGHT;
        u8 buf[LOG_LINE_BUF_SIZE];
        u8 *dst;

        switch (entry->kind)
        {
        case LOG_KIND_GUESS:
            dst = StringCopy(buf, sText_LogPrompt);
            StringCopy(dst, entry->word);
            AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NARROW, T_LOG_X, yTop,
                sTerminalTextColors, TEXT_SKIP_DRAW, buf);

            dst = StringCopy(buf, sText_LogLikeness);
            ConvertIntToDecimalStringN(dst, entry->likeness, STR_CONV_MODE_LEFT_ALIGN, 2);
            AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NARROW, T_LOG_X, yTop + T_ROW_HEIGHT,
                sTerminalTextColors, TEXT_SKIP_DRAW, buf);
            break;

        case LOG_KIND_DUD_REMOVED:
            AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NARROW, T_LOG_X, yTop,
                sTerminalTextColors, TEXT_SKIP_DRAW, sText_LogDudRemoved);
            break;
        }

        drawRow += LogEntryRowCount(entry);
    }
}

static void TerminalHack_RenderBoard(void)
{
    u8 charBuf[2];
    u8 row, col;
    u8 hlStart, hlEnd;

    FillWindowPixelBuffer(T_WIN_TEXT, PIXEL_FILL(0));

    RenderHeader();

    ComputeHighlightSpan(&hlStart, &hlEnd);

    charBuf[1] = EOS;
    for (row = 0; row < BOARD_ROWS; row++)
    {
        for (col = 0; col < BOARD_COLS; col++)
        {
            u8 ch = sHack->board[row * BOARD_COLS + col];
            u8 font, width, xOffset;
            const u8 *colors;
            bool8 highlighted = (sHack->endResult == TERMINAL_IN_PROGRESS)
                             && (row == sHack->cursorRow && col >= hlStart && col <= hlEnd);
            u16 cellX = T_BODY_X + col * T_CELL_W;
            u16 cellY = T_BODY_Y + row * T_ROW_HEIGHT;

            if (highlighted)
                FillWindowPixelRect(T_WIN_TEXT, PIXEL_FILL(2), cellX, cellY, T_CELL_W, T_ROW_HEIGHT);
            colors = highlighted ? sTerminalTextColors_Inverted : sTerminalTextColors;

            if (ch >= 0xBB && ch <= 0xD4)
            {
                font = FONT_NORMAL;
                width = gFontNormalLatinGlyphWidths[ch];
            }
            else
            {
                font = FONT_NARROW;
                width = gFontNarrowLatinGlyphWidths[ch];
            }
            xOffset = (width < T_CELL_W) ? (T_CELL_W - width + 1) / 2 : 0;
            charBuf[0] = ch;
            AddTextPrinterParameterized3(T_WIN_TEXT, font,
                cellX + xOffset, cellY,
                colors, TEXT_SKIP_DRAW, charBuf);
        }
    }

    RenderLogPanel();

    PutWindowTilemap(T_WIN_TEXT);
    CopyWindowToVram(T_WIN_TEXT, COPYWIN_FULL);
    CopyBgTilemapBufferToVram(T_BG_TEXT);
}

static void MoveCursor(s8 dx, s8 dy)
{
    s16 newCol = sHack->cursorCol;
    s16 newRow = sHack->cursorRow;

    if (dx != 0)
    {
        u8 spanStart, spanEnd;
        if (GetWordSpanAt(sHack->cursorRow, sHack->cursorCol, &spanStart, &spanEnd)
         || GetBlankSpanAt(sHack->cursorRow, sHack->cursorCol, &spanStart, &spanEnd)
         || GetEmptyBracketSpanAt(sHack->cursorRow, sHack->cursorCol, &spanStart, &spanEnd))
            newCol = (dx > 0) ? (spanEnd + 1) : (spanStart - 1);
        else
            newCol = (s16)sHack->cursorCol + dx;

        if (newCol < 0)
            newCol = BOARD_COLS - 1;
        else if (newCol >= BOARD_COLS)
            newCol = 0;
    }
    if (dy != 0)
    {
        newRow = (s16)sHack->cursorRow + dy;
        if (newRow < 0)
            newRow = BOARD_ROWS - 1;
        else if (newRow >= BOARD_ROWS)
            newRow = 0;
    }

    if ((u8)newCol == sHack->cursorCol && (u8)newRow == sHack->cursorRow)
        return;

    sHack->cursorCol = (u8)newCol;
    sHack->cursorRow = (u8)newRow;
    TerminalHack_RenderBoard();
}

static void CB2_TerminalHackMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void TerminalHack_VBlank(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Task_TerminalHackMainInput(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    if (JOY_REPEAT(DPAD_LEFT))       { PlaySE(SE_SELECT); MoveCursor(-1,  0); }
    else if (JOY_REPEAT(DPAD_RIGHT)) { PlaySE(SE_SELECT); MoveCursor( 1,  0); }
    else if (JOY_REPEAT(DPAD_UP))    { PlaySE(SE_SELECT); MoveCursor( 0, -1); }
    else if (JOY_REPEAT(DPAD_DOWN))  { PlaySE(SE_SELECT); MoveCursor( 0,  1); }

    if (JOY_NEW(A_BUTTON))
        ActivateAtCursor(taskId);
}

static void Task_TerminalHackEndState(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSpecialVar_Result = (sHack->endResult == TERMINAL_WON) ? 1 : 0;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_TerminalHackFadeOut;
    }
}

static void Task_TerminalHackFadeOut(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    // Skip overworld CB2 to keep screen black between hack and content.
    if (sHack->endResult == TERMINAL_WON)
    {
        u16 contentId = sHack->chainContentId;
        TerminalHack_Teardown();
        DestroyTask(taskId);
        gSpecialVar_0x8000 = contentId;
        ShowTerminalContent();
        return;
    }

    SetMainCallback2(sHack->exitCb);
    TerminalHack_Teardown();
    DestroyTask(taskId);
}

static void TerminalHack_Teardown(void)
{
    FreeAllWindowBuffers();
    if (sHack != NULL)
    {
        Free(sHack);
        sHack = NULL;
    }
}


EWRAM_DATA struct TerminalContentState *sTC = NULL;

static void CB2_TerminalContentSetup(void);
static void CB2_TerminalContentMain(void);
static void TerminalContent_VBlank(void);
static void Task_TerminalContentInput(u8 taskId);
static void Task_TerminalContentFadeOut(u8 taskId);
void TerminalContent_Teardown(void);
static void TerminalContent_RenderPage(void);
static void TerminalContent_RenderBody(void);
static void TerminalContent_DrawCursor(u8 itemIdx);
static void TerminalContent_EraseCursor(u8 itemIdx);
static void TerminalContent_SetCursor(u8 newIdx);
static s8 TerminalContent_FindSelectable(s8 fromIdx, s8 direction);
static s8 TerminalContent_FindRowByCallback(TerminalItemCallback cb);
void TerminalContent_ExitCallback(void);
void TerminalContent_CreatePlayerSprite(u16 x, u16 y);
static void TerminalContent_CreatePlayerSpriteAs(u16 x, u16 y, u8 gender);

extern const struct TerminalPage *const gTerminalContents[NUM_TERMINAL_CONTENTS];

void EnterTerminalContent(const struct TerminalPage *page, u8 initialCursorIdx)
{
    AGB_ASSERT(page != NULL);
    AGB_ASSERT(page->cols > 0);

    sTC = AllocZeroed(sizeof(*sTC));
    sTC->exitCb = CB2_ReturnToFieldContinueScript;
    sTC->page = page;
    sTC->currentHeader = page->header;
    sTC->activeItems = page->items;
    sTC->activeItemCount = page->itemCount;
    sTC->activeCols = page->cols;
    sTC->playerSpriteId = MAX_SPRITES;

    if (page->prepare != NULL)
        page->prepare();

    if (initialCursorIdx < sTC->activeItemCount
        && sTC->activeItems[initialCursorIdx].type == TERMINAL_ITEM_SELECTABLE)
    {
        sTC->cursorItemIdx = initialCursorIdx;
    }
    else
    {
        s8 idx = TerminalContent_FindSelectable(0, 1);
        sTC->cursorItemIdx = (idx < 0) ? 0 : (u8)idx;
    }

    // Cancel pending fade-in to avoid one-frame field flash.
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetPaletteFade();
    SetMainCallback2(CB2_TerminalContentSetup);
    gMain.state = 0;
}

void ShowTerminalContent(void)
{
    u16 contentId = gSpecialVar_0x8000;
    AGB_ASSERT(contentId < NUM_TERMINAL_CONTENTS);
    EnterTerminalContent(gTerminalContents[contentId], 0xFF);
}

static void CB2_TerminalContentSetup(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        CpuFill32(0, (void *)VRAM, VRAM_SIZE);
        ClearScheduledBgCopiesToVram();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        ResetPaletteFade();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 1:
        TerminalUI_InitBgsAndWindows();
        gMain.state++;
        break;
    case 2:
        TerminalUI_LoadPalettes();
        TerminalUI_LoadBgImage();
        TerminalUI_LoadSpotlight();
        gMain.state++;
        break;
    case 3:
        TerminalContent_RenderPage();
        if (sTC->page->createSprites != NULL)
            sTC->page->createSprites();
        gMain.state++;
        break;
    case 4:
        SetVBlankCallback(TerminalContent_VBlank);
        EnableInterrupts(INTR_FLAG_VBLANK);
        ShowBg(T_BG_TEXT);
        ShowBg(T_BG_IMAGE);
        HideBg(3);
        SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        sTC->taskId = CreateTask(Task_TerminalContentInput, 0);
        SetMainCallback2(CB2_TerminalContentMain);
        break;
    }
}

static void CB2_TerminalContentMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void TerminalContent_VBlank(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static u16 TerminalContent_BodyWidth(void)
{
    return (sTC->page->layout == TERMINAL_LAYOUT_SINGLE) ? T_CONTENT_W : T_BODY_W;
}

static void TerminalContent_ItemPos(u8 itemIdx, u16 *outX, u16 *outY)
{
    u8 cols = sTC->activeCols;
    u8 row  = itemIdx / cols;
    u8 col  = itemIdx % cols;
    u16 colW = TerminalContent_BodyWidth() / cols;
    *outX = T_BODY_X + col * colW;
    *outY = T_BODY_Y + (row - sTC->scrollOffset) * T_ROW_HEIGHT;
}

static u8 TerminalContent_TotalRows(void)
{
    return (sTC->activeItemCount + sTC->activeCols - 1) / sTC->activeCols;
}

static bool8 TerminalContent_IsItemVisible(u8 itemIdx)
{
    u8 row = itemIdx / sTC->activeCols;
    return row >= sTC->scrollOffset
        && row <  sTC->scrollOffset + T_BODY_ROWS;
}

static bool8 TerminalContent_IsScrollable(void)
{
    return TerminalContent_TotalRows() > T_BODY_ROWS;
}

#define T_SCROLL_ARROW_X   (T_MARGIN_X + T_CONTENT_W - 12)

static void TerminalContent_RenderBody(void)
{
    const struct TerminalItem *items = sTC->activeItems;
    u8 count = sTC->activeItemCount;
    u8 i;

    FillWindowPixelBuffer(T_WIN_TEXT, PIXEL_FILL(0));

    if (sTC->currentHeader != NULL)
        TerminalUI_DrawCenteredHeader(T_WIN_TEXT, sTC->currentHeader);

    for (i = 0; i < count; i++)
    {
        const struct TerminalItem *item = &items[i];
        u16 x, y;

        if (!TerminalContent_IsItemVisible(i))
            continue;
        TerminalContent_ItemPos(i, &x, &y);

        switch (item->type)
        {
        case TERMINAL_ITEM_TEXT:
            if (item->text != NULL)
                AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL, x, y,
                    sTerminalTextColors, TEXT_SKIP_DRAW, item->text);
            break;
        case TERMINAL_ITEM_SELECTABLE:
            if (item->text != NULL)
                AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL,
                    x + TC_CURSOR_COL_W, y,
                    sTerminalTextColors, TEXT_SKIP_DRAW, item->text);
            break;
        case TERMINAL_ITEM_SPRITE:
        case TERMINAL_ITEM_BLANK:
        default:
            break;
        }
    }

    if (TerminalContent_IsScrollable())
    {
        if (sTC->scrollOffset > 0)
            AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL,
                T_SCROLL_ARROW_X, T_BODY_Y,
                sTerminalTextColors, TEXT_SKIP_DRAW, sText_TerminalScrollUp);
        if (sTC->scrollOffset + T_BODY_ROWS < TerminalContent_TotalRows())
            AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL,
                T_SCROLL_ARROW_X, T_BODY_Y + (T_BODY_ROWS - 1) * T_ROW_HEIGHT,
                sTerminalTextColors, TEXT_SKIP_DRAW, sText_TerminalScrollDown);
    }

    TerminalContent_DrawCursor(sTC->cursorItemIdx);

    PutWindowTilemap(T_WIN_TEXT);
    CopyWindowToVram(T_WIN_TEXT, COPYWIN_FULL);
    CopyBgTilemapBufferToVram(T_BG_TEXT);
}

static void TerminalContent_RenderPage(void)
{
    const struct TerminalItem *items = sTC->activeItems;
    u8 count = sTC->activeItemCount;
    u8 i;

    TerminalContent_RenderBody();

    for (i = 0; i < count && i < T_BODY_ROWS; i++)
    {
        const struct TerminalItem *item = &items[i];
        if (item->type == TERMINAL_ITEM_SPRITE
            && item->sprite != NULL && item->sprite->template != NULL
            && sTC->spriteCount < ARRAY_COUNT(sTC->spriteIds))
        {
            u8 spriteId = CreateSprite(item->sprite->template,
                T_BODY_X + item->sprite->dx,
                T_BODY_Y + item->sprite->dy,
                0);
            if (spriteId != MAX_SPRITES)
                sTC->spriteIds[sTC->spriteCount++] = spriteId;
        }
    }
}

static void TerminalContent_DrawCursor(u8 itemIdx)
{
    u16 x, y;
    if (itemIdx >= sTC->activeItemCount)
        return;
    if (sTC->activeItems[itemIdx].type != TERMINAL_ITEM_SELECTABLE)
        return;
    if (!TerminalContent_IsItemVisible(itemIdx))
        return;
    TerminalContent_ItemPos(itemIdx, &x, &y);
    AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL, x, y,
        sTerminalTextColors, TEXT_SKIP_DRAW, gText_SelectorArrow3);
}

static void TerminalContent_EraseCursor(u8 itemIdx)
{
    u16 x, y;
    if (itemIdx >= sTC->activeItemCount)
        return;
    if (!TerminalContent_IsItemVisible(itemIdx))
        return;
    TerminalContent_ItemPos(itemIdx, &x, &y);
    FillWindowPixelRect(T_WIN_TEXT, PIXEL_FILL(0), x, y, TC_CURSOR_COL_W, T_ROW_HEIGHT);
}

static s8 TerminalContent_FindSelectable(s8 fromIdx, s8 direction)
{
    u8 count = sTC->activeItemCount;
    s8 i = fromIdx;
    u8 tried = 0;

    while (tried < count)
    {
        if (i < 0)
            i = count - 1;
        else if (i >= (s8)count)
            i = 0;
        if (sTC->activeItems[i].type == TERMINAL_ITEM_SELECTABLE)
            return i;
        i += direction;
        tried++;
    }
    return -1;
}

static s8 TerminalContent_FindRowByCallback(TerminalItemCallback cb)
{
    u8 i;
    if (cb == NULL)
        return -1;
    for (i = 0; i < sTC->activeItemCount; i++)
    {
        if (sTC->activeItems[i].type == TERMINAL_ITEM_SELECTABLE
            && sTC->activeItems[i].onActivate == cb)
            return (s8)i;
    }
    return -1;
}

static void TerminalContent_SetCursor(u8 newIdx)
{
    u8 newRow;
    bool8 scrollChanged = FALSE;

    if (newIdx == sTC->cursorItemIdx || newIdx >= sTC->activeItemCount)
        return;
    PlaySE(SE_SELECT);

    newRow = newIdx / sTC->activeCols;
    if (newRow < sTC->scrollOffset)
    {
        sTC->scrollOffset = newRow;
        scrollChanged = TRUE;
    }
    else if (newRow >= sTC->scrollOffset + T_BODY_ROWS)
    {
        sTC->scrollOffset = newRow - T_BODY_ROWS + 1;
        scrollChanged = TRUE;
    }

    if (scrollChanged)
    {
        sTC->cursorItemIdx = newIdx;
        TerminalContent_RenderBody();
    }
    else
    {
        TerminalContent_EraseCursor(sTC->cursorItemIdx);
        sTC->cursorItemIdx = newIdx;
        TerminalContent_DrawCursor(sTC->cursorItemIdx);
        CopyWindowToVram(T_WIN_TEXT, COPYWIN_GFX);
    }

    if (sTC->onCursorMove != NULL)
        sTC->onCursorMove(newIdx);
}

// Mirrors vanilla ChangeMenuGridCursorPosition. Returns TRUE if cursor moved.
static bool8 TerminalContent_MoveCursorGrid(s8 dx, s8 dy)
{
    u8 cols  = sTC->activeCols;
    u8 count = sTC->activeItemCount;
    u8 rows  = (count + cols - 1) / cols;
    u8 startIdx = sTC->cursorItemIdx;
    u8 row = startIdx / cols;
    u8 col = startIdx % cols;
    u8 idx = startIdx;
    u8 tried;

    if (dx != 0)
    {
        u8 rowStart = row * cols;
        u8 rowWidth = (rowStart + cols > count) ? (count - rowStart) : cols;
        if (rowWidth <= 1)
            return FALSE;

        for (tried = 0; tried < rowWidth; tried++)
        {
            s8 next = (s8)col + dx;
            if (next < 0)                col = rowWidth - 1;
            else if (next >= rowWidth)   col = 0;
            else                         col = (u8)next;
            idx = rowStart + col;
            if (sTC->activeItems[idx].type == TERMINAL_ITEM_SELECTABLE)
                break;
        }
        if (tried >= rowWidth)
            return FALSE;
    }
    else if (dy != 0)
    {
        for (tried = 0; tried < rows; tried++)
        {
            s8 next = (s8)row + dy;
            if (next < 0)            row = rows - 1;
            else if (next >= rows)   row = 0;
            else                     row = (u8)next;
            idx = row * cols + col;
            if (idx >= count)
                continue;
            if (sTC->activeItems[idx].type == TERMINAL_ITEM_SELECTABLE)
                break;
        }
        if (tried >= rows)
            return FALSE;
    }
    else
    {
        return FALSE;
    }

    if (idx == startIdx)
        return FALSE;
    TerminalContent_SetCursor(idx);
    return TRUE;
}

static bool8 TerminalContent_TryScroll(s8 direction)
{
    if (direction < 0 && sTC->scrollOffset > 0)
    {
        sTC->scrollOffset--;
        TerminalContent_RenderBody();
        return TRUE;
    }
    if (direction > 0 && sTC->scrollOffset + T_BODY_ROWS < TerminalContent_TotalRows())
    {
        sTC->scrollOffset++;
        TerminalContent_RenderBody();
        return TRUE;
    }
    return FALSE;
}

void TerminalContent_ExitCallback(void)
{
    PlaySE(SE_SELECT);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[sTC->taskId].func = Task_TerminalContentFadeOut;
}

static void TerminalContent_Activate(void)
{
    const struct TerminalItem *item;
    if (sTC->cursorItemIdx >= sTC->activeItemCount)
        return;
    item = &sTC->activeItems[sTC->cursorItemIdx];
    if (item->type != TERMINAL_ITEM_SELECTABLE)
        return;
    if (item->onActivate != NULL)
        item->onActivate();
}

static void Task_TerminalContentInput(u8 taskId)
{
    if (JOY_REPEAT(DPAD_UP))
    {
        sTC->cancelArmed = FALSE;
        if (!TerminalContent_MoveCursorGrid(0, -1))
            TerminalContent_TryScroll(-1);
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        sTC->cancelArmed = FALSE;
        if (!TerminalContent_MoveCursorGrid(0, 1))
            TerminalContent_TryScroll(1);
    }
    else if (JOY_REPEAT(DPAD_LEFT))
    {
        sTC->cancelArmed = FALSE;
        TerminalContent_MoveCursorGrid(-1, 0);
    }
    else if (JOY_REPEAT(DPAD_RIGHT))
    {
        sTC->cancelArmed = FALSE;
        TerminalContent_MoveCursorGrid(1, 0);
    }

    if (JOY_NEW(A_BUTTON | START_BUTTON))
    {
        sTC->cancelArmed = FALSE;
        TerminalContent_Activate();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        TerminalItemCallback cancelCb = (sTC->onCancel != NULL)
                                      ? sTC->onCancel
                                      : TerminalContent_ExitCallback;
        const struct TerminalItem *current = &sTC->activeItems[sTC->cursorItemIdx];
        bool8 cursorOnCancel = (current->type == TERMINAL_ITEM_SELECTABLE
                                && current->onActivate == cancelCb);

        if (cursorOnCancel && sTC->cancelArmed)
        {
            cancelCb();
        }
        else if (cursorOnCancel)
        {
            u8 row = sTC->cursorItemIdx / sTC->activeCols;
            bool8 scrollChanged = FALSE;
            if (row < sTC->scrollOffset)
            {
                sTC->scrollOffset = row;
                scrollChanged = TRUE;
            }
            else if (row >= sTC->scrollOffset + T_BODY_ROWS)
            {
                sTC->scrollOffset = row - T_BODY_ROWS + 1;
                scrollChanged = TRUE;
            }
            if (scrollChanged)
                TerminalContent_RenderBody();
            PlaySE(SE_SELECT);
            sTC->cancelArmed = TRUE;
        }
        else
        {
            s8 cancelIdx = TerminalContent_FindRowByCallback(cancelCb);
            if (cancelIdx >= 0)
            {
                TerminalContent_SetCursor((u8)cancelIdx);
                sTC->cancelArmed = TRUE;
            }
            else
            {
                cancelCb();
            }
        }
    }
}

static void Task_TerminalContentFadeOut(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    SetMainCallback2(sTC->exitCb);
    TerminalContent_Teardown();
    DestroyTask(taskId);
}

void TerminalContent_Teardown(void)
{
    TerminalUI_HideSpotlight();
    FreeAllWindowBuffers();
    if (sTC != NULL)
    {
        u8 i;
        for (i = 0; i < sTC->spriteCount; i++)
            DestroySprite(&gSprites[sTC->spriteIds[i]]);
        if (sTC->playerSpriteId != MAX_SPRITES)
            DestroySprite(&gSprites[sTC->playerSpriteId]);
        Free(sTC);
        sTC = NULL;
    }
}

static void TerminalContent_CreatePlayerSpriteAs(u16 x, u16 y, u8 gender)
{
    u8 facilityClass = (gender == MALE) ? FACILITY_CLASS_BRENDAN
                                        : FACILITY_CLASS_MAY;
    u8 spriteId;

    if (sTC->playerSpriteId != MAX_SPRITES)
    {
        DestroySprite(&gSprites[sTC->playerSpriteId]);
        sTC->playerSpriteId = MAX_SPRITES;
    }

    spriteId = CreateTrainerSprite(FacilityClassToPicIndex(facilityClass),
                                   x, y, 0, NULL);
    if (spriteId == MAX_SPRITES)
        return;
    ApplyPlayerAppearancePalette(gSprites[spriteId].oam.paletteNum);
    sTC->playerSpriteId = spriteId;
    sTC->playerSpriteX = x;
    sTC->playerSpriteY = y;
}

void TerminalContent_CreatePlayerSprite(u16 x, u16 y)
{
    TerminalContent_CreatePlayerSpriteAs(x, y, gSaveBlock2Ptr->playerGender);
}

void TerminalContent_RecreatePlayerSprite(u8 gender)
{
    if (sTC->playerSpriteId == MAX_SPRITES)
        return;
    TerminalContent_CreatePlayerSpriteAs(sTC->playerSpriteX, sTC->playerSpriteY, gender);
}

static void TerminalContent_SeedCursor(u8 initialCursorIdx)
{
    if (initialCursorIdx < sTC->activeItemCount
        && sTC->activeItems[initialCursorIdx].type == TERMINAL_ITEM_SELECTABLE)
    {
        sTC->cursorItemIdx = initialCursorIdx;
    }
    else
    {
        s8 fallback = TerminalContent_FindSelectable(0, 1);
        sTC->cursorItemIdx = (fallback < 0) ? 0 : (u8)fallback;
    }
}

void TerminalContent_SetActiveItems(const struct TerminalItem *items,
                                           u8 itemCount,
                                           u8 cols,
                                           u8 initialCursorIdx)
{
    AGB_ASSERT(items != NULL && itemCount > 0 && cols > 0);
    sTC->activeItems     = items;
    sTC->activeItemCount = itemCount;
    sTC->activeCols      = cols;
    sTC->scrollOffset    = 0;
    sTC->cancelArmed     = FALSE;
    TerminalContent_SeedCursor(initialCursorIdx);
    TerminalContent_RenderBody();
}

void TerminalContent_SetScroll(u8 scrollOffset)
{
    u8 maxScroll;
    u8 totalRows = TerminalContent_TotalRows();
    maxScroll = (totalRows > T_BODY_ROWS) ? (totalRows - T_BODY_ROWS) : 0;
    if (scrollOffset > maxScroll)
        scrollOffset = maxScroll;
    sTC->scrollOffset = scrollOffset;
    TerminalContent_RenderBody();
}

void TerminalContent_RestorePageItems(u8 initialCursorIdx)
{
    sTC->activeItems     = sTC->page->items;
    sTC->activeItemCount = sTC->page->itemCount;
    sTC->activeCols      = sTC->page->cols;
    sTC->scrollOffset    = 0;
    sTC->cancelArmed     = FALSE;
    TerminalContent_SeedCursor(initialCursorIdx);
    TerminalContent_RenderBody();
}

void TerminalContent_SwapPage(const struct TerminalPage *page, u8 initialCursorIdx)
{
    u8 i;

    AGB_ASSERT(page != NULL);
    AGB_ASSERT(page->cols > 0);

    sTC->onCursorMove = NULL;
    sTC->onCancel = page->onBack;

    for (i = 0; i < sTC->spriteCount; i++)
        DestroySprite(&gSprites[sTC->spriteIds[i]]);
    sTC->spriteCount = 0;
    if (sTC->playerSpriteId != MAX_SPRITES)
    {
        DestroySprite(&gSprites[sTC->playerSpriteId]);
        sTC->playerSpriteId = MAX_SPRITES;
    }
    TerminalUI_HideSpotlight();

    sTC->page            = page;
    if (page->header != NULL)
        sTC->currentHeader = page->header;
    sTC->activeItems     = page->items;
    sTC->activeItemCount = page->itemCount;
    sTC->activeCols      = page->cols;
    sTC->scrollOffset    = 0;
    sTC->cancelArmed     = FALSE;

    if (page->prepare != NULL)
        page->prepare();

    TerminalContent_SeedCursor(initialCursorIdx);
    TerminalContent_RenderBody();

    if (page->createSprites != NULL)
        page->createSprites();
}

void TerminalContent_SetCursorMoveHook(void (*cb)(u8 newCursorIdx))
{
    sTC->onCursorMove = cb;
}

void TerminalContent_SetCancelCallback(void (*cb)(void))
{
    sTC->onCancel = cb;
}

