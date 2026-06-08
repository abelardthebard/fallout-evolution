@ Local IDs (array order in map.json, 1-indexed):
@   1 = Overseer Clay
@   2 = Todd (guest -- only present for the Overseer scene)
.set LOCALID_OVERSEER, 1
.set LOCALID_OFFICE_TODD, 2

Vault42_OverseersOffice_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Vault42_OverseersOffice_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Vault42_OverseersOffice_OnFrame
	.byte 0

@ The Overseer scene auto-plays the first time the player walks in with an
@ appointment (VAR_PROLOGUE_STATE == PROLOGUE_SENT_TO_OVERSEER). The state
@ advance at the end of the scene stops it re-firing.
Vault42_OverseersOffice_OnFrame:
	map_script_2 VAR_PROLOGUE_STATE, PROLOGUE_SENT_TO_OVERSEER, Vault42_OverseersOffice_EventScript_OverseerScene
	.2byte 0

@ Todd doesn't live here -- he's only staged in for the Overseer scene, which
@ plays while VAR_PROLOGUE_STATE == PROLOGUE_SENT_TO_OVERSEER (the only state at
@ which the player can be in the office: appointment confirmed, scene not yet
@ done). Hidden in every other state.
Vault42_OverseersOffice_OnTransition::
	call_if_eq VAR_PROLOGUE_STATE, PROLOGUE_SENT_TO_OVERSEER, Vault42_OverseersOffice_EventScript_ShowTodd
	call_if_ne VAR_PROLOGUE_STATE, PROLOGUE_SENT_TO_OVERSEER, Vault42_OverseersOffice_EventScript_HideTodd
	end

Vault42_OverseersOffice_EventScript_ShowTodd::
	clearflag FLAG_HIDE_OFFICE_TODD
	return

Vault42_OverseersOffice_EventScript_HideTodd::
	setflag FLAG_HIDE_OFFICE_TODD
	return

@ ===========================================================================
@ The Overseer scene (WORK IN PROGRESS -- ends at Clay calling for the dogs).
@ Still to come below the marker: Regine enters with the Dogmeats, the starter
@ selection, and the real closing bookkeeping (re-lock door, Todd's exit,
@ complete the "Speak with the Overseer" subquest).
@ ===========================================================================
Vault42_OverseersOffice_EventScript_OverseerScene::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_FaceUp
	waitmovement 0
	@ Clay spots the player
	playse SE_PIN
	applymovement LOCALID_OVERSEER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	setspeaker SP_NAME_OVERSEER
	msgbox Vault42_OverseersOffice_Text_OverseerGreeting, MSGBOX_DEFAULT
	closemessage
	@ Player crosses up to (4,9), beside Todd
	applymovement LOCALID_PLAYER, Vault42_OverseersOffice_Movement_PlayerApproach
	waitmovement 0

	@ --- "Your scores" preamble ---
	applymovement LOCALID_OVERSEER, Vault42_OverseersOffice_Movement_OverseerStepWest
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay1, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OVERSEER, Common_Movement_FaceDown
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay2, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OVERSEER, Vault42_OverseersOffice_Movement_OverseerStepEast
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay3, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OVERSEER, Common_Movement_FaceDown
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay4, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OVERSEER, Vault42_OverseersOffice_Movement_OverseerStepWest
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay5, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OVERSEER, Common_Movement_FaceDown
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay6, MSGBOX_DEFAULT
	closemessage

	@ --- Player response #1 ---
	dynmultipush Vault42_OverseersOffice_Text_M1_WhoDecides, 0
	dynmultipush Vault42_OverseersOffice_Text_M1_Why, 1
	dynmultipush Vault42_OverseersOffice_Text_M1_HowMany, 2
	dynmultistack 0, 0, TRUE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Vault42_OverseersOffice_EventScript_M1_WhoDecides
	case 1, Vault42_OverseersOffice_EventScript_M1_Why
	case 2, Vault42_OverseersOffice_EventScript_M1_HowMany
	goto Vault42_OverseersOffice_EventScript_M1_WhoDecides

Vault42_OverseersOffice_EventScript_M1_WhoDecides::
	msgbox Vault42_OverseersOffice_Text_M1R_WhoDecides, MSGBOX_DEFAULT
	goto Vault42_OverseersOffice_EventScript_M1_Done
Vault42_OverseersOffice_EventScript_M1_Why::
	msgbox Vault42_OverseersOffice_Text_M1R_Why, MSGBOX_DEFAULT
	goto Vault42_OverseersOffice_EventScript_M1_Done
