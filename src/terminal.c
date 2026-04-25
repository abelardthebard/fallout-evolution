#include "global.h"
#include "bg.h"
#include "data.h"
#include "event_data.h"
#include "field_effect.h"
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
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/vars.h"
#include "terminal.h"

#include "data/terminal_hack.h"

// ============================================================================
// Shared geometry + BG/window layout
// ============================================================================

#define T_ROW_HEIGHT         15
#define T_CELL_W             6

#define T_BODY_COLS          20
#define T_BODY_ROWS          8

#define T_HEADER_H           15
#define T_HEADER_GAP         4

#define T_LOG_PAD_X          8
#define T_LOG_RESERVED_W     72

#define T_BODY_W             (T_BODY_COLS * T_CELL_W)
#define T_BODY_H             (T_BODY_ROWS * T_ROW_HEIGHT)
#define T_CONTENT_W          (T_BODY_W + T_LOG_PAD_X + T_LOG_RESERVED_W)
#define T_CONTENT_H          (T_HEADER_H + T_HEADER_GAP + T_BODY_H)

#define T_MARGIN_X           ((DISPLAY_WIDTH  - T_CONTENT_W) / 2)
#define T_MARGIN_Y           ((DISPLAY_HEIGHT - T_CONTENT_H) / 2)

#define T_HEADER_Y           T_MARGIN_Y
#define T_BODY_X             T_MARGIN_X
#define T_BODY_Y             (T_MARGIN_Y + T_HEADER_H + T_HEADER_GAP)
#define T_LOG_X              (T_BODY_X + T_BODY_W + T_LOG_PAD_X)
#define T_LOG_Y              T_BODY_Y

#define T_BG_TEXT            0
#define T_BG_SPOTLIGHT       1
#define T_BG_IMAGE           2
#define T_WIN_TEXT           0

// VRAM layout (BG VRAM = 64 KB):
//   char base 0-1  BG 0 window tile data (~600 tiles)
//   char base 2    BG 2 chrome tiles
//   char base 3    BG 1 spotlight tiles
//   map base 28    BG 1 spotlight tilemap
//   map base 30    BG 0 tilemap
//   map base 31    BG 2 chrome tilemap
// Priority front->back: BG 0 (text) | BG 1 (spotlight) | BG 2 (chrome).

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

// bg = 0 (transparent) lets the chrome BG show between glyph strokes.
static const u8 sTerminalTextColors[]          = { 0, 2, 3 };
// Inverted: shadow == bg merges outline into the bright cell.
static const u8 sTerminalTextColors_Inverted[] = { 2, 1, 2 };

static const u32 sBgImageTiles[] = INCBIN_U32("graphics/terminal/background.4bpp");
static const u32 sBgImageMap[]   = INCBIN_U32("graphics/terminal/background.bin");
static const u16 sBgImagePal[]   = INCBIN_U16("graphics/terminal/background.gbapal");

// Chrome tileset must stay below tile 960 -- BG 1's tilemap lives there.
STATIC_ASSERT(sizeof(sBgImageTiles) / 32 < 960, terminalBgImage_tilesetOverlapsMap);

struct SpotlightLayout
{
    u8  clipLeft;
    u8  clipTop;
    u8  clipWidth;
    u8  clipHeight;
    u16 scrollX;
    u16 scrollY;
};

static void TerminalUI_InitBgsAndWindows(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sTerminalBgTemplates, ARRAY_COUNT(sTerminalBgTemplates));
    // Zero scrolls -- ShowBg doesn't touch them, so they retain whatever
    // the previous screen left behind.
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
    // Authored green; remap ramp colors to the active theme.
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

static void TerminalUI_ShowSpotlight(const struct SpotlightLayout *layout)
{
    // WIN0 clips BG 1 to the rect; WININ/WINOUT allow OBJ in both regions.
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
}

