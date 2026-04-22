#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
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
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "terminal_hack.h"
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

#define BOARD_ROWS          7
#define COL_LEFT_START      0
#define COL_LEFT_WIDTH      10
#define COL_GAP             2
#define COL_RIGHT_START     (COL_LEFT_START + COL_LEFT_WIDTH + COL_GAP)
#define COL_RIGHT_WIDTH     10
#define BOARD_COLS          (COL_RIGHT_START + COL_RIGHT_WIDTH)

#define SLOT_COUNT          (BOARD_ROWS * 2)

#define MAX_CANDIDATES      TERMINAL_WORD_COUNT
#define MAX_BRACKETS        8

#define BRACKET_EFFECT_DUD_REMOVE      0
#define BRACKET_EFFECT_ALLOWANCE_RESET 1

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

struct TerminalHack
{
    MainCallback exitCb;
    u16 flagId;
    u8 tier;
    u8 wordLen;
    u8 wordCount;
    u8 bracketCount;
    u8 cursorCol;
    u8 cursorRow;
    u8 attemptsRemaining;
    u8 passwordCandidateIdx;
    u8 board[BOARD_ROWS * BOARD_COLS];
    struct Candidate candidates[MAX_CANDIDATES];
    struct BracketPair brackets[MAX_BRACKETS];
};

static EWRAM_DATA struct TerminalHack *sHack = NULL;

#define TERMINAL_BG        0
#define TERMINAL_WIN       0

#define P2_HEADER_X        32
#define P2_HEADER_Y        5
#define P2_BOARD_X         16
#define P2_BOARD_Y0        24
#define P2_ROW_HEIGHT      14
#define P2_EXIT_HINT_X     16
#define P2_EXIT_HINT_Y     140
#define CELL_W             6   // Fixed cell width; forces monospace grid over FONT_NARROW's variable glyphs.

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg            = TERMINAL_BG,
        .charBaseIndex = 0,
        .mapBaseIndex  = 30,
        .screenSize    = 0,
        .paletteMode   = 0,
        .priority      = 0,
        .baseTile      = 0,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    {
        .bg          = TERMINAL_BG,
        .tilemapLeft = 0,
        .tilemapTop  = 0,
        .width       = 30,
        .height      = 20,
        .paletteNum  = 15,
        .baseBlock   = 1,
    },
    DUMMY_WIN_TEMPLATE,
};

static const u8 sTextColors_Green[] = { 1, 2, 3 };
static const u16 sBackdropBlack[] = { RGB_BLACK };

extern const u8 gFontNormalLatinGlyphWidths[];
extern const u8 gFontNarrowLatinGlyphWidths[];

static void CB2_TerminalHackSetup(void);
static void CB2_TerminalHackMain(void);
static void TerminalHack_VBlank(void);
static void TerminalHack_InitBgWindow(void);
static void TerminalHack_LoadPalette(void);
static void TerminalHack_GenerateBoard(void);
static void TerminalHack_RenderBoard(void);
static void Task_TerminalHackMainInput(u8 taskId);
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

