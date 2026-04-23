#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "random.h"
#include "scanline_effect.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "pipboy_theme.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "terminal_content.h"
#include "terminal_hack.h"
#include "terminal_layout.h"
#include "terminal_ui.h"
#include "constants/vars.h"

#include "data/terminal_hack.h"

#define TERMINAL_TOTAL_WORDS \
    (ARRAY_COUNT(sTerminalWordsLen4)  + \
     ARRAY_COUNT(sTerminalWordsLen5)  + \
     ARRAY_COUNT(sTerminalWordsLen6)  + \
     ARRAY_COUNT(sTerminalWordsLen7)  + \
     ARRAY_COUNT(sTerminalWordsLen8)  + \
     ARRAY_COUNT(sTerminalWordsLen9)  + \
     ARRAY_COUNT(sTerminalWordsLen10))

STATIC_ASSERT(TERMINAL_TOTAL_WORDS <= TERMINAL_MAX_WORD_IDS,
              terminalWordPool_exceedsBitfieldCapacity);

STATIC_ASSERT(sizeof(((struct SaveBlock1 *)0)->terminalWordBurned) * 8 == TERMINAL_MAX_WORD_IDS,
              terminalWordBurned_sizeMustMatchMaxWordIds);

// Hack treats the body's 20x8 cell grid as its board; alias so existing
// board-oriented code reads naturally.
#define BOARD_ROWS          T_BODY_ROWS
#define BOARD_COLS          T_BODY_COLS

#define MAX_CANDIDATES      TERMINAL_WORD_COUNT
#define MAX_BRACKETS        8
// Total cells from opener to closer inclusive. Matches Fallout's visual
// tightness (brackets ~3-5 wide regardless of password length).
#define MAX_BRACKET_SPAN    5
// +1 for EOS terminator when copying guess text for the log.
#define LOG_WORD_BUF_SIZE   (TERMINAL_MAX_WORD_LENGTH + 1)

#define BRACKET_EFFECT_DUD_REMOVE      0
#define BRACKET_EFFECT_ALLOWANCE_RESET 1

#define TERMINAL_IN_PROGRESS  0
#define TERMINAL_WON          1
#define TERMINAL_LOST         2

// Anchor the log to the board's vertical extent so both share top and
// bottom margins, and every log text line sits on a board row. With an
// even BOARD_ROWS the fit is exact; with an odd BOARD_ROWS the log
// truncates one row short of the board (integer division).
#define MAX_LOG_ENTRIES      (T_BODY_H / T_LOG_ENTRY_HEIGHT)
// Longest line is "LIKENESS=NN" (9 prefix + 2 digits) or ">WORD" (1 + word).
// Both fit in LOG_WORD_BUF_SIZE + 1 (+EOS); +2 gives slack.
#define LOG_LINE_BUF_SIZE    (LOG_WORD_BUF_SIZE + 2)

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
    u8 effect;
    bool8 consumed;
};

struct TerminalLogEntry
{
    u8 word[LOG_WORD_BUF_SIZE];
    u8 likeness;
};

struct TerminalHack
{
    MainCallback exitCb;
    u16 flagId;
    u16 chainContentId;         // TERMINAL_CONTENT_* to chain to on success
    u8 tier;
    u8 wordLen;
    u8 wordCount;
    u8 bracketCount;
    u8 cursorCol;
    u8 cursorRow;
    u8 attemptsRemaining;
    u8 passwordCandidateIdx;
    u8 endResult;
    u8 logHead;   // index of next write; also the oldest entry when buffer is full
    u8 logCount;  // live entries in buffer, <= MAX_LOG_ENTRIES
    u8 board[BOARD_ROWS * BOARD_COLS];
    struct Candidate candidates[MAX_CANDIDATES];
    struct BracketPair brackets[MAX_BRACKETS];
    struct TerminalLogEntry logEntries[MAX_LOG_ENTRIES];
};

static EWRAM_DATA struct TerminalHack *sHack = NULL;