static void TerminalUI_HideSpotlight(void)
{
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

// ============================================================================
// Hack minigame
// ============================================================================

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
// Bracket pair geometry. Span = closer - opener + 1 (cells inclusive).
// MIN/MAX bound a single pair's width; MIN_BRACKET_GAP is the minimum
// number of cells between distinct pairs on the same row. Spans must be
// unique across the whole board (see IsBracketSpanUnique).
#define MIN_BRACKET_SPAN    2
#define MAX_BRACKET_SPAN    10
#define MIN_BRACKET_GAP     4
#define LOG_WORD_BUF_SIZE   (TERMINAL_MAX_WORD_LENGTH + 1)

#define WORD_MIN_GAP        2

// Geometric upper bound on per-row word count, derived for buffer sizing.
#define MAX_WORDS_PER_ROW \
    (((BOARD_COLS) + (WORD_MIN_GAP)) / ((TERMINAL_MIN_WORD_LENGTH) + (WORD_MIN_GAP)))

// Build-time proof that each tier's worst-case row fits in BOARD_COLS.
#define TIER_MAX_QUOTA(WC)            (((WC) + (BOARD_ROWS) - 1) / (BOARD_ROWS))
#define TIER_MAX_SPAN(WC, WLMAX)      (TIER_MAX_QUOTA(WC) * (WLMAX) + (TIER_MAX_QUOTA(WC) - 1) * (WORD_MIN_GAP))
#define TIER_PLACEMENT_FEASIBLE(WC, WLMAX)  (TIER_MAX_SPAN(WC, WLMAX) <= (BOARD_COLS))

STATIC_ASSERT(TIER_PLACEMENT_FEASIBLE(TIER_1_WORD_COUNT, TIER_1_WORD_LEN_MAX), tier1_placement);
STATIC_ASSERT(TIER_PLACEMENT_FEASIBLE(TIER_2_WORD_COUNT, TIER_2_WORD_LEN_MAX), tier2_placement);
STATIC_ASSERT(TIER_PLACEMENT_FEASIBLE(TIER_3_WORD_COUNT, TIER_3_WORD_LEN_MAX), tier3_placement);

#define TERMINAL_IN_PROGRESS  0
#define TERMINAL_WON          1
#define TERMINAL_LOST         2

// Sized so an all-duds run (1 row each) fills the column without eviction.
#define MAX_LOG_ENTRIES      T_BODY_ROWS
// Fits worst case ">DUD REMOVED" + EOS = 13.
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

// Script: VAR_0x8000=tier, VAR_0x8001=flagId, VAR_0x8002=content_id (chained on win).
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

// Called via `specialvar VAR_RESULT, IsTerminalLockedOut`, which writes the
// return value into VAR_RESULT. Must return a value -- a void function would
// leave undefined garbage in r0, breaking the goto_if_eq check downstream.
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
        // ShowBg flips per-BG bits only; OBJ has its own master enable.
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

// Distributes BOARD_COLS slack as a uniform random partition (stars-and-
// bars) across count+1 gap slots: leading, internal (>= WORD_MIN_GAP),
// trailing. Build-time STATIC_ASSERTs above prove minSpan <= BOARD_COLS.
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

// Floor + random `extras` bump per row, then row-by-row placement.
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

// No overlap or nesting; AND at least MIN_BRACKET_GAP empty cells between
// proposed and any existing pair on the row. `existing` is flat
// [open1, close1, open2, close2, ...].
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

// Bracket span can't cover a word cell (avoids highlight ambiguity).
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

// Span uniqueness is board-wide, not per-row.
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

// Degrades to fewer pairs if the board is too dense.
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

// Empty pairs like `()` collapse to one click-through; non-empty fall
// through to per-cell stepping.
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

// Word: whole word. Bracket endpoint: opener..closer span. Else: cell.
static void ComputeHighlightSpan(u8 *outStartCol, u8 *outEndCol)
{
    u8 col = sHack->cursorCol;
    u8 row = sHack->cursorRow;
    s8 bracketIdx;

    if (GetWordSpanAt(row, col, outStartCol, outEndCol))
        return;

    // Match MoveCursor's single-jump treatment of blank runs.
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

// Duds a random non-password, non-dudded candidate. No-op if none eligible.
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

// When full, overwrites oldest. Caller fills kind-specific fields.
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

// Bottom-anchored. Newest entries fit first (variable heights); older
// entries stay in the ring buffer but render off-screen until they age out.
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
            // Suppress highlight after end-state so the frozen cursor
            // doesn't visually blink on no-longer-actionable cells.
            bool8 highlighted = (sHack->endResult == TERMINAL_IN_PROGRESS)
                             && (row == sHack->cursorRow && col >= hlStart && col <= hlEnd);
            u16 cellX = T_BODY_X + col * T_CELL_W;
            u16 cellY = T_BODY_Y + row * T_ROW_HEIGHT;

            // Pre-fill: text printer only paints the glyph area, so with
            // xOffset > 0 the cell's left padding would stay untouched.
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
        // Word cells, blank runs, and empty `()` pairs collapse to a
        // single-press jump; non-empty brackets fall through to per-cell.
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

    // Chain directly to the content viewer on win -- skipping the
    // overworld CB2 keeps the screen black between hack and content.
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

// ============================================================================
// Content viewer
// ============================================================================

#define TC_CURSOR_COL_W   8
#define TC_MAX_SPRITES    8

enum TerminalItemType
{
    TERMINAL_ITEM_TEXT,
    TERMINAL_ITEM_SELECTABLE,
    TERMINAL_ITEM_SPRITE,
    TERMINAL_ITEM_BLANK,
};

// NULL = inert SELECTABLE (cursor lands but A is a no-op).
typedef void (*TerminalItemCallback)(void);

struct TerminalSpriteItem
{
    const struct SpriteTemplate *template;
    s16 dx;
    s16 dy;
};

struct TerminalItem
{
    u8 type;
    const u8 *text;
    const struct TerminalSpriteItem *sprite;
    TerminalItemCallback onActivate;
};

struct TerminalPage
{
    const u8 *header;
    const struct TerminalItem *items;
    u8 itemCount;
    u8 cols;               // grid column count, must be >= 1 (1 = list)
    void (*prepare)(void);
    void (*createSprites)(void);
};

struct TerminalContentState
{
    MainCallback exitCb;
    const struct TerminalPage *page;
    const struct TerminalItem *activeItems;
    u8 activeItemCount;
    u8 activeCols;         // current column count -- pickers override page
    u8 cursorItemIdx;
    u8 taskId;
    void (*onCursorMove)(u8 newCursorIdx);
    void (*onCancel)(void);
    u8 spriteIds[TC_MAX_SPRITES];
    u8 spriteCount;
    u8 playerSpriteId;     // MAX_SPRITES = none tracked
    u16 playerSpriteX;
    u16 playerSpriteY;
};

static EWRAM_DATA struct TerminalContentState *sTC = NULL;

static void CB2_TerminalContentSetup(void);
static void CB2_TerminalContentMain(void);
static void TerminalContent_VBlank(void);
static void Task_TerminalContentInput(u8 taskId);
static void Task_TerminalContentFadeOut(u8 taskId);
static void TerminalContent_Teardown(void);
static void TerminalContent_RenderPage(void);
static void TerminalContent_RenderBody(void);
static void TerminalContent_DrawCursor(u8 itemIdx);
static void TerminalContent_EraseCursor(u8 itemIdx);
static void TerminalContent_SetCursor(u8 newIdx);
static s8 TerminalContent_FindSelectable(s8 fromIdx, s8 direction);
static s8 TerminalContent_FindExit(void);
static void TerminalContent_ExitCallback(void);
static void TerminalContent_CreatePlayerSprite(u16 x, u16 y);
static void TerminalContent_CreatePlayerSpriteAs(u16 x, u16 y, u8 gender);

extern const struct TerminalPage *const gTerminalContents[NUM_TERMINAL_CONTENTS];

void ShowTerminalContent(void)
{
    u16 contentId = gSpecialVar_0x8000;

    sTC = AllocZeroed(sizeof(*sTC));
    sTC->exitCb = CB2_ReturnToFieldContinueScript;

    AGB_ASSERT(contentId < NUM_TERMINAL_CONTENTS);
    sTC->page = gTerminalContents[contentId];
    AGB_ASSERT(sTC->page != NULL);
    AGB_ASSERT(sTC->page->cols > 0);

    sTC->activeItems = sTC->page->items;
    sTC->activeItemCount = sTC->page->itemCount;
    sTC->activeCols = sTC->page->cols;
    sTC->playerSpriteId = MAX_SPRITES;

    if (sTC->page->prepare != NULL)
        sTC->page->prepare();

    {
        s8 idx = TerminalContent_FindSelectable(0, 1);
        sTC->cursorItemIdx = (idx < 0) ? 0 : (u8)idx;
    }

    // Cancel the overworld's pending fade-in -- otherwise the field
    // becomes visible for one frame before our state-0 black-out.
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetPaletteFade();
    SetMainCallback2(CB2_TerminalContentSetup);
    gMain.state = 0;
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
        ShowBg(T_BG_SPOTLIGHT);
        ShowBg(T_BG_IMAGE);
        HideBg(3);
        // ShowBg flips per-BG bits only; OBJ has its own master enable.
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

// Row-major grid layout. For cols=1 the column term collapses and items
// stack vertically as before. Mirrors vanilla PrintMenuGridTable indexing.
static void TerminalContent_ItemPos(u8 itemIdx, u16 *outX, u16 *outY)
{
    u8 cols = sTC->activeCols;
    u8 row  = itemIdx / cols;
    u8 col  = itemIdx % cols;
    u16 colW = T_BODY_W / cols;
    *outX = T_BODY_X + col * colW;
    *outY = T_BODY_Y + row * T_ROW_HEIGHT;
}

// Skips TERMINAL_ITEM_SPRITE spawn so picker swaps don't duplicate sprites.
static void TerminalContent_RenderBody(void)
{
    const struct TerminalItem *items = sTC->activeItems;
    u8 count = sTC->activeItemCount;
    u8 cap   = sTC->activeCols * T_BODY_ROWS;
    u8 i;

    FillWindowPixelBuffer(T_WIN_TEXT, PIXEL_FILL(0));

    if (sTC->page->header != NULL)
        TerminalUI_DrawCenteredHeader(T_WIN_TEXT, sTC->page->header);

    for (i = 0; i < count && i < cap; i++)
    {
        const struct TerminalItem *item = &items[i];
        u16 x, y;
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
    TerminalContent_ItemPos(itemIdx, &x, &y);
    AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL, x, y,
        sTerminalTextColors, TEXT_SKIP_DRAW, gText_SelectorArrow3);
}

static void TerminalContent_EraseCursor(u8 itemIdx)
{
    u16 x, y;
    if (itemIdx >= sTC->activeItemCount)
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

static s8 TerminalContent_FindExit(void)
{
    u8 i;
    for (i = 0; i < sTC->activeItemCount; i++)
    {
        if (sTC->activeItems[i].type == TERMINAL_ITEM_SELECTABLE
            && sTC->activeItems[i].onActivate == TerminalContent_ExitCallback)
            return (s8)i;
    }
    return -1;
}

static void TerminalContent_SetCursor(u8 newIdx)
{
    if (newIdx == sTC->cursorItemIdx || newIdx >= sTC->activeItemCount)
        return;
    PlaySE(SE_SELECT);
    TerminalContent_EraseCursor(sTC->cursorItemIdx);
    sTC->cursorItemIdx = newIdx;
    TerminalContent_DrawCursor(sTC->cursorItemIdx);
    CopyWindowToVram(T_WIN_TEXT, COPYWIN_GFX);

    if (sTC->onCursorMove != NULL)
        sTC->onCursorMove(newIdx);
}

// Wrap-around 2D motion matching vanilla ChangeMenuGridCursorPosition.
// Horizontal walks stay within the current row; vertical walks stay within
// the current column. cols=1 collapses to pure vertical -- the column
// constraint becomes the whole array, so a single canonical path serves
// both list and grid pages. Skip-walks past non-SELECTABLE cells along the
// motion axis only, never crossing axes.
static void TerminalContent_MoveCursorGrid(s8 dx, s8 dy)
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
        // Last row may be partial -- shrink wrap range to the existing cells.
        u8 rowStart = row * cols;
        u8 rowWidth = (rowStart + cols > count) ? (count - rowStart) : cols;
        if (rowWidth <= 1)
            return;

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
            return;
    }
    else if (dy != 0)
    {
        // Walk row-by-row in the same column. Rows whose partial-end excludes
        // this column are skipped (rowStart + col >= count).
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
            return;
    }
    else
    {
        return;
    }

    if (idx == startIdx)
        return;
    TerminalContent_SetCursor(idx);
}

static void TerminalContent_ExitCallback(void)
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
    if (JOY_REPEAT(DPAD_UP))         TerminalContent_MoveCursorGrid( 0, -1);
    else if (JOY_REPEAT(DPAD_DOWN))  TerminalContent_MoveCursorGrid( 0,  1);
    else if (JOY_REPEAT(DPAD_LEFT))  TerminalContent_MoveCursorGrid(-1,  0);
    else if (JOY_REPEAT(DPAD_RIGHT)) TerminalContent_MoveCursorGrid( 1,  0);

    if (JOY_NEW(A_BUTTON | START_BUTTON))
    {
        TerminalContent_Activate();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        if (sTC->onCancel != NULL)
        {
            sTC->onCancel();
        }
        else
        {
            // First B jumps cursor to EXIT, second B activates it.
            const struct TerminalItem *item = &sTC->activeItems[sTC->cursorItemIdx];
            if (item->type == TERMINAL_ITEM_SELECTABLE
                && item->onActivate == TerminalContent_ExitCallback)
            {
                TerminalContent_Activate();
            }
            else
            {
                s8 exitIdx = TerminalContent_FindExit();
                if (exitIdx >= 0)
                    TerminalContent_SetCursor((u8)exitIdx);
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

static void TerminalContent_Teardown(void)
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

static void TerminalContent_CreatePlayerSprite(u16 x, u16 y)
{
    TerminalContent_CreatePlayerSpriteAs(x, y, gSaveBlock2Ptr->playerGender);
}

static void TerminalContent_RecreatePlayerSprite(u8 gender)
{
    if (sTC->playerSpriteId == MAX_SPRITES)
        return;
    TerminalContent_CreatePlayerSpriteAs(sTC->playerSpriteX, sTC->playerSpriteY, gender);
}

// Cursor seed: prefer initialCursorIdx if it lands on SELECTABLE, else first
// SELECTABLE in the array. Shared by both active-items entry points.
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

// Picker entry point. Caller owns the items array + column count for the
// duration of the picker. cols must be >= 1; pass 1 for a single-column
// list, 2+ for a grid.
static void TerminalContent_SetActiveItems(const struct TerminalItem *items,
                                           u8 itemCount,
                                           u8 cols,
                                           u8 initialCursorIdx)
{
    AGB_ASSERT(items != NULL && itemCount > 0 && cols > 0);
    sTC->activeItems     = items;
    sTC->activeItemCount = itemCount;
    sTC->activeCols      = cols;
    TerminalContent_SeedCursor(initialCursorIdx);
    TerminalContent_RenderBody();
}

// Restores the page's authored items[] + cols. Used when a picker closes.
static void TerminalContent_RestorePageItems(u8 initialCursorIdx)
{
    sTC->activeItems     = sTC->page->items;
    sTC->activeItemCount = sTC->page->itemCount;
    sTC->activeCols      = sTC->page->cols;
    TerminalContent_SeedCursor(initialCursorIdx);
    TerminalContent_RenderBody();
}

static void TerminalContent_SetCursorMoveHook(void (*cb)(u8 newCursorIdx))
{
    sTC->onCursorMove = cb;
}

static void TerminalContent_SetCancelCallback(void (*cb)(void))
{
    sTC->onCancel = cb;
}

// ============================================================================
// Pages
// ============================================================================

// ---- Medical Records ----

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

// Sized for "GENDER: " (8) + "Masculine" (9) + EOS.
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

// Row indices into sTerminalMedicalRecords_Items. Pickers restore the
// cursor to their parent row on close.
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

// ---- HAIR picker ----
//
// 2-col grid (5 rows x 2 cols = 10 hair options). Cancel is the B-button --
// onCancel restores the original color, no explicit Cancel row.

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

static const struct TerminalItem sTerminalMedicalRecords_Items[] =
{
    [MED_ROW_NAME]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_NameRow   },
    [MED_ROW_HOME]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Home },
    [MED_ROW_GENDER] = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_GenderRow },
    [MED_ROW_HAIR]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_HairRow,   .onActivate = HairPicker_Open },
    [MED_ROW_SKIN]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sTerminalMedicalRecords_SkinRow   },
    [MED_ROW_EVAL]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Eval },
    [MED_ROW_BLANK]  = { .type = TERMINAL_ITEM_BLANK },
    [MED_ROW_EXIT]   = { .type = TERMINAL_ITEM_SELECTABLE, .text = sText_TerminalMedicalRecords_Exit, .onActivate = TerminalContent_ExitCallback },
};

