@ Local IDs (array order in map.json, 1-indexed):
@   1 = Todd
.set LOCALID_HALLWAY_TODD, 1

Vault42_2F_Hallway_MapScripts::
	.byte 0

Vault42_2F_Hallway_EventScript_ToddCatchUpScene::
	lockall
	clearflag FLAG_HIDE_HALLWAY_TODD
	addobject LOCALID_HALLWAY_TODD
	playse SE_PIN
	applymovement LOCALID_HALLWAY_TODD, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_ToddWalkToPlayer
	waitmovement 0
	setspeaker SP_NAME_TODD
	msgbox Vault42_2F_Hallway_Text_ToddCatchUp, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_ToddRunOffEast
	waitmovement 0
	removeobject LOCALID_HALLWAY_TODD
	setflag FLAG_HIDE_HALLWAY_TODD
	setvar VAR_PROLOGUE_STATE, 3
	releaseall
	end

@ Fallback if player tries to A Todd somehow during the scene.
Vault42_2F_Hallway_EventScript_ToddIdle::
	end

@ Second hallway cutscene — fires once when player crosses (25,21)/(26,21)/(27,21)
@ with VAR_PROLOGUE_STATE == 3. Todd reveals at (26,25), rushes up to player,
@ delivers his line, then leads the player south + west to (4,25)/(5,25).
Vault42_2F_Hallway_EventScript_ToddSecondScene::
	lockall
	setobjectxyperm LOCALID_HALLWAY_TODD, 26, 25
	clearflag FLAG_HIDE_HALLWAY_TODD
	addobject LOCALID_HALLWAY_TODD
	playse SE_PIN
	applymovement LOCALID_HALLWAY_TODD, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 25, Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol25
	call_if_eq VAR_TEMP_0, 26, Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol26
	call_if_eq VAR_TEMP_0, 27, Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol27
	setspeaker SP_NAME_TODD
	msgbox Vault42_2F_Hallway_Text_ToddSecondScene, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_0, 25, Vault42_2F_Hallway_EventScript_BothSouthFromCol25
	call_if_eq VAR_TEMP_0, 26, Vault42_2F_Hallway_EventScript_BothSouthFromCol26
	call_if_eq VAR_TEMP_0, 27, Vault42_2F_Hallway_EventScript_BothSouthFromCol27
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_ToddCornerStep
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_PlayerCornerStep
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_WalkWestLong
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_WalkWestLong
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_FaceUp
	waitmovement 0
	setobjectxyperm LOCALID_HALLWAY_TODD, 4, 25
	setobjectmovementtype LOCALID_HALLWAY_TODD, MOVEMENT_TYPE_FACE_UP
	setvar VAR_PROLOGUE_STATE, 4
	releaseall
	end

Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol25::
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_ToddUpFromCol25
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol26::
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_ToddUpFromCol26
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol27::
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_ToddUpFromCol27
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_BothSouthFromCol25::
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_SouthFromCol25
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_SouthFromCol25
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_BothSouthFromCol26::
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_SouthFromCol26
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_SouthFromCol26
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_BothSouthFromCol27::
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_SouthFromCol27
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_SouthFromCol27
	waitmovement 0
	return

Vault42_2F_Hallway_Movement_ToddUpFromCol25:
	walk_fast_left
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Vault42_2F_Hallway_Movement_ToddUpFromCol26:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Vault42_2F_Hallway_Movement_ToddUpFromCol27:
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Vault42_2F_Hallway_Movement_SouthFromCol25:
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Vault42_2F_Hallway_Movement_SouthFromCol26:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Vault42_2F_Hallway_Movement_SouthFromCol27:
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Vault42_2F_Hallway_Movement_ToddCornerStep:
	walk_fast_left
	step_end

Vault42_2F_Hallway_Movement_PlayerCornerStep:
	walk_fast_down
	step_end

Vault42_2F_Hallway_Movement_WalkWestLong:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Vault42_2F_Hallway_Movement_FaceUp:
	face_up
	step_end

Vault42_2F_Hallway_Text_ToddSecondScene:
	.string "I already carried you once today.\n"
	.string "Don't make me do it twice.\p"
	.string "Hurry!$"

Vault42_2F_Hallway_Movement_ToddWalkToPlayer:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	face_up
	step_end

Vault42_2F_Hallway_Movement_ToddRunOffEast:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

Vault42_2F_Hallway_Text_ToddCatchUp:
	.string "Took you long enough.\p"
	.string "Come on, we can't waste\n"
	.string "any more time!\p"
	.string "Try and keep up.$"

Vault42_2F_Hallway_EventScript_SignMedicalBay::
	msgbox Vault42_2F_Hallway_Text_SignMedicalBay, MSGBOX_SIGN
	end

Vault42_2F_Hallway_EventScript_SignTrashRoom::
	msgbox Vault42_2F_Hallway_Text_SignTrashRoom, MSGBOX_SIGN
	end

Vault42_2F_Hallway_EventScript_SignDiningHall::
	msgbox Vault42_2F_Hallway_Text_SignDiningHall, MSGBOX_SIGN
	end

Vault42_2F_Hallway_EventScript_SignEducationCenter::
	msgbox Vault42_2F_Hallway_Text_SignEducationCenter, MSGBOX_SIGN
	end

Vault42_2F_Hallway_EventScript_SignRestrooms::
	msgbox Vault42_2F_Hallway_Text_SignRestrooms, MSGBOX_SIGN
	end

Vault42_2F_Hallway_EventScript_SignOverseersOffice::
	msgbox Vault42_2F_Hallway_Text_SignOverseersOffice, MSGBOX_SIGN
	end

Vault42_2F_Hallway_EventScript_SignElevator::
	msgbox Vault42_2F_Hallway_Text_SignElevator, MSGBOX_SIGN
	end

Vault42_2F_Hallway_Text_SignMedicalBay:
	.string "MEDICAL BAY$"

Vault42_2F_Hallway_Text_SignTrashRoom:
	.string "TRASH ROOM$"

Vault42_2F_Hallway_Text_SignDiningHall:
	.string "DINING HALL$"

Vault42_2F_Hallway_Text_SignEducationCenter:
	.string "EDUCATION CENTER$"

Vault42_2F_Hallway_Text_SignRestrooms:
	.string "RESTROOMS$"

Vault42_2F_Hallway_Text_SignOverseersOffice:
	.string "OVERSEER'S OFFICE$"

Vault42_2F_Hallway_Text_SignElevator:
	.string "ELEVATOR$"