// Script contract: VAR_0x8000 = tier, VAR_0x8001 = flag_id.
void StartTerminalHack(void)
{
    sHack = AllocZeroed(sizeof(*sHack));
    sHack->tier   = gSpecialVar_0x8000;
    sHack->flagId = gSpecialVar_0x8001;
    sHack->exitCb = CB2_ReturnToFieldContinueScript;

    AGB_ASSERT(sHack->tier >= TERMINAL_TIER_1 && sHack->tier < NUM_TERMINAL_TIERS);

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
        TerminalHack_InitBgWindow();
        gMain.state++;
        break;
    case 2:
        TerminalHack_LoadPalette();
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
        ShowBg(TERMINAL_BG);
        HideBg(1);
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

static void TerminalHack_InitBgWindow(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    InitWindows(sWindowTemplates);
    DeactivateAllTextPrinters();
    FillBgTilemapBufferRect_Palette0(TERMINAL_BG, 0, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(TERMINAL_BG);
}

static void TerminalHack_LoadPalette(void)
{
    LoadPalette(gPipThemes[THEME_GREEN].textPal, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadPalette(sBackdropBlack, 0, PLTT_SIZEOF(1));
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

static void AssignWordSlots(void)
{
    u8 slots[SLOT_COUNT];
    u8 i;

    for (i = 0; i < SLOT_COUNT; i++)
        slots[i] = i;
    ShuffleU8(slots, SLOT_COUNT);

    for (i = 0; i < sHack->wordCount; i++)
    {
        u8 slot     = slots[i];
        u8 row      = slot >> 1;
        u8 isRight  = slot & 1;
        u8 colBase  = isRight ? COL_RIGHT_START : COL_LEFT_START;
        u8 maxOff   = COL_LEFT_WIDTH - sHack->wordLen;
        u8 offset   = (maxOff == 0) ? 0 : (Random() % (maxOff + 1));

        sHack->candidates[i].row = row;
        sHack->candidates[i].col = colBase + offset;
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

// Single-opener-single-closer: reject if a same-type bracket col sits
// strictly between the proposed pair.
static bool8 IsValidBracketPlacement(u8 openerCol, u8 closerCol, const u8 *existing, u8 numExisting)
{
    u8 e;
    for (e = 0; e < numExisting; e++)
    {
        if (existing[e] > openerCol && existing[e] < closerCol)
            return FALSE;
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

    for (i = 0; i < sHack->bracketCount; i++)
    {
        const struct BracketPair *b = &sHack->brackets[i];
        if (b->row == row && b->type == type)
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

static const u8 sText_PlaceholderAttempts[] = _("ATTEMPTS:  X X X X");
static const u8 sText_PlaceholderExit[]     = _("<START> EXIT /P3 STUB/");

static void TerminalHack_RenderBoard(void)
{
    u8 charBuf[2];
    u8 row, col;

    FillWindowPixelBuffer(TERMINAL_WIN, PIXEL_FILL(1));

    AddTextPrinterParameterized3(TERMINAL_WIN, FONT_NARROW, P2_HEADER_X, P2_HEADER_Y,
        sTextColors_Green, TEXT_SKIP_DRAW, sText_PlaceholderAttempts);

    charBuf[1] = EOS;
    for (row = 0; row < BOARD_ROWS; row++)
    {
        for (col = 0; col < BOARD_COLS; col++)
        {
            u8 ch = sHack->board[row * BOARD_COLS + col];
            u8 font, width, xOffset;

            // Uppercase A-Z get FONT_NORMAL (uniform 6px); everything else uses
            // FONT_NARROW. Centering per-glyph keeps narrow chars visually mid-cell.
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
            xOffset = (width < CELL_W) ? (CELL_W - width + 1) / 2 : 0;
            charBuf[0] = ch;
            AddTextPrinterParameterized3(TERMINAL_WIN, font,
                P2_BOARD_X + col * CELL_W + xOffset,
                P2_BOARD_Y0 + row * P2_ROW_HEIGHT,
                sTextColors_Green, TEXT_SKIP_DRAW, charBuf);
        }
    }

    AddTextPrinterParameterized3(TERMINAL_WIN, FONT_NARROW, P2_EXIT_HINT_X, P2_EXIT_HINT_Y,
        sTextColors_Green, TEXT_SKIP_DRAW, sText_PlaceholderExit);

    PutWindowTilemap(TERMINAL_WIN);
    CopyWindowToVram(TERMINAL_WIN, COPYWIN_FULL);
    CopyBgTilemapBufferToVram(TERMINAL_BG);
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

    // TODO: D-pad cursor, A = activate
    if (JOY_NEW(START_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSpecialVar_Result = 1;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_TerminalHackFadeOut;
    }
}

static void Task_TerminalHackFadeOut(u8 taskId)
{
    if (gPaletteFade.active)
        return;

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