Vault42_OverseersOffice_EventScript_M1_HowMany::
	msgbox Vault42_OverseersOffice_Text_M1R_HowMany, MSGBOX_DEFAULT
	goto Vault42_OverseersOffice_EventScript_M1_Done

Vault42_OverseersOffice_EventScript_M1_Done::
	closemessage
	applymovement LOCALID_OVERSEER, Vault42_OverseersOffice_Movement_OverseerStepEast
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay7, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OVERSEER, Common_Movement_FaceDown
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay8, MSGBOX_DEFAULT
	closemessage

	@ Todd interjects
	playse SE_PIN
	applymovement LOCALID_OFFICE_TODD, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	setspeaker SP_NAME_TODD
	msgbox Vault42_OverseersOffice_Text_ToddFather, MSGBOX_DEFAULT
	closemessage
	setspeaker SP_NAME_OVERSEER
	msgbox Vault42_OverseersOffice_Text_ClayPaperwork, MSGBOX_DEFAULT
	closemessage

	@ --- Player response #2 ---
	dynmultipush Vault42_OverseersOffice_Text_M2_WellDie, 0
	dynmultipush Vault42_OverseersOffice_Text_M2_Family, 1
	dynmultipush Vault42_OverseersOffice_Text_M2_Mistake, 2
	dynmultipush Vault42_OverseersOffice_Text_M2_Silent, 3
	dynmultistack 0, 0, TRUE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	switch VAR_RESULT
	case 0, Vault42_OverseersOffice_EventScript_M2_WellDie
	case 1, Vault42_OverseersOffice_EventScript_M2_Family
	case 2, Vault42_OverseersOffice_EventScript_M2_Mistake
	case 3, Vault42_OverseersOffice_EventScript_M2_Silent
	goto Vault42_OverseersOffice_EventScript_M2_WellDie

Vault42_OverseersOffice_EventScript_M2_WellDie::
	msgbox Vault42_OverseersOffice_Text_M2R_WellDie, MSGBOX_DEFAULT
	goto Vault42_OverseersOffice_EventScript_M2_Done
Vault42_OverseersOffice_EventScript_M2_Family::
	msgbox Vault42_OverseersOffice_Text_M2R_Family, MSGBOX_DEFAULT
	goto Vault42_OverseersOffice_EventScript_M2_Done
Vault42_OverseersOffice_EventScript_M2_Mistake::
	msgbox Vault42_OverseersOffice_Text_M2R_Mistake, MSGBOX_DEFAULT
	goto Vault42_OverseersOffice_EventScript_M2_Done
Vault42_OverseersOffice_EventScript_M2_Silent::
	msgbox Vault42_OverseersOffice_Text_M2R_Silent, MSGBOX_DEFAULT
	goto Vault42_OverseersOffice_EventScript_M2_Done

Vault42_OverseersOffice_EventScript_M2_Done::
	closemessage
	applymovement LOCALID_OVERSEER, Vault42_OverseersOffice_Movement_OverseerStepWest
	waitmovement 0
	applymovement LOCALID_OVERSEER, Common_Movement_FaceDown
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay9, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OVERSEER, Vault42_OverseersOffice_Movement_OverseerStepEast
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay10, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_OVERSEER, Common_Movement_FaceDown
	waitmovement 0
	msgbox Vault42_OverseersOffice_Text_Clay11, MSGBOX_DEFAULT
	closemessage

	@ --- REGINE + DOGMEATS + STARTER SELECTION CONTINUES HERE (TBD) ---

	@ Provisional close: advance the spine so the trigger fires exactly once.
	@ Stays the final step as the scene grows; real bookkeeping lands here.
	setvar VAR_PROLOGUE_STATE, PROLOGUE_MET_OVERSEER
	releaseall
	end

Vault42_OverseersOffice_Movement_PlayerApproach:
	walk_up
	walk_up
	step_end

Vault42_OverseersOffice_Movement_OverseerStepWest:
	walk_left
	step_end

Vault42_OverseersOffice_Movement_OverseerStepEast:
	walk_right
	step_end

@ Fallback -- the scene locks the player, so this rarely fires.
Vault42_OverseersOffice_EventScript_OfficeTodd::
	end

@ Post-scene placeholder for talking to Clay. The real post-scene line goes here.
Vault42_OverseersOffice_EventScript_Overseer::
	lock
	faceplayer
	msgbox Vault42_OverseersOffice_Text_OverseerPlaceholder, MSGBOX_DEFAULT
	release
	end

