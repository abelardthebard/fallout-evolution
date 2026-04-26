@ Local IDs (array order in map.json, 1-indexed):
@   1 = Nanny
@   2 = Todd
.set LOCALID_MEDBAY_NANNY, 1
.set LOCALID_MEDBAY_TODD, 2

Vault42_MedicalBay_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Vault42_MedicalBay_OnFrame
	.byte 0

Vault42_MedicalBay_OnFrame:
	map_script_2 VAR_PROLOGUE_STATE, 0, Vault42_MedicalBay_EventScript_Intro
	.2byte 0

Vault42_MedicalBay_EventScript_Intro::
	lockall
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddStepWest
	waitmovement 0
	applymovement LOCALID_PLAYER, Vault42_MedicalBay_Movement_PlayerFaceEast
	waitmovement 0
	message Vault42_MedicalBay_Text_ToddFinally
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	playse SE_PIN
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddBeatEmote
	waitmovement 0
	waitfieldeffect FLDEFF_ELLIPSIS_ICON
	message Vault42_MedicalBay_Text_ToddForCarrying
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	message Vault42_MedicalBay_Text_ToddHeavyExam
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddWalkSouth2
	waitmovement 0
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddFaceNorth
	waitmovement 0
	message Vault42_MedicalBay_Text_ToddWaitOutside
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	applymovement LOCALID_MEDBAY_TODD, Vault42_MedicalBay_Movement_ToddWalkToExit
	waitmovement 0
	playse SE_EXIT
	removeobject LOCALID_MEDBAY_TODD
	setflag FLAG_HIDE_MEDBAY_TODD
	setvar VAR_PROLOGUE_STATE, 1
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
	goto_if_set FLAG_RECEIVED_RUNNING_SHOES, Vault42_MedicalBay_EventScript_NannyAfterShoes
	msgbox Vault42_MedicalBay_Text_NannyBeforeShoes, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_EventScript_NannyAfterShoes::
	msgbox Vault42_MedicalBay_Text_NannyAfterShoes, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_EventScript_NannyBlockExit::
	lockall
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
	msgbox Vault42_MedicalBay_Text_ToddWaitOutside, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_Text_NannyBeforeShoes:
	.string "MISS NANNY: You'd best put on your\n"
	.string "shoes and get going.\p"
	.string "The G.O.A.T. waits for no one!$"

Vault42_MedicalBay_Text_NannyAfterShoes:
	.string "MISS NANNY: You'd best get going.\n"
	.string "The G.O.A.T. waits for no one!$"

Vault42_MedicalBay_Text_NannyBlockExit:
	.string "MISS NANNY: Wait!\n"
	.string "Don't forget your shoes!$"

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

