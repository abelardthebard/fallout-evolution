@ Local IDs (array order in map.json, 1-indexed):
@   1 = Mrs. Bauer
@   2 = Todd
.set LOCALID_CLASSROOM_MRSBAUER, 1
.set LOCALID_CLASSROOM_TODD, 2

Vault42_Classroom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Vault42_Classroom_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Vault42_Classroom_OnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Vault42_Classroom_OnFrame
	.byte 0

Vault42_Classroom_OnWarp:
	map_script_2 VAR_PROLOGUE_STATE, 5, Vault42_Classroom_EventScript_TurnPlayerNorth
	.2byte 0

Vault42_Classroom_EventScript_TurnPlayerNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

Vault42_Classroom_OnFrame:
	map_script_2 VAR_PROLOGUE_STATE, 5, Vault42_Classroom_EventScript_AfterTimeJump
	map_script_2 VAR_PROLOGUE_STATE, 6, Vault42_Classroom_EventScript_PostScoresCheck
	.2byte 0

@ Stage-NPC hide-flag recovery (resilient against stale saves).
@ Bauer visible from state 5 (post-warp). Todd visible only at state 5
@ (removeobject'd at end of AfterTimeJump as state → 6).
Vault42_Classroom_OnTransition::
	call_if_lt VAR_PROLOGUE_STATE, 5, Vault42_Classroom_EventScript_HideBauer
	call_if_ge VAR_PROLOGUE_STATE, 5, Vault42_Classroom_EventScript_ShowBauer
	call_if_ne VAR_PROLOGUE_STATE, 5, Vault42_Classroom_EventScript_HideTodd
	call_if_eq VAR_PROLOGUE_STATE, 5, Vault42_Classroom_EventScript_ShowTodd
	end

Vault42_Classroom_EventScript_HideBauer::
	setflag FLAG_HIDE_CLASSROOM_MRSBAUER
	return

Vault42_Classroom_EventScript_ShowBauer::
	clearflag FLAG_HIDE_CLASSROOM_MRSBAUER
	return

Vault42_Classroom_EventScript_HideTodd::
	setflag FLAG_HIDE_CLASSROOM_TODD
	return

Vault42_Classroom_EventScript_ShowTodd::
	clearflag FLAG_HIDE_CLASSROOM_TODD
	return

@ Fires once on classroom arrival after the time-jump (VAR_PROLOGUE_STATE == 5).
@ Todd realizes time was lost, blames the player, exits south.
Vault42_Classroom_EventScript_AfterTimeJump::
	lockall

	@ Todd: "!!" — still facing left, reacts to the lost time
	playse SE_PIN
	applymovement LOCALID_CLASSROOM_TODD, Vault42_Classroom_Movement_ToddDoubleExclaim
	waitmovement 0
	waitfieldeffect FLDEFF_DOUBLE_EXCL_MARK_ICON
	delay 15

	@ Todd and Bauer turn to face each other
	applymovement LOCALID_CLASSROOM_TODD, Common_Movement_FaceRight
	applymovement LOCALID_CLASSROOM_MRSBAUER, Common_Movement_FaceLeft
	waitmovement 0
	delay 15

	@ Todd: "?"
	playse SE_PIN
	applymovement LOCALID_CLASSROOM_TODD, Vault42_Classroom_Movement_ToddQuestion
	waitmovement 0
	waitfieldeffect FLDEFF_QUESTION_MARK_ICON
	delay 15

	@ Bauer: "..."
	applymovement LOCALID_CLASSROOM_MRSBAUER, Vault42_Classroom_Movement_BauerEllipsis
	waitmovement 0
	waitfieldeffect FLDEFF_ELLIPSIS_ICON

	@ Todd's path: (3,4) -> (3,3) -> (7,3) -> (7,6) -> (5,6). Stops next to player at (4,6).
	@ Bauer returns to her default south-facing as Todd walks off.
	applymovement LOCALID_CLASSROOM_MRSBAUER, Common_Movement_FaceDown
	applymovement LOCALID_CLASSROOM_TODD, Vault42_Classroom_Movement_ToddApproachPlayer
	waitmovement 0

	@ Todd and player face each other for the accusation
	applymovement LOCALID_CLASSROOM_TODD, Common_Movement_FaceLeft
	applymovement LOCALID_PLAYER, Common_Movement_FaceRight
	waitmovement 0

	setspeaker SP_NAME_TODD
	msgbox Vault42_Classroom_Text_ToddYourFault, MSGBOX_DEFAULT
	closemessage

	@ Todd exits south from (5,6) to (5,18) and vanishes
	applymovement LOCALID_CLASSROOM_TODD, Vault42_Classroom_Movement_ToddExit
	waitmovement 0
	playse SE_EXIT
	removeobject LOCALID_CLASSROOM_TODD
	setflag FLAG_HIDE_CLASSROOM_TODD

	setvar VAR_PROLOGUE_STATE, 6
	releaseall
	end

@ Mrs. Bauer dialog routes by story state. Todd is despawned after the
@ AfterTimeJump scene so his idle never fires in practice.
Vault42_Classroom_EventScript_MrsBauerIdle::
	lock
	faceplayer
	setspeaker SP_NAME_MRSBAUER
	goto_if_eq VAR_PROLOGUE_STATE, 6, Vault42_Classroom_EventScript_BauerCheckScoresPrompt
	goto_if_eq VAR_PROLOGUE_STATE, 7, Vault42_Classroom_EventScript_BauerOverseerReminder
	release
	end

Vault42_Classroom_EventScript_BauerCheckScoresPrompt::
	msgbox Vault42_Classroom_Text_BauerCheckScores, MSGBOX_DEFAULT
	release
	end

Vault42_Classroom_EventScript_BauerOverseerReminder::
	msgbox Vault42_Classroom_Text_BauerOverseerReminder, MSGBOX_DEFAULT
	release
	end

Vault42_Classroom_EventScript_ToddIdle::
	end

@ Fires polling at VAR_PROLOGUE_STATE == 6. No-ops until the player has
@ opened their own row on the test scores terminal (FLAG_VIEWED_OWN_TEST_SCORES).
@ Then Bauer addresses the player about their results and sends them to the Overseer.
Vault42_Classroom_EventScript_PostScoresCheck::
	goto_if_unset FLAG_VIEWED_OWN_TEST_SCORES, Vault42_Classroom_EventScript_PostScoresCheck_End
	lockall

	applymovement LOCALID_CLASSROOM_MRSBAUER, Common_Movement_FaceLeft
	applymovement LOCALID_PLAYER, Common_Movement_FaceRight
	waitmovement 0

	setspeaker SP_NAME_MRSBAUER
	msgbox Vault42_Classroom_Text_BauerPostScoresIntro, MSGBOX_DEFAULT
	multichoice 20, 8, MULTI_VAULT42_SCORES_RESPONSE, TRUE
	switch VAR_RESULT
	case 0, Vault42_Classroom_EventScript_PostScores_Mistake
	case 1, Vault42_Classroom_EventScript_PostScores_WhatNow

Vault42_Classroom_EventScript_PostScores_Mistake::
	msgbox Vault42_Classroom_Text_BauerMistakeReply, MSGBOX_DEFAULT
	goto Vault42_Classroom_EventScript_PostScores_Merge

Vault42_Classroom_EventScript_PostScores_WhatNow::
	msgbox Vault42_Classroom_Text_BauerWhatNowReply, MSGBOX_DEFAULT
	goto Vault42_Classroom_EventScript_PostScores_Merge

Vault42_Classroom_EventScript_PostScores_Merge::
	msgbox Vault42_Classroom_Text_BauerOverseerClose, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_CLASSROOM_MRSBAUER, Common_Movement_FaceDown
	waitmovement 0
	setvar VAR_PROLOGUE_STATE, 7
	releaseall
	end

Vault42_Classroom_EventScript_PostScoresCheck_End::
	end

@ Fires on either south-exit doormat (4,18)/(5,18) when VAR_PROLOGUE_STATE == 6.
@ Bauer reminds the player to review test scores before leaving the classroom.
Vault42_Classroom_EventScript_BauerBlockExit::
	lockall
	setspeaker SP_NAME_MRSBAUER
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	msgbox Vault42_Classroom_Text_BauerBlockExit, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Vault42_Classroom_Movement_PlayerStepNorth
	waitmovement 0
	releaseall
	end

Vault42_Classroom_Movement_PlayerStepNorth:
	walk_up
	step_end

Vault42_Classroom_Movement_ToddDoubleExclaim:
	emote_double_exclamation_mark
	step_end

Vault42_Classroom_Movement_ToddQuestion:
	emote_question_mark
	step_end

Vault42_Classroom_Movement_BauerEllipsis:
	emote_ellipsis
	step_end

Vault42_Classroom_Movement_ToddApproachPlayer:
	walk_up               @ (3,4) -> (3,3)
	walk_right            @ -> (4,3)
	walk_right            @ -> (5,3)
	walk_right            @ -> (6,3)
	walk_right            @ -> (7,3)
	walk_down             @ -> (7,4)
	walk_down             @ -> (7,5)
	walk_down             @ -> (7,6)
	walk_left             @ -> (6,6)
	walk_left             @ -> (5,6)
	step_end

Vault42_Classroom_Movement_ToddExit:
	walk_down             @ (5,6) -> (5,7)
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down             @ -> (5,18)
	set_invisible
	step_end

Vault42_Classroom_Text_ToddYourFault:
	.string "This is all your fault…$"

Vault42_Classroom_Text_BauerCheckScores:
	.string "We can have a chat after\n"
	.string "you've reviewed your test\l"
	.string "scores on the terminal.$"

Vault42_Classroom_Text_BauerBlockExit:
	.string "Hold it!\p"
	.string "You need to check your test\n"
	.string "scores before you leave.$"

Vault42_Classroom_Text_BauerPostScoresIntro:
	.string "Ah. There you are, dear.\n"
	.string "A quick word before you go.\p"
	.string "In regards to your exam results…\p"
	.string "Your scores were very consistent.\p"
	.string "The same number, in every\n"
	.string "single category.\l"
	.string "And that number was zero.$"

Vault42_Classroom_Text_BauerMistakeReply:
	.string "I did check it twice, dear.\n"
	.string "The machine does not err…$"

Vault42_Classroom_Text_BauerWhatNowReply:
	.string "That, dear, is above my station.$"

Vault42_Classroom_Text_BauerOverseerClose:
	.string "The particulars are best\n"
	.string "left to the Overseer…\p"
	.string "He is expecting you.\n"
	.string "Do try not to keep him waiting.$"

Vault42_Classroom_Text_BauerOverseerReminder:
	.string "Still here?\p"
	.string "The Overseer's office is\n"
	.string "just down the hall.\p"
	.string "Do hurry along.$"