// BG templates, window templates, bg image INCBIN, text colors, and the
// VRAM layout STATIC_ASSERT all live in terminal_ui.c now (shared with
// the content screen). Hack pulls them via terminal_ui.h.

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

// Ticks for cutscene-driven movement too; gate on !ScriptContext_IsEnabled()
// if that ever becomes a problem.
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

// Script contract: VAR_0x8000 = tier, VAR_0x8001 = flag_id,
// VAR_0x8002 = TERMINAL_CONTENT_* to show on success (chained internally so
// the hack -> content transition stays black; the overworld CB is never
// re-entered between them).
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

void IsTerminalLockedOut(void)
{
    u16 terminalId = gSpecialVar_0x8000;
    u16 lockedId   = VarGet(VAR_TERMINAL_LOCKED_ID);
    u16 steps      = VarGet(VAR_TERMINAL_LOCKOUT_STEPS);

    gSpecialVar_Result = (lockedId == terminalId && steps > 0) ? 1 : 0;
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
        HideBg(2);
        HideBg(3);
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

// Falls back to allowing burned words if filter leaves pool < wordCount.
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

// `padding` adds required empty cells between the proposed placement and
// any prior placement on the same row (0 = may abut, 1 = must have one
// garbage char between them).
static bool8 WordCollidesWithPriorPlacements(u8 row, u8 col, u8 numPlaced, u8 padding)
{
    u8 j;
    for (j = 0; j < numPlaced; j++)
    {
        const struct Candidate *other = &sHack->candidates[j];
        if (other->row != row)
            continue;
        if (col + sHack->wordLen + padding <= other->col)
            continue;
        if (other->col + sHack->wordLen + padding <= col)
            continue;
        return TRUE;
    }
    return FALSE;
}

// Ceiling of wordCount / BOARD_ROWS -- the smallest per-row cap that still
// admits placing every word. Derived, not tuned: for any (wordCount,
// BOARD_ROWS) it's the threshold above which a row is "clumped" relative
// to the uniform average.
static u8 MaxWordsPerRow(void)
{
    return (sHack->wordCount + BOARD_ROWS - 1) / BOARD_ROWS;
}

static u8 CountWordsInRow(u8 row, u8 numPlaced)
{
    u8 count = 0;
    u8 j;
    for (j = 0; j < numPlaced; j++)
        if (sHack->candidates[j].row == row)
            count++;
    return count;
}

static u16 CountValidWordPositions(u8 numPlaced, u8 padding)
{
    u8 maxCol = BOARD_COLS - sHack->wordLen;
    u8 cap = MaxWordsPerRow();
    u16 count = 0;
    u8 row, col;
    for (row = 0; row < BOARD_ROWS; row++)
    {
        if (CountWordsInRow(row, numPlaced) >= cap)
            continue;
        for (col = 0; col <= maxCol; col++)
            if (!WordCollidesWithPriorPlacements(row, col, numPlaced, padding))
                count++;
    }
    return count;
}

static void PlaceWordAtRank(u8 candidateIdx, u16 rank, u8 padding)
{
    u8 maxCol = BOARD_COLS - sHack->wordLen;
    u8 cap = MaxWordsPerRow();
    u8 row, col;
    for (row = 0; row < BOARD_ROWS; row++)
    {
        if (CountWordsInRow(row, candidateIdx) >= cap)
            continue;
        for (col = 0; col <= maxCol; col++)
        {
            if (WordCollidesWithPriorPlacements(row, col, candidateIdx, padding))
                continue;
            if (rank == 0)
            {
                sHack->candidates[candidateIdx].row = row;
                sHack->candidates[candidateIdx].col = col;
                return;
            }
            rank--;
        }
    }
}

// Uniform random placement over the whole board, with two filters:
// (1) a per-row cap at ceil(wordCount/BOARD_ROWS) so no single row can
//     hold more than the natural average ceiling (blocks clumping without
//     forcing a fixed distribution shape -- empty rows still occur);
// (2) a preferred 1-cell buffer between words, falling back to zero buffer
//     when row geometry can't accommodate it (Tier 3 edge case).
// AGB_ASSERT catches broken tier definitions rather than silently stacking
// words on (0, 0).
static void AssignWordSlots(void)
{
    u8 i;
    for (i = 0; i < sHack->wordCount; i++)
    {
        u8 padding = 1;
        u16 validCount = CountValidWordPositions(i, padding);
        if (validCount == 0)
        {
            padding = 0;
            validCount = CountValidWordPositions(i, padding);
        }
        AGB_ASSERT(validCount > 0);
        if (validCount == 0)
            continue;
        PlaceWordAtRank(i, Random() % validCount, padding);
    }
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

// Reject any overlap with an existing pair: no existing col strictly between
// the proposed pair, AND the proposed pair not strictly inside any existing
// pair. Applies across all bracket types so differently-typed pairs can't
// interleave or nest either. `existing` is flat [open1, close1, open2, ...].
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
    }
    return TRUE;
}

