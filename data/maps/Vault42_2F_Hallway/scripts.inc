@ Local IDs (array order in map.json, 1-indexed):
@   1 = Todd
@   2 = Mrs. Bauer
.set LOCALID_HALLWAY_TODD, 1
.set LOCALID_HALLWAY_MRSBAUER, 2

Vault42_2F_Hallway_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Vault42_2F_Hallway_OnFrame
	.byte 0

Vault42_2F_Hallway_OnFrame:
	map_script_2 VAR_PROLOGUE_STATE, 2, Vault42_2F_Hallway_EventScript_ToddCatchUpScene
	.2byte 0

@ First hallway cutscene — fires via ON_FRAME when player arrives in the hallway
@ with VAR_PROLOGUE_STATE == 2. Todd is already visible at his perm position
@ (11, 3) from the moment the player enters; no clearflag/addobject needed at
@ start. At the end Todd transitions to (26, 25) facing up so he is visible
@ waiting for the second cutscene.
Vault42_2F_Hallway_EventScript_ToddCatchUpScene::
	lockall
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
	setobjectxyperm LOCALID_HALLWAY_TODD, 26, 25
	setobjectmovementtype LOCALID_HALLWAY_TODD, MOVEMENT_TYPE_FACE_UP
	clearflag FLAG_HIDE_HALLWAY_TODD
	addobject LOCALID_HALLWAY_TODD
	setvar VAR_PROLOGUE_STATE, 3
	releaseall
	end

@ Fallback if player tries to A Todd somehow during the scene.
Vault42_2F_Hallway_EventScript_ToddIdle::
	end

@ Fallback for Mrs. Bauer (she's only visible during the door cutscene).
Vault42_2F_Hallway_EventScript_MrsBauerIdle::
	end

@ Second hallway cutscene — fires once when player crosses (25,21)/(26,21)/(27,21)
@ with VAR_PROLOGUE_STATE == 3. Todd reveals at (26,25), rushes up to player,
@ delivers his line, then leads the player south + west to (4,25)/(5,25).
Vault42_2F_Hallway_EventScript_ToddSecondScene::
	lockall
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

	@ Mrs. Bauer comes out of the door at (4, 23)
	opendoor 4, 23
	waitdooranim
	clearflag FLAG_HIDE_HALLWAY_MRSBAUER
	addobject LOCALID_HALLWAY_MRSBAUER
	applymovement LOCALID_HALLWAY_MRSBAUER, Vault42_2F_Hallway_Movement_BauerWalkOutOfDoor
	waitmovement 0
	closedoor 4, 23
	waitdooranim

	@ Mrs. Bauer's pre-entry dialogue
	setspeaker SP_NAME_MRSBAUER
	msgbox Vault42_2F_Hallway_Text_MrsBauerArrival, MSGBOX_DEFAULT
	closemessage

	@ Trio re-enters the door (sequential — Bauer first, Todd second, Player last)
	opendoor 4, 23
	waitdooranim
	applymovement LOCALID_HALLWAY_MRSBAUER, Vault42_2F_Hallway_Movement_BauerEnterDoor
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD, Vault42_2F_Hallway_Movement_ToddEnterDoor
	waitmovement 0
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_PlayerEnterDoor
	waitmovement 0
	hideplayer
	closedoor 4, 23
	waitdooranim

	@ Persist hidden state for both NPCs in the hallway
	removeobject LOCALID_HALLWAY_MRSBAUER
	removeobject LOCALID_HALLWAY_TODD
	setflag FLAG_HIDE_HALLWAY_MRSBAUER
	setflag FLAG_HIDE_HALLWAY_TODD

	@ Time-jump: fade to black, hold a beat, Mrs. Bauer narrates over black
	fadescreen FADE_TO_BLACK
	delay 60
	setspeaker SP_NAME_MRSBAUER
	msgbox Vault42_2F_Hallway_Text_MrsBauerTimesUp, MSGBOX_DEFAULT
	closemessage
	setspeaker SP_NAME_MRSBAUER
	msgbox Vault42_2F_Hallway_Text_MrsBauerCheckScores, MSGBOX_DEFAULT
	closemessage

	@ Reveal classroom NPCs and warp player to the classroom
	clearflag FLAG_HIDE_CLASSROOM_MRSBAUER
	clearflag FLAG_HIDE_CLASSROOM_TODD
	warp MAP_VAULT42_CLASSROOM, 8, 12
	waitstate

	@ Face player north on classroom arrival
	applymovement LOCALID_PLAYER, Common_Movement_FaceUp
	waitmovement 0

	setvar VAR_PROLOGUE_STATE, 5
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

Vault42_2F_Hallway_Movement_BauerWalkOutOfDoor:
	walk_down
	step_end

Vault42_2F_Hallway_Movement_BauerEnterDoor:
	walk_up
	set_invisible
	step_end

Vault42_2F_Hallway_Movement_ToddEnterDoor:
	walk_up
	walk_up
	set_invisible
	step_end

Vault42_2F_Hallway_Movement_PlayerEnterDoor:
	walk_left
	walk_up
	walk_up
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

Vault42_2F_Hallway_Text_MrsBauerArrival:
	.string "There you are, dears!\p"
	.string "I was just about to come\n"
	.string "looking for you.\p"
	.string "In, in! The G.O.A.T. has\n"
	.string "already begun!$"

Vault42_2F_Hallway_Text_MrsBauerTimesUp:
	.string "Time's up!$"

Vault42_2F_Hallway_Text_MrsBauerCheckScores:
	.string "You may check your scores\n"
	.string "on the classroom terminal…$"

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