// Y_OFFSET nudges the sprite above the geometric body-area center.
// clipTop tracks it so sprite + spotlight translate as one unit.
#define MEDICAL_RECORDS_Y_OFFSET           (-6)
#define MEDICAL_RECORDS_SPRITE_X           (T_LOG_X + T_LOG_RESERVED_W / 2)
#define MEDICAL_RECORDS_SPRITE_Y           (T_LOG_Y + T_BODY_H / 2 + MEDICAL_RECORDS_Y_OFFSET)

// Up to T_LOG_PAD_X uses the column-gutter; beyond that the glow extends
// behind text rows (BG 0 priority keeps text on top).
#define MEDICAL_RECORDS_WIN_EXTEND_LEFT    8

// Glow center in the 10x4-tile spotlight canvas (glow is 76x22 px
// centered within the top-left 80x32 px subregion).
#define TERMINAL_SPOTLIGHT_CENTER_X        40
#define TERMINAL_SPOTLIGHT_CENTER_Y        16

// +28 places the disc 4 px above the sprite's feet line (Birch convention).
#define MEDICAL_RECORDS_GLOW_VS_SPRITE_Y   28

// BG scroll: register value = tilemap coord - screen coord. Negatives wrap via u16.
#define MEDICAL_RECORDS_BG1_HOFS   ((u16)(TERMINAL_SPOTLIGHT_CENTER_X - MEDICAL_RECORDS_SPRITE_X))
#define MEDICAL_RECORDS_BG1_VOFS   ((u16)(TERMINAL_SPOTLIGHT_CENTER_Y \
                                          - (MEDICAL_RECORDS_SPRITE_Y + MEDICAL_RECORDS_GLOW_VS_SPRITE_Y)))

static const struct SpotlightLayout sMedicalRecords_Spotlight =
{
    .clipLeft   = T_LOG_X - MEDICAL_RECORDS_WIN_EXTEND_LEFT,
    .clipTop    = T_LOG_Y + MEDICAL_RECORDS_Y_OFFSET,
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
    .cols          = 1,
    .prepare       = TerminalMedicalRecords_Prepare,
    .createSprites = TerminalMedicalRecords_CreateSprites,
};

// ---- Page dispatch table ----

const struct TerminalPage *const gTerminalContents[NUM_TERMINAL_CONTENTS] =
{
    [TERMINAL_CONTENT_MEDICAL_RECORDS] = &sTerminalMedicalRecords_Page,
};