// Reject placements whose span covers any word cell -- keeps bracket pairs
// visually bounded, avoids highlight ambiguity over enclosed words.
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

    // Collect endpoints of all brackets on this row regardless of type --
    // different-type pairs must not interleave or nest either.
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
            u8 openerCol = garbageCols[i];
            u8 closerCol = garbageCols[j];
            if (closerCol - openerCol + 1 > MAX_BRACKET_SPAN)
                continue;
            if (IsValidBracketPlacement(openerCol, closerCol, existing, numExisting)
                && !HasWordCellBetween(row, openerCol, closerCol))
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
            if (closerCol - openerCol + 1 > MAX_BRACKET_SPAN)
                continue;
            if (!IsValidBracketPlacement(openerCol, closerCol, existing, numExisting)
                || HasWordCellBetween(row, openerCol, closerCol))
                continue;
            if (idx == pick)
            {
                struct BracketPair *bp = &sHack->brackets[sHack->bracketCount];
                bp->row       = row;
                bp->openerCol = openerCol;
                bp->closerCol = closerCol;
                bp->type      = type;
                bp->effect    = BRACKET_EFFECT_DUD_REMOVE;
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

// Graceful degradation: fewer pairs placed if board too dense.
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

    if (sHack->bracketCount > 0)
    {
        u8 resetIdx = Random() % sHack->bracketCount;
        sHack->brackets[resetIdx].effect = BRACKET_EFFECT_ALLOWANCE_RESET;
    }
}

static void TerminalHack_GenerateBoard(void)
{
    const struct TerminalDifficulty *tier = &sTerminalDifficulty[sHack->tier];
    u8 bracketRoll;

    sHack->wordLen     = tier->wordLenMin + (Random() % (tier->wordLenMax - tier->wordLenMin + 1));
    sHack->wordCount   = tier->wordCount;
    sHack->cursorCol   = 0;
    sHack->cursorRow   = 0;
    sHack->attemptsRemaining = TERMINAL_MAX_ATTEMPTS;

    SampleCandidateWords();
    FillBoardWithGarbage();
    AssignWordSlots();
    WriteWordsToBoard();

    bracketRoll = tier->bracketPairsMin + (Random() % (tier->bracketPairsMax - tier->bracketPairsMin + 1));
    PlaceBracketPairs(bracketRoll);
}

static const u8 sText_AttemptsPrefix[] = _("ATTEMPTS: ");
static const u8 sText_AttemptMark[]    = _(" X");
static const u8 sText_AttemptGone[]    = _("  ");
static const u8 sText_AccessGranted[]  = _("ACCESS GRANTED - PRESS A");
static const u8 sText_AccessDenied[]   = _("ACCESS DENIED - PRESS A");
static const u8 sText_LogPrompt[]      = _(">");
static const u8 sText_LogLikeness[]    = _("LIKENESS=");

// Index of the live (undudded) candidate whose cells cover (row, col), or
// -1. Central lookup used by hit-test, highlight, and activation paths.
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

// Index of the unconsumed bracket pair whose opener OR closer is at
// (row, col), or -1. Central lookup for hit-test and activation paths.
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