@ ===========================================================================
@ Text
@ ===========================================================================
Vault42_OverseersOffice_Text_OverseerGreeting:
	.string "Ah, {PLAYER}! There you\n"
	.string "are! Come on in…$"

Vault42_OverseersOffice_Text_Clay1:
	.string "Your future begins today,\n"
	.string "dweller.$"

Vault42_OverseersOffice_Text_Clay2:
	.string "Both of you. This won't\n"
	.string "take long.\p"
	.string "Now… your scores…$"

Vault42_OverseersOffice_Text_Clay3:
	.string "The G.O.A.T. is graded on\n"
	.string "a curve.\p"
	.string "You are not just measured\n"
	.string "against the test, you\l"
	.string "understand.$"

Vault42_OverseersOffice_Text_Clay4:
	.string "You are measured against\n"
	.string "your peers.\p"
	.string "That's how a vault stays\n"
	.string "running like a machine.$"

Vault42_OverseersOffice_Text_Clay5:
	.string "And this year… well…\n"
	.string "it was a crowded class.\p"
	.string "We came in over capacity.\p"
	.string "It happens.$"

Vault42_OverseersOffice_Text_Clay6:
	.string "But a vault can't carry\n"
	.string "slack.\p"
	.string "So the slack gets trimmed.$"

Vault42_OverseersOffice_Text_M1_WhoDecides:
	.string "Who decides?$"
Vault42_OverseersOffice_Text_M1_Why:
	.string "Why?$"
Vault42_OverseersOffice_Text_M1_HowMany:
	.string "How many?$"

Vault42_OverseersOffice_Text_M1R_WhoDecides:
	.string "Vault-Tec did.\p"
	.string "They designed the test,\n"
	.string "the test fills the jobs,\l"
	.string "and when we're full,\l"
	.string "it picks the overflow.\p"
	.string "I just run the numbers.$"

Vault42_OverseersOffice_Text_M1R_Why:
	.string "Too many mouths.\p"
	.string "Not all of them are\n"
	.string "productive.$"

Vault42_OverseersOffice_Text_M1R_HowMany:
	.string "Enough.\p"
	.string "In this case, two.$"

Vault42_OverseersOffice_Text_Clay7:
	.string "On a curve, someone's\n"
	.string "always at the far end.\p"
	.string "Has to be. That's what\n"
	.string "makes it a curve.$"

Vault42_OverseersOffice_Text_Clay8:
	.string "This year, the far end\n"
	.string "is the two of you.\p"
	.string "Surplus.$"

Vault42_OverseersOffice_Text_ToddFather:
	.string "My father will hear\n"
	.string "about this.$"

Vault42_OverseersOffice_Text_ClayPaperwork:
	.string "I would hope so.\n"
	.string "He files the paperwork.\p"
	.string "Onward…$"

Vault42_OverseersOffice_Text_M2_WellDie:
	.string "We'll die$"
Vault42_OverseersOffice_Text_M2_Family:
	.string "My family's here$"
Vault42_OverseersOffice_Text_M2_Mistake:
	.string "A mistake.$"
Vault42_OverseersOffice_Text_M2_Silent:
	.string "Say nothing$"

Vault42_OverseersOffice_Text_M2R_WellDie:
	.string "What happens is entirely\n"
	.string "up to you.$"

Vault42_OverseersOffice_Text_M2R_Family:
	.string "And now they're one\n"
	.string "mouth lighter.\p"
	.string "You've already helped.\p"
	.string "See?$"

Vault42_OverseersOffice_Text_M2R_Mistake:
	.string "Yes, there has been\n"
	.string "a mistake…\p"
	.string "You assumed this was\n"
	.string "a conversation.$"

Vault42_OverseersOffice_Text_M2R_Silent:
	.string "…Good.\p"
	.string "Silence files faster\n"
	.string "than appeals.$"

Vault42_OverseersOffice_Text_Clay9:
	.string "Now… good news.\p"
	.string "Surplus isn't an ending.\n"
	.string "It's a new beginning.$"

Vault42_OverseersOffice_Text_Clay10:
	.string "Most dwellers die under\n"
	.string "fluorescent light.\p"
	.string "You'll die under an\n"
	.string "open sky.$"

Vault42_OverseersOffice_Text_Clay11:
	.string "But you won't die alone!\p"
	.string "We aren't going to leave\n"
	.string "you empty-handed.\p"
	.string "You're getting a\n"
	.string "going-away present!\p"
	.string "Regine? …Regine!\n"
	.string "The dogs!$"

Vault42_OverseersOffice_Text_OverseerPlaceholder:
	.string "(Overseer Clay placeholder.)$"
