#ifndef GUARD_PLAYER_APPEARANCE_H
#define GUARD_PLAYER_APPEARANCE_H

// Single source of truth for player-selectable hair + skin options. One
// struct binds the palette and the display name together so they can't
// drift. Both the Birch-intro character creator and any runtime editor
// (Medical Bay terminal, etc.) consume the same tables.
struct AppearanceOption
{
    const u16 colors[3];
    const u8 *name;
};

// Compile-time option counts. The authored tables in player_appearance.c
// STATIC_ASSERT that they match ARRAY_COUNT of the table, so the two
// cannot silently drift. Callers that need a compile-time constant (e.g.
// for STATIC_ASSERT on grid layouts, or stack-array sizing) use these
// macros; callers that just want to bounds-check at runtime can use the
// extern counts below instead.
#define PLAYER_HAIR_OPTION_COUNT   10
#define PLAYER_SKIN_OPTION_COUNT    6

extern const struct AppearanceOption gPlayerHairOptions[];
extern const struct AppearanceOption gPlayerSkinTones[];
extern const u8 gPlayerHairOptionCount;
extern const u8 gPlayerSkinToneCount;

// Display-name accessors for callers that just want a label (e.g. the
// terminal's info-row text). Return NULL if idx is out of range.
const u8 *GetPlayerHairColorName(u8 idx);
const u8 *GetPlayerSkinToneName(u8 idx);

// Load the player's current hair + skin palettes into the supplied OBJ
// palette slot. Hair writes slot positions 4..6, skin writes positions
// 1..3. Reads gSaveBlock2Ptr live and clamps bad indices to 0.
void ApplyPlayerAppearancePalette(u8 paletteSlot);

// Preview helpers: load a SPECIFIC hair/skin option into an OBJ palette
// slot without touching the save block. Used by the character creator to
// live-update the player sprite as the user scrubs through options
// before committing. Out-of-range indices AGB_ASSERT in debug, silent
// no-op in release.
void PreviewPlayerHairPalette(u8 paletteSlot, u8 hairIdx);
void PreviewPlayerSkinPalette(u8 paletteSlot, u8 skinIdx);

// Atomic setter API for post-character-creation edits (e.g. a live change
// from the Medical Bay terminal). Each writes the save-block field and
// nothing else.
//
// IMPORTANT: these setters do NOT proactively refresh the overworld
// player object event's gfx/palette. They are intentionally simple and
// safe to call from ANY CB context. The guarantee is:
//
//   * If called from a non-overworld CB (terminal, menu, etc.): the
//     save-block is updated. On return to the overworld,
//     CB2_ReturnToFieldLocal's state machine re-runs InitPlayerAvatar,
//     which reads the save-block fresh. Overworld sprite picks up the
//     new values automatically. Battle sprites also read the save-block
//     live on creation (PlayerGenderToFrontTrainerPicId).
//
//   * If called from the overworld CB itself: the save-block is updated,
//     but the on-screen overworld sprite WILL NOT reflect the change
//     until the next map load or a manual refresh call (InitPlayerAvatar
//     + ObjectEventSetGraphicsId + LoadPlayerObjectEventPalette). No
//     direct-from-overworld editor exists yet, so this is a known gap,
//     not a shipped bug -- but if you add one, call the full refresh
//     sequence yourself after the setter.
//
// Out-of-range values AGB_ASSERT in debug, silent no-op in release.
void SetPlayerGender(u8 gender);
void SetPlayerHairColor(u8 idx);
void SetPlayerSkinTone(u8 idx);

#endif // GUARD_PLAYER_APPEARANCE_H