// Returns TRUE if (row, col) is inside a live word, with the full word span
// in outStart/outEnd. Shared by highlight and cursor-movement logic.
static bool8 GetWordSpanAt(u8 row, u8 col, u8 *outStartCol, u8 *outEndCol)
{
    s8 idx = FindLiveCandidateAt(row, col);
    if (idx < 0)
        return FALSE;
    *outStartCol = sHack->candidates[idx].col;
    *outEndCol = sHack->candidates[idx].col + sHack->wordLen - 1;
    return TRUE;
}

// An empty bracket pair (no interior cells) collapses to a single
// movement span so it's one click-through, not two. Non-empty brackets
// don't match here -- the cursor still steps through their interior.
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

// Contiguous run of CHAR_SPACE cells on the cursor's row. Used by MoveCursor
// to let the player jump past dudded/consumed regions as a single block.
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

// Cursor hit-testing for the visible highlight.
// Word: cursor on any cell of a word highlights the whole word.
// Bracket: cursor on the opener OR closer highlights the [opener..closer] span.
//   Cells between are treated as standalone garbage (no span highlight).
// Standalone: just the cursor cell.
static void ComputeHighlightSpan(u8 *outStartCol, u8 *outEndCol)
{
    u8 col = sHack->cursorCol;
    u8 row = sHack->cursorRow;
    s8 bracketIdx;

    if (GetWordSpanAt(row, col, outStartCol, outEndCol))
        return;

    // Blank runs highlight as a block so they match how MoveCursor treats
    // them (single-press jump past the whole span).
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

// ---- Game logic (A-press handling, guesses, bracket effects) -------------

// Positional char match. Words are the same length so a simple parallel walk.
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

// Erase the word's board cells and mark the candidate dudded so hit-testing
// and cursor-movement treat it as blanks.
static void DudCandidate(u8 candidateIdx)
{
    struct Candidate *c = &sHack->candidates[candidateIdx];
    if (c->dudded)
        return;
    c->dudded = TRUE;
    EraseBoardCellsWithSpace(c->row, c->col, c->col + sHack->wordLen - 1);
}

// Erase the bracket's opener, interior, and closer, then mark it consumed.
static void ConsumeBracket(u8 bracketIdx)
{
    struct BracketPair *bp = &sHack->brackets[bracketIdx];
    if (bp->consumed)
        return;
    bp->consumed = TRUE;
    EraseBoardCellsWithSpace(bp->row, bp->openerCol, bp->closerCol);
}

// Pick a random eligible candidate (non-password, non-dudded) and dud it.
// Silently no-ops if nothing's eligible.
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
}

// Restore attempts to max. No-op (wasted) if already at max.
static void ApplyAllowanceResetEffect(void)
{
    sHack->attemptsRemaining = TERMINAL_MAX_ATTEMPTS;
}

static void ApplyBracketEffect(u8 bracketIdx)
{
    switch (sHack->brackets[bracketIdx].effect)
    {
    case BRACKET_EFFECT_DUD_REMOVE:
        ApplyDudRemovalEffect();
        break;
    case BRACKET_EFFECT_ALLOWANCE_RESET:
        ApplyAllowanceResetEffect();
        break;
    }
}

// Ring-buffer append. When full, logHead is the next write slot AND the
// oldest entry (which we overwrite). logCount saturates at MAX_LOG_ENTRIES.
static void AddLogEntry(const struct Candidate *guess, u8 likeness)
{
    struct TerminalLogEntry *entry = &sHack->logEntries[sHack->logHead];
    const u8 *src = &sHack->board[guess->row * BOARD_COLS + guess->col];
    u8 i;
    for (i = 0; i < sHack->wordLen; i++)
        entry->word[i] = src[i];
    entry->word[sHack->wordLen] = EOS;
    entry->likeness = likeness;

    sHack->logHead = (sHack->logHead + 1) % MAX_LOG_ENTRIES;
    if (sHack->logCount < MAX_LOG_ENTRIES)
        sHack->logCount++;
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
    AddLogEntry(guess, likeness);

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
    ApplyBracketEffect(bracketIdx);
    ConsumeBracket(bracketIdx);
    PlaySE(SE_SUCCESS);
    TerminalHack_RenderBoard();
}

