#include "global.h"
#include "bg.h"
#include "data.h"
#include "event_data.h"
#include "field_effect.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "player_appearance.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "terminal_content.h"
#include "terminal_layout.h"
#include "terminal_ui.h"

// Per-selectable-item reserved column for the cursor glyph.
#define TC_CURSOR_COL_W          8

// Upper bound on sprites a single page can create; enforced at render time.
#define TC_MAX_SPRITES           8

struct TerminalContentState
{
    MainCallback exitCb;
    const struct TerminalPage *page;
    u8 cursorItemIdx;                          // which item the cursor currently sits on
    u8 spriteIds[TC_MAX_SPRITES];              // sprites created for this page, for teardown
    u8 spriteCount;
};

static EWRAM_DATA struct TerminalContentState *sTC = NULL;

static void CB2_TerminalContentSetup(void);
static void CB2_TerminalContentMain(void);
static void TerminalContent_VBlank(void);
static void Task_TerminalContentInput(u8 taskId);
static void Task_TerminalContentFadeOut(u8 taskId);
static void TerminalContent_Teardown(void);
static void TerminalContent_RenderPage(void);
static void TerminalContent_DrawCursor(u8 itemIdx);
static void TerminalContent_EraseCursor(u8 itemIdx);
static void TerminalContent_SetCursor(u8 newIdx);
static s8 TerminalContent_FindSelectable(s8 fromIdx, s8 direction);
static s8 TerminalContent_FindExit(void);

// -------------------------------------------------------------------------
// Entry point (called via script special)
// -------------------------------------------------------------------------

void ShowTerminalContent(void)
{
    u16 contentId = gSpecialVar_0x8000;

    sTC = AllocZeroed(sizeof(*sTC));
    sTC->exitCb = CB2_ReturnToFieldContinueScript;

    AGB_ASSERT(contentId < NUM_TERMINAL_CONTENTS);
    sTC->page = gTerminalContents[contentId];
    AGB_ASSERT(sTC->page != NULL);

    // Populate any dynamic row text before the first render. Hoisted out
    // of RenderPage so the contract is "exactly once per page entry."
    if (sTC->page->prepare != NULL)
        sTC->page->prepare();

    // Seed cursor on the first selectable item; falls back to 0 if none.
    {
        s8 idx = TerminalContent_FindSelectable(0, 1);
        sTC->cursorItemIdx = (idx < 0) ? 0 : (u8)idx;
    }

    // Blank the display and cancel the overworld's pending fade-in before
    // handing control to our CB. We're called from inside the overworld's
    // script processor, which just started a fade-in from black; if we
    // wait for it to finish we'd watch the overworld become visible for
    // a moment before our state 0 blanked the screen. Clobbering DISPCNT
    // and the fade state here lets the content build in a clean slate.
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetPaletteFade();
    SetMainCallback2(CB2_TerminalContentSetup);
    gMain.state = 0;
}

// -------------------------------------------------------------------------
// CB2 state machine
// -------------------------------------------------------------------------

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
        CreateTask(Task_TerminalContentInput, 0);
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

// -------------------------------------------------------------------------
// Rendering
// -------------------------------------------------------------------------

