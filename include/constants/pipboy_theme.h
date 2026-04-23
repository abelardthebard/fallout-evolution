#ifndef GUARD_CONSTANTS_PIPBOY_THEME_H
#define GUARD_CONSTANTS_PIPBOY_THEME_H

// Pip-Boy theme identifiers. Stored in gSaveBlock2Ptr->optionsWindowFrameType;
// queried via GetActiveTheme() (include/pipboy_theme.h). See design/palette_slot_contract.md
// for how these drive palette allocation across the UI.

#define THEME_COUNT         4
#define PIPBOY_RAMP_SIZE    8

enum
{
    THEME_GREEN,
    THEME_BLUE,
    THEME_RED,
    THEME_YELLOW,
};

#endif // GUARD_CONSTANTS_PIPBOY_THEME_H