// A-press dispatcher: word guess, bracket activation, or nope-beep.
// Only unconsumed/undudded targets are actionable; everything else (garbage,
// blanks, dudded words, consumed brackets) falls through to SE_FAILURE.
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

// --------------------------------------------------------------------------

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

static void RenderLogPanel(void)
{
    u8 i;
    // Empty log: show a bare ">" prompt at the bottom slot as a visual cue
    // that this area will hold guesses. Gets overwritten once the first
    // guess lands at that same slot.
    if (sHack->logCount == 0)
    {
        u16 yTop = T_LOG_Y + (MAX_LOG_ENTRIES - 1) * T_LOG_ENTRY_HEIGHT;
        AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NARROW, T_LOG_X, yTop,
            gTerminalTextColors, TEXT_SKIP_DRAW, sText_LogPrompt);
        return;
    }
    // Newest entry anchors to the bottom slot; older entries stack above.
    // When the buffer isn't full, the top slots are left empty. When full,
    // the logHead position is the oldest-visible; iterating forward from
    // there yields oldest -> newest, which we place top -> bottom.
    u8 firstIdx = (sHack->logCount < MAX_LOG_ENTRIES) ? 0 : sHack->logHead;
    u8 slotBase = MAX_LOG_ENTRIES - sHack->logCount;

    for (i = 0; i < sHack->logCount; i++)
    {
        u8 idx = (firstIdx + i) % MAX_LOG_ENTRIES;
        const struct TerminalLogEntry *entry = &sHack->logEntries[idx];
        u16 yTop = T_LOG_Y + (slotBase + i) * T_LOG_ENTRY_HEIGHT;
        u8 buf[LOG_LINE_BUF_SIZE];
        u8 *dst;

        dst = StringCopy(buf, sText_LogPrompt);
        StringCopy(dst, entry->word);
        AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NARROW, T_LOG_X, yTop,
            gTerminalTextColors, TEXT_SKIP_DRAW, buf);

        dst = StringCopy(buf, sText_LogLikeness);
        ConvertIntToDecimalStringN(dst, entry->likeness, STR_CONV_MODE_LEFT_ALIGN, 2);
        AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NARROW, T_LOG_X, yTop + T_ROW_HEIGHT,
            gTerminalTextColors, TEXT_SKIP_DRAW, buf);
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
            // Suppress highlight once the game's over; cursor stays at its
            // last position in state but shouldn't visually blink on cells
            // the player can no longer interact with.
            bool8 highlighted = (sHack->endResult == TERMINAL_IN_PROGRESS)
                             && (row == sHack->cursorRow && col >= hlStart && col <= hlEnd);
            u16 cellX = T_BODY_X + col * T_CELL_W;
            u16 cellY = T_BODY_Y + row * T_ROW_HEIGHT;

            // Text printer only paints the glyph tile area; with xOffset > 0 the
            // cell's left padding is untouched, so pre-fill the whole cell.
            if (highlighted)
                FillWindowPixelRect(T_WIN_TEXT, PIXEL_FILL(2), cellX, cellY, T_CELL_W, T_ROW_HEIGHT);
            colors = highlighted ? gTerminalTextColors_Inverted : gTerminalTextColors;

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
        // Three movement-span cases collapse a region into a single jump:
        //   - word cells (jump past the whole word)
        //   - blank runs (dudded words, consumed brackets)
        //   - empty bracket pairs with zero interior like `()`
        // Non-empty brackets fall through to per-cell step so the cursor
        // stays able to inspect each interior char.
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

// Post-win/lose: hold the end screen until the player acknowledges with A,
// then fade out. VAR_RESULT signals script-side success (1) or failure (0).
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

    // On success, chain directly into the content viewer. Teardown the hack
    // state first so its EWRAM is freed; ShowTerminalContent will allocate
    // its own. Skipping CB2_ReturnToFieldContinueScript here is what keeps
    // the overworld from fading back in between the two screens.
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