static void TerminalContent_RenderPage(void)
{
    const struct TerminalPage *page = sTC->page;
    u8 i;

    FillWindowPixelBuffer(T_WIN_TEXT, PIXEL_FILL(0));

    if (page->header != NULL)
        TerminalUI_DrawCenteredHeader(T_WIN_TEXT, page->header);

    for (i = 0; i < page->itemCount && i < T_BODY_ROWS; i++)
    {
        const struct TerminalItem *item = &page->items[i];
        u16 y = T_BODY_Y + i * T_ROW_HEIGHT;

        switch (item->type)
        {
        case TERMINAL_ITEM_TEXT:
            if (item->text != NULL)
                AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL, T_BODY_X, y,
                    gTerminalTextColors, TEXT_SKIP_DRAW, item->text);
            break;

        case TERMINAL_ITEM_SELECTABLE:
            if (item->text != NULL)
                AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL,
                    T_BODY_X + TC_CURSOR_COL_W, y,
                    gTerminalTextColors, TEXT_SKIP_DRAW, item->text);
            break;

        case TERMINAL_ITEM_SPRITE:
            if (item->sprite != NULL && item->sprite->template != NULL
                && sTC->spriteCount < ARRAY_COUNT(sTC->spriteIds))
            {
                u8 spriteId = CreateSprite(item->sprite->template,
                    T_BODY_X + item->sprite->dx,
                    T_BODY_Y + item->sprite->dy,
                    0);
                if (spriteId != MAX_SPRITES)
                    sTC->spriteIds[sTC->spriteCount++] = spriteId;
            }
            break;

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

static void TerminalContent_DrawCursor(u8 itemIdx)
{
    if (itemIdx >= sTC->page->itemCount)
        return;
    if (sTC->page->items[itemIdx].type != TERMINAL_ITEM_SELECTABLE)
        return;
    // Vanilla selector arrow -- the same glyph used by "I remember / Remind me"
    // and every other pokeemerald menu cursor.
    AddTextPrinterParameterized3(T_WIN_TEXT, FONT_NORMAL,
        T_BODY_X, T_BODY_Y + itemIdx * T_ROW_HEIGHT,
        gTerminalTextColors, TEXT_SKIP_DRAW, gText_SelectorArrow3);
}

static void TerminalContent_EraseCursor(u8 itemIdx)
{
    if (itemIdx >= sTC->page->itemCount)
        return;
    FillWindowPixelRect(T_WIN_TEXT, PIXEL_FILL(0),
        T_BODY_X, T_BODY_Y + itemIdx * T_ROW_HEIGHT,
        TC_CURSOR_COL_W, T_ROW_HEIGHT);
}

// Walks items looking for the next SELECTABLE in `direction` (+1 or -1)
// starting FROM fromIdx (inclusive). Returns -1 if none.
static s8 TerminalContent_FindSelectable(s8 fromIdx, s8 direction)
{
    const struct TerminalPage *page = sTC->page;
    s8 i = fromIdx;
    u8 tried = 0;

    while (tried < page->itemCount)
    {
        if (i < 0)
            i = page->itemCount - 1;
        else if (i >= page->itemCount)
            i = 0;
        if (page->items[i].type == TERMINAL_ITEM_SELECTABLE)
            return i;
        i += direction;
        tried++;
    }
    return -1;
}

// Returns idx of the first SELECTABLE whose action is EXIT, or -1 if none.
// Used by B to let the player jump to "exit" from anywhere on the page.
static s8 TerminalContent_FindExit(void)
{
    const struct TerminalPage *page = sTC->page;
    u8 i;

    for (i = 0; i < page->itemCount; i++)
    {
        if (page->items[i].type == TERMINAL_ITEM_SELECTABLE
            && page->items[i].action == TERMINAL_ACTION_EXIT)
            return (s8)i;
    }
    return -1;
}

// -------------------------------------------------------------------------
// Input
// -------------------------------------------------------------------------

static void TerminalContent_SetCursor(u8 newIdx)
{
    if (newIdx == sTC->cursorItemIdx || newIdx >= sTC->page->itemCount)
        return;
    PlaySE(SE_SELECT);
    TerminalContent_EraseCursor(sTC->cursorItemIdx);
    sTC->cursorItemIdx = newIdx;
    TerminalContent_DrawCursor(sTC->cursorItemIdx);
    CopyWindowToVram(T_WIN_TEXT, COPYWIN_GFX);
}

static void TerminalContent_MoveCursor(s8 direction)
{
    s8 next = TerminalContent_FindSelectable(sTC->cursorItemIdx + direction, direction);
    if (next < 0)
        return;
    TerminalContent_SetCursor((u8)next);
}

static void TerminalContent_Activate(u8 taskId)
{
    const struct TerminalItem *item;
    if (sTC->cursorItemIdx >= sTC->page->itemCount)
        return;
    item = &sTC->page->items[sTC->cursorItemIdx];
    if (item->type != TERMINAL_ITEM_SELECTABLE)
        return;

    switch (item->action)
    {
    case TERMINAL_ACTION_EXIT:
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_TerminalContentFadeOut;
        break;
    case TERMINAL_ACTION_NONE:
    default:
        break;
    }
}

static void Task_TerminalContentInput(u8 taskId)
{
    if (JOY_REPEAT(DPAD_UP))        TerminalContent_MoveCursor(-1);
    else if (JOY_REPEAT(DPAD_DOWN)) TerminalContent_MoveCursor( 1);

    // A / START activate the item under the cursor.
    // B acts as "back": if the cursor is already on an EXIT item it
    // activates; otherwise it jumps the cursor to the EXIT item without
    // activating, so a second B press (or A / START) exits.
    if (JOY_NEW(A_BUTTON | START_BUTTON))
    {
        TerminalContent_Activate(taskId);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        const struct TerminalItem *item = &sTC->page->items[sTC->cursorItemIdx];
        if (item->type == TERMINAL_ITEM_SELECTABLE && item->action == TERMINAL_ACTION_EXIT)
        {
            TerminalContent_Activate(taskId);
        }
        else
        {
            s8 exitIdx = TerminalContent_FindExit();
            if (exitIdx >= 0)
                TerminalContent_SetCursor((u8)exitIdx);
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
    // Unconditional hide. No-op if this page never called ShowSpotlight;
    // otherwise it clears WIN0 + BG1 scroll so nothing leaks into the
    // next screen's GPU state.
    TerminalUI_HideSpotlight();

    FreeAllWindowBuffers();
    if (sTC != NULL)
    {
        u8 i;
        for (i = 0; i < sTC->spriteCount; i++)
            DestroySprite(&gSprites[sTC->spriteIds[i]]);
        Free(sTC);
        sTC = NULL;
    }
}

// -------------------------------------------------------------------------
// Public helpers for page createSprites hooks
// -------------------------------------------------------------------------

void TerminalContent_CreatePlayerSprite(u16 x, u16 y)
{
    u8 facilityClass = (gSaveBlock2Ptr->playerGender == MALE)
                        ? FACILITY_CLASS_BRENDAN
                        : FACILITY_CLASS_MAY;
    u8 spriteId = CreateTrainerSprite(FacilityClassToPicIndex(facilityClass),
                                      x, y, 0, NULL);
    if (spriteId == MAX_SPRITES)
        return;
    if (sTC->spriteCount >= ARRAY_COUNT(sTC->spriteIds))
    {
        DestroySprite(&gSprites[spriteId]);
        return;
    }
    ApplyPlayerAppearancePalette(gSprites[spriteId].oam.paletteNum);
    sTC->spriteIds[sTC->spriteCount++] = spriteId;
}
