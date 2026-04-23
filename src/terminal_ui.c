#include "global.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "palette.h"
#include "text.h"
#include "text_window.h"
#include "pipboy_theme.h"
#include "window.h"
#include "terminal_layout.h"
#include "terminal_ui.h"

// VRAM layout for every terminal screen (GBA BG VRAM = 64 KB, 0x06000000..0x06010000):
//
//   char base 0  0x06000000..0x06004000  BG 0 window tile data starts here
//   char base 1  0x06004000..0x06008000  BG 0 window spills in (600 tiles * 32B)
//   char base 2  0x06008000..0x0600C000  BG 1 image tiles live here
//   char base 3  0x0600C000..0x06010000  (free; BG 1 addressable range extends here)
//   map base 30  0x0600F000..0x0600F800  BG 0 tilemap (outside BG 0's tile range)
//   map base 31  0x0600F800..0x06010000  BG 1 tilemap
//
// Each BG addresses 1024 tiles = 32 KB = 2 consecutive char bases from its
// charBaseIndex. So BG 1 (charBase=2) can read tiles 0..1023 from
// 0x06008000..0x06010000. BG 1's map sits at tile-index 960 within that
// range; if background.4bpp ever grew past 960 tiles, it would read the
// map as tile data. Static-asserted below.
const struct BgTemplate gTerminalBgTemplates[2] =
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
        .bg            = T_BG_IMAGE,
        .charBaseIndex = 2,
        .mapBaseIndex  = 31,
        .screenSize    = 0,
        .paletteMode   = 0,
        .priority      = 1,
        .baseTile      = 0,
    },
};

const struct WindowTemplate gTerminalWindowTemplates[2] =
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

// bg = 0 (transparent) so the decorative BG layer shows between glyph strokes.
const u8 gTerminalTextColors[]          = { 0, 2, 3 };
// Inverted: shadow = bg so the outline merges into the bright cell,
// leaving a crisp dark silhouette instead of a halo.
const u8 gTerminalTextColors_Inverted[] = { 2, 1, 2 };

static const u32 sBgImageTiles[] = INCBIN_U32("graphics/terminal/background.4bpp");
static const u32 sBgImageMap[]   = INCBIN_U32("graphics/terminal/background.bin");
static const u16 sBgImagePal[]   = INCBIN_U16("graphics/terminal/background.gbapal");

// BG 1's tilemap sits at tile-index 960 in its addressable range. The
// tileset must stay below that or it would overlap the map in VRAM.
STATIC_ASSERT(sizeof(sBgImageTiles) / 32 < 960, terminalBgImage_tilesetOverlapsMap);

void TerminalUI_InitBgsAndWindows(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, gTerminalBgTemplates, ARRAY_COUNT(gTerminalBgTemplates));
    // Zero the hardware scroll registers -- neither InitBgsFromTemplates nor
    // ShowBg touch them, so they retain whatever the previous screen left
    // behind (overworld map scrolling) and the BG image would render from
    // a non-zero offset, wrapping the 32-tile tilemap.
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    InitWindows(gTerminalWindowTemplates);
    DeactivateAllTextPrinters();
    FillBgTilemapBufferRect_Palette0(T_BG_TEXT, 0, 0, 0, 32, 32);
    CopyBgTilemapBufferToVram(T_BG_TEXT);
}

void TerminalUI_LoadPalettes(void)
{
    // Text palette follows the active Pip-Boy theme directly -- each theme
    // has its own piptext_pal*.gbapal on disk with matching fg/bg/shadow
    // slots 1-3 pre-tuned for that theme.
    LoadPalette(GetActiveThemeTextPal(), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    // BG image palette: load the green-authored version, then remap any
    // entries that match the standard Pip-Boy green ramp to the active
    // theme's equivalent shade. Non-ramp colors (black, magenta padding,
    // near-black texture) are left alone by the remap.
    LoadPalette(sBgImagePal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
    PipBoy_ApplyThemeToPalettes(BG_PLTT_ID(0), 16, 0, 0);
}

void TerminalUI_LoadBgImage(void)
{
    LoadBgTiles(T_BG_IMAGE, sBgImageTiles, sizeof(sBgImageTiles), 0);
    // LoadBgTilemap writes directly to VRAM at the BG's mapBase; this avoids
    // needing a software tilemap buffer allocated via SetBgTilemapBuffer.
    LoadBgTilemap(T_BG_IMAGE, sBgImageMap, sizeof(sBgImageMap), 0);
}

void TerminalUI_DrawCenteredHeader(u8 winId, const u8 *text)
{
    u16 x = T_MARGIN_X + GetStringCenterAlignXOffset(FONT_NORMAL, text, T_CONTENT_W);
    AddTextPrinterParameterized3(winId, FONT_NORMAL, x, T_HEADER_Y,
        gTerminalTextColors, TEXT_SKIP_DRAW, text);
}
