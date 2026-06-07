@ Local IDs (array order in map.json, 1-indexed):
@   1 = Nanny
@   2 = Todd
.set LOCALID_MEDBAY_NANNY, 1
.set LOCALID_MEDBAY_TODD, 2

Vault42_MedicalBay_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Vault42_MedicalBay_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Vault42_MedicalBay_OnFrame
	.byte 0

Vault42_MedicalBay_OnFrame:
	map_script_2 VAR_PROLOGUE_STATE, PROLOGUE_MEDBAY_INTRO, Vault42_MedicalBay_EventScript_Intro
	.2byte 0

@ Arms the hallway dilly-dally detour if the player backtracked here
@ during VAR_PROLOGUE_STATE 3 (post-first-catchup, pre-second-catchup).
Vault42_MedicalBay_OnTransition::
	goto_if_ne VAR_PROLOGUE_STATE, PROLOGUE_CATCHUP_1, Vault42_MedicalBay_OnTransition_End
	goto_if_eq VAR_TODD_HALLWAY_DETOUR, 9, Vault42_MedicalBay_OnTransition_End
	setvar VAR_TODD_HALLWAY_DETOUR, 2
Vault42_MedicalBay_OnTransition_End::
	end

Vault42_MedicalBay_EventScript_Intro::
	lockall
	setspeaker SP_NAME_TODD
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddStepWest
	waitmovement 0
	applymovement LOCALID_PLAYER, Vault42_MedicalBay_Movement_PlayerFaceEast
	waitmovement 0
	msgbox Vault42_MedicalBay_Text_ToddFinally, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddBeatEmote
	waitmovement 0
	waitfieldeffect FLDEFF_ELLIPSIS_ICON
	msgbox Vault42_MedicalBay_Text_ToddForCarrying, MSGBOX_DEFAULT
	msgbox Vault42_MedicalBay_Text_ToddHeavyExam, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddWalkSouth2
	waitmovement 0
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddFaceNorth
	waitmovement 0
	msgbox Vault42_MedicalBay_Text_ToddWaitOutside, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddWalkToExit
	waitmovement 0
	playse SE_EXIT
	removeobject LOCALID_MEDBAY_TODD
	setflag FLAG_HIDE_MEDBAY_TODD
	setflag FLAG_HIDE_HALLWAY_MRSBAUER
	setflag FLAG_HIDE_HALLWAY_TODD_LATER
	setflag FLAG_HIDE_CLASSROOM_MRSBAUER
	setflag FLAG_HIDE_CLASSROOM_TODD
	setvar VAR_PROLOGUE_STATE, PROLOGUE_AWAKE
	@ Begin the umbrella quest; the "QUEST ACTIVE" toast is what hands control back.
	revealsubquest QUEST_BRIGHT_FUTURE, SUB_BF_TAKE_THE_GOAT
	startquest QUEST_BRIGHT_FUTURE
	releaseall
	end

Vault42_MedicalBay_Movement_ToddStepWest:
	walk_left
	step_end

Vault42_MedicalBay_Movement_PlayerFaceEast:
	face_right
	step_end

Vault42_MedicalBay_Movement_ToddBeatEmote:
	emote_ellipsis
	step_end

Vault42_MedicalBay_Movement_ToddWalkSouth2:
	walk_down
	walk_down
	step_end

Vault42_MedicalBay_Movement_ToddFaceNorth:
	face_up
	step_end

Vault42_MedicalBay_Movement_ToddWalkToExit:
	walk_down
	walk_down
	step_end

Vault42_MedicalBay_EventScript_Nanny::
	lock
	faceplayer
	setspeaker SP_NAME_NANNY
	goto_if_ge VAR_PROLOGUE_STATE, PROLOGUE_TIMEJUMP, Vault42_MedicalBay_EventScript_NannyPostExam
	goto_if_set FLAG_RECEIVED_RUNNING_SHOES, Vault42_MedicalBay_EventScript_NannyAfterShoes
	msgbox Vault42_MedicalBay_Text_NannyBeforeShoes, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_EventScript_NannyAfterShoes::
	setspeaker SP_NAME_NANNY
	msgbox Vault42_MedicalBay_Text_NannyAfterShoes, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_EventScript_NannyPostExam::
	msgbox Vault42_MedicalBay_Text_NannyOfferHeal, MSGBOX_DEFAULT
	yesnobox 20, 8
	goto_if_eq VAR_RESULT, YES, Vault42_MedicalBay_EventScript_NannyDoHeal
	goto Vault42_MedicalBay_EventScript_NannyDecline

Vault42_MedicalBay_EventScript_NannyDoHeal::
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	incrementgamestat GAME_STAT_RESTED_AT_VAULT
	setspeaker SP_NAME_NANNY
	msgbox Vault42_MedicalBay_Text_NannyRestored, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_EventScript_NannyDecline::
	msgbox Vault42_MedicalBay_Text_NannyDoorOpen, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_EventScript_NannyBlockExit::
	lockall
	setspeaker SP_NAME_NANNY
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	msgbox Vault42_MedicalBay_Text_NannyBlockExit, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Vault42_MedicalBay_Movement_PlayerStepNorth
	waitmovement 0
	releaseall
	end

Vault42_MedicalBay_Movement_PlayerStepNorth:
	walk_up
	step_end

@ Fallback — lockall during intro means this never fires normally.
Vault42_MedicalBay_EventScript_Todd::
	lock
	faceplayer
	setspeaker SP_NAME_TODD
	msgbox Vault42_MedicalBay_Text_ToddWaitOutside, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_Text_NannyBeforeShoes:
	.string "You'd best put on your shoes and\n"
	.string "get going.\p"
	.string "The G.O.A.T. waits for no one!$"

Vault42_MedicalBay_Text_NannyAfterShoes:
	.string "You'd best get going.\n"
	.string "The G.O.A.T. waits for no one!\p"
	.string "And mind your step. We've patched\n"
	.string "you up once today already.$"

Vault42_MedicalBay_Text_NannyBlockExit:
	.string "Wait!\n"
	.string "Don't forget your shoes!$"

Vault42_MedicalBay_Text_NannyOfferHeal:
	.string "Do you require\n"
	.string "medical attention?$"

Vault42_MedicalBay_Text_NannyRestored:
	.string "You have been restored to\n"
	.string "working order.\l"
	.string "On your way!$"

Vault42_MedicalBay_Text_NannyDoorOpen:
	.string "Very well.\n"
	.string "My door remains open.$"

Vault42_MedicalBay_Text_ToddFinally:
	.string "Finally.\p"
	.string "You're welcome, by the way.$"

Vault42_MedicalBay_Text_ToddForCarrying:
	.string "For carrying you here?\n"
	.string "While you were unconscious?$"

Vault42_MedicalBay_Text_ToddHeavyExam:
	.string "We can talk about how heavy\n"
	.string "you are later.\p"
	.string "The G.O.A.T. already started.\n"
	.string "Grab your shoes and let's go.$"

Vault42_MedicalBay_Text_ToddWaitOutside:
	.string "I'll wait for you outside.\n"
	.string "Watch the door on your way out.$"

