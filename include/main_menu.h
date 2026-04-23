#ifndef GUARD_MAIN_MENU_H
#define GUARD_MAIN_MENU_H

void CB2_InitMainMenu(void);
void CreateYesNoMenuParameterized(u8 x, u8 y, u16 baseTileNum, u16 baseBlock, u8 yesNoPalNum, u8 winPalNum);
void NewGameBirchSpeech_SetDefaultPlayerName(u8);

// Display names for gSaveBlock2Ptr->hairColor / ->skinTone indices.
// Return NULL if idx is out of range.
const u8 *GetPlayerHairColorName(u8 idx);
const u8 *GetPlayerSkinToneName(u8 idx);

#endif // GUARD_MAIN_MENU_H
