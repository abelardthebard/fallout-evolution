#ifndef GUARD_TERMINAL_INTERNAL_H
#define GUARD_TERMINAL_INTERNAL_H

#include "main.h"
#include "sprite.h"

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

#define TC_CURSOR_COL_W      8
#define TC_MAX_SPRITES       8

struct SpotlightLayout
{
    u8  clipLeft;
    u8  clipTop;
    u8  clipWidth;
    u8  clipHeight;
    u16 scrollX;
    u16 scrollY;
};

enum TerminalItemType
{
    TERMINAL_ITEM_TEXT,
    TERMINAL_ITEM_SELECTABLE,
    TERMINAL_ITEM_SPRITE,
    TERMINAL_ITEM_BLANK,
};

enum TerminalLayout
{
    TERMINAL_LAYOUT_SPLIT,
    TERMINAL_LAYOUT_SINGLE,
};

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
    u8 cols;
    u8 layout;             // TERMINAL_LAYOUT_*
    void (*prepare)(void);
    void (*createSprites)(void);
    void (*onBack)(void);
};

struct TerminalContentState
{
    MainCallback exitCb;
    const struct TerminalPage *page;
    const u8 *currentHeader;
    const struct TerminalItem *activeItems;
    u8 activeItemCount;
    u8 activeCols;
    u8 cursorItemIdx;
    u8 scrollOffset;
    u8 taskId;
    void (*onCursorMove)(u8 newCursorIdx);
    void (*onCancel)(void);
    u8 spriteIds[TC_MAX_SPRITES];
    u8 spriteCount;
    u8 playerSpriteId;
    u16 playerSpriteX;
    u16 playerSpriteY;
    bool8 cancelArmed;
};

extern struct TerminalContentState *sTC;

void EnterTerminalContent(const struct TerminalPage *page, u8 initialCursorIdx);
void TerminalContent_SwapPage(const struct TerminalPage *page, u8 initialCursorIdx);
void TerminalContent_RestorePageItems(u8 initialCursorIdx);
void TerminalContent_SetActiveItems(const struct TerminalItem *items, u8 itemCount, u8 cols, u8 initialCursorIdx);
void TerminalContent_SetScroll(u8 scrollOffset);
void TerminalContent_SetCancelCallback(void (*cb)(void));
void TerminalContent_SetCursorMoveHook(void (*cb)(u8 newCursorIdx));
void TerminalContent_CreatePlayerSprite(u16 x, u16 y);
void TerminalContent_RecreatePlayerSprite(u8 gender);
void TerminalContent_ExitCallback(void);
void TerminalContent_Teardown(void);
void TerminalUI_ShowSpotlight(const struct SpotlightLayout *layout);
void TerminalUI_HideSpotlight(void);

extern const struct TerminalPage sTerminalMedicalRecords_Page;
extern const struct TerminalPage sTerminalTestScores_Page;

#endif // GUARD_TERMINAL_INTERNAL_H
