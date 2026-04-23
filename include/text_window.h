#ifndef GUARD_TEXT_WINDOW_H
#define GUARD_TEXT_WINDOW_H

// Window-frame tile art. The paired palettes (one per Pip-Boy theme) live
// in pipboy_theme.c; the tile gfx is shared across every theme.
extern const u8 gTextWindowFrame1_Gfx[];

// Player-appearance palettes -- indexed by gSaveBlock2Ptr->hairColor /
// ->skinTone, written into OBJ palette slots at rendering time.
extern const u16 gHairPalettes[][3];
extern const u16 gSkinPalettes[][3];
void ApplyPlayerAppearancePalette(u8 paletteSlot);

void LoadMessageBoxGfx(u8 windowId, u16 destOffset, u8 palOffset);
void LoadStdWindowGfx(u8 windowId, u16 tileStart, u8 palette);
void LoadSignBoxGfx(u8 windowId, u16 destOffset, u8 palOffset);
void LoadWindowGfx(u8 windowId, u8 frameId, u16 destOffset, u8 palOffset);
void LoadUserWindowBorderGfx(u8 windowId, u16 destOffset, u8 palOffset);
void LoadUserWindowBorderGfx_(u8 windowId, u16 destOffset, u8 palOffset);
void LoadUserWindowBorderGfxOnBg(u8 bg, u16 destOffset, u8 palOffset);
void DrawTextBorderOuter(u8 windowId, u16 tileNum, u8 palNum);
void DrawTextBorderInner(u8 windowId, u16 tileNum, u8 palNum);
void rbox_fill_rectangle(u8 windowId);
const u16 *GetTextWindowPalette(u8 id);
const u16 *GetOverworldTextboxPalettePtr(void);
void LoadSignPostWindowFrameGfx(void);
void LoadDexNavWindowGfx(u8 windowId, u16 destOffset, u8 palOffset);

#endif // GUARD_TEXT_WINDOW_H
