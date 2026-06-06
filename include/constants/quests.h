#ifndef GUARD_CONSTANTS_QUESTS_H
#define GUARD_CONSTANTS_QUESTS_H

// questmenu / subquestmenu scripting command params
#define QUEST_MENU_OPEN                 0   // opens the quest menu (questId = 0)
#define QUEST_MENU_UNLOCK_QUEST         1   // questId = QUEST_X (0-indexed)
#define QUEST_MENU_SET_ACTIVE           2   // questId = QUEST_X (0-indexed)
#define QUEST_MENU_SET_REWARD           3   // questId = QUEST_X (0-indexed)
#define QUEST_MENU_COMPLETE_QUEST       4   // questId = QUEST_X (0-indexed)
#define QUEST_MENU_CHECK_UNLOCKED       5   // returns result to gSpecialVar_Result
#define QUEST_MENU_CHECK_INACTIVE       6   // returns result to gSpecialVar_Result
#define QUEST_MENU_CHECK_ACTIVE         7   // returns result to gSpecialVar_Result
#define QUEST_MENU_CHECK_REWARD         8   // returns result to gSpecialVar_Result
#define QUEST_MENU_CHECK_COMPLETE       9   // returns result to gSpecialVar_Result
#define QUEST_MENU_BUFFER_QUEST_NAME    10  // buffers a quest name to gStringVar1

// ============================================================================
// MAIN QUESTS
// Append only -- reordering shifts the save-block bit layout.
// ============================================================================
#define QUEST_BRIGHT_FUTURE   0
#define QUEST_COUNT           (QUEST_BRIGHT_FUTURE + 1)

// ============================================================================
// SUBQUESTS
// Two indices per subquest matter:
//   * the child INDEX within its parent's subquest array (what scripts pass to
//     subquestmenu/revealsubquest/completesubquest -- the SUB_* defines below)
//   * the global unique `id` in each sub_quest() entry, which addresses the
//     save-block reveal/complete bitfields. Keep ids unique and append-only.
// ============================================================================

// Bright Future
#define SUB_BF_TAKE_THE_GOAT    0
#define SUB_BF_SPEAK_OVERSEER   1
#define QUEST_BRIGHT_FUTURE_SUB_COUNT  2

#define SUB_QUEST_COUNT  (QUEST_BRIGHT_FUTURE_SUB_COUNT)

#define QUEST_ARRAY_COUNT (SUB_QUEST_COUNT > QUEST_COUNT ? SUB_QUEST_COUNT : QUEST_COUNT)
#endif // GUARD_CONSTANTS_QUESTS_H
