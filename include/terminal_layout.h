#ifndef GUARD_TERMINAL_LAYOUT_H
#define GUARD_TERMINAL_LAYOUT_H

// Shared geometry for all terminal screens (hack + content).
// Every visible element positions itself via these constants so the hack
// screen and content screens share an identical "safe play area" footprint
// on the GBA display. Any edit here propagates to every terminal screen.

// Cell grid
#define T_ROW_HEIGHT         15  // Matches FONT_NORMAL / FONT_NARROW glyph tile height (prevents row overlap)
#define T_CELL_W             6   // Fixed cell width; forces monospaced layout

// Safe play area dimensions (in cells)
#define T_BODY_COLS          20
#define T_BODY_ROWS          8

// Header row (ATTEMPTS on hack, page title on content)
#define T_HEADER_H           15  // FONT_NORMAL glyph tile height
#define T_HEADER_GAP         4   // Blank rows between header bottom and body top

// Horizontal subdivisions inside the body. Hack splits its body into a
// left "board" area and a right "log" area; content screens can use the
// full T_CONTENT_W instead.
#define T_LOG_PAD_X          8   // Gap between left-block and right-block
#define T_LOG_RESERVED_W     72  // Right-block reserved width (worst-case log text ~66 px)

// Derived dimensions
#define T_BODY_W             (T_BODY_COLS * T_CELL_W)
#define T_BODY_H             (T_BODY_ROWS * T_ROW_HEIGHT)
#define T_CONTENT_W          (T_BODY_W + T_LOG_PAD_X + T_LOG_RESERVED_W)
#define T_CONTENT_H          (T_HEADER_H + T_HEADER_GAP + T_BODY_H)

// Content block is centered on the GBA screen. Integer division truncates,
// which drops the odd-pixel leftover into the bottom margin -- shifting the
// block one pixel up relative to true center.
#define T_MARGIN_X           ((DISPLAY_WIDTH  - T_CONTENT_W) / 2)
#define T_MARGIN_Y           ((DISPLAY_HEIGHT - T_CONTENT_H) / 2)

// Derived positions
#define T_HEADER_Y           T_MARGIN_Y
#define T_BODY_X             T_MARGIN_X
#define T_BODY_Y             (T_MARGIN_Y + T_HEADER_H + T_HEADER_GAP)
#define T_LOG_X              (T_BODY_X + T_BODY_W + T_LOG_PAD_X)
#define T_LOG_Y              T_BODY_Y
#define T_LOG_ENTRY_HEIGHT   (2 * T_ROW_HEIGHT)

#endif // GUARD_TERMINAL_LAYOUT_H
