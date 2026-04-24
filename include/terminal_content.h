#ifndef GUARD_TERMINAL_CONTENT_H
#define GUARD_TERMINAL_CONTENT_H

#include "sprite.h"
#include "constants/terminal_content.h"

// What a single row in a page body can be.
enum TerminalItemType
{
    TERMINAL_ITEM_TEXT,        // Locked display text, renders full-width
    TERMINAL_ITEM_SELECTABLE,  // Cursor-reachable text; reserves column for cursor glyph
    TERMINAL_ITEM_SPRITE,      // Inline image at an offset within the body
    TERMINAL_ITEM_BLANK,       // Empty row (spacer; renders nothing)
};

// What happens when a SELECTABLE item is activated with A.
enum TerminalItemAction
{
    TERMINAL_ACTION_NONE,
    TERMINAL_ACTION_EXIT,       // Exit terminal to the overworld
    // Extend later: NEXT_PAGE, SET_FLAG, SCRIPT_CALLBACK, etc.
};

struct TerminalSpriteItem
{
    const struct SpriteTemplate *template;
    s16 dx;                     // Offset from body's top-left (in pixels)
    s16 dy;
};

struct TerminalItem
{
    u8 type;                    // enum TerminalItemType
    u8 action;                  // enum TerminalItemAction (SELECTABLE only)
    const u8 *text;             // TEXT / SELECTABLE
    const struct TerminalSpriteItem *sprite;  // SPRITE
};

struct TerminalPage
{
    const u8 *header;           // Centered on the content block
    const struct TerminalItem *items;
    u8 itemCount;
    // Optional: called exactly once at page entry (from ShowTerminalContent)
    // -- not per render. Populates any dynamic row-text buffers referenced
    // by items[].text. NULL for static pages.
    void (*prepare)(void);
    // Optional: called once after the CB2 setup state machine has reset
    // sprite/palette state, so sprites created here survive. Use
    // TerminalContent_CreatePlayerSprite() or a similar helper that
    // registers the returned sprite for teardown.
    void (*createSprites)(void);
};

// Lookup table: TERMINAL_CONTENT_* id -> page. Defined in
// src/terminal_content_pages.c.
extern const struct TerminalPage *const gTerminalContents[NUM_TERMINAL_CONTENTS];

// Script contract:
//   setvar VAR_0x8000, <TERMINAL_CONTENT_*>
//   special ShowTerminalContent
//   waitstate
// VAR_RESULT is not set; the viewer always returns to the overworld on EXIT.
void ShowTerminalContent(void);

// Create a player trainer sprite (gender + hair + skin from gSaveBlock2Ptr)
// at the supplied screen coordinates and register it with the content
// viewer so it's destroyed on EXIT. Intended to be called from a page's
// createSprites hook.
void TerminalContent_CreatePlayerSprite(u16 x, u16 y);

#endif // GUARD_TERMINAL_CONTENT_H
