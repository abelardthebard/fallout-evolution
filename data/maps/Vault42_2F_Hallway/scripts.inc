@ Local IDs (array order in map.json, 1-indexed):
@   1 = Todd (first scene, north end at (11, 3))
@   2 = Mrs. Bauer (door at (4, 23))
@   3 = Todd_Later (second/third scene, south corridor at (26, 25))
.set LOCALID_HALLWAY_TODD, 1
.set LOCALID_HALLWAY_MRSBAUER, 2
.set LOCALID_HALLWAY_TODD_LATER, 3

Vault42_2F_Hallway_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Vault42_2F_Hallway_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Vault42_2F_Hallway_OnFrame
	.byte 0

Vault42_2F_Hallway_OnFrame:
	map_script_2 VAR_PROLOGUE_STATE, 2, Vault42_2F_Hallway_EventScript_ToddCatchup1
	map_script_2 VAR_TODD_HALLWAY_DETOUR, 2, Vault42_2F_Hallway_EventScript_DetourFromMedbay
	map_script_2 VAR_TODD_HALLWAY_DETOUR, 3, Vault42_2F_Hallway_EventScript_DetourFromTrash
	map_script_2 VAR_TODD_HALLWAY_DETOUR, 4, Vault42_2F_Hallway_EventScript_DetourFromDining
	map_script_2 VAR_TODD_HALLWAY_DETOUR, 5, Vault42_2F_Hallway_EventScript_DetourFromElevator
	.2byte 0

Vault42_2F_Hallway_OnTransition::
	@ Stage-NPC hide-flag recovery (resilient against stale saves).
	setflag FLAG_HIDE_HALLWAY_MRSBAUER
	call_if_ge VAR_PROLOGUE_STATE, 3, Vault42_2F_Hallway_EventScript_HideFirstTodd
	call_if_lt VAR_PROLOGUE_STATE, 3, Vault42_2F_Hallway_EventScript_HideToddLater
	call_if_ge VAR_PROLOGUE_STATE, 5, Vault42_2F_Hallway_EventScript_HideToddLater
	goto_if_eq VAR_TODD_HALLWAY_DETOUR, 2, Vault42_2F_Hallway_OnTransition_StageMedbay
	goto_if_eq VAR_TODD_HALLWAY_DETOUR, 3, Vault42_2F_Hallway_OnTransition_StageTrash
	goto_if_eq VAR_TODD_HALLWAY_DETOUR, 4, Vault42_2F_Hallway_OnTransition_StageDining
	goto_if_eq VAR_TODD_HALLWAY_DETOUR, 5, Vault42_2F_Hallway_OnTransition_StageElevator
	end

Vault42_2F_Hallway_EventScript_HideFirstTodd::
	setflag FLAG_HIDE_HALLWAY_TODD
	return

Vault42_2F_Hallway_EventScript_HideToddLater::
	setflag FLAG_HIDE_HALLWAY_TODD_LATER
	return

Vault42_2F_Hallway_OnTransition_StageMedbay::
	setobjectxyperm LOCALID_HALLWAY_TODD_LATER, 7, 3
	setobjectmovementtype LOCALID_HALLWAY_TODD_LATER, MOVEMENT_TYPE_FACE_UP
	end

Vault42_2F_Hallway_OnTransition_StageTrash::
	setobjectxyperm LOCALID_HALLWAY_TODD_LATER, 18, 3
	setobjectmovementtype LOCALID_HALLWAY_TODD_LATER, MOVEMENT_TYPE_FACE_UP
	end

Vault42_2F_Hallway_OnTransition_StageDining::
	setobjectxyperm LOCALID_HALLWAY_TODD_LATER, 31, 3
	setobjectmovementtype LOCALID_HALLWAY_TODD_LATER, MOVEMENT_TYPE_FACE_UP
	end

Vault42_2F_Hallway_OnTransition_StageElevator::
	setobjectxyperm LOCALID_HALLWAY_TODD_LATER, 30, 13
	setobjectmovementtype LOCALID_HALLWAY_TODD_LATER, MOVEMENT_TYPE_FACE_UP
	end

@ First hallway cutscene — fires via ON_FRAME when player arrives in the hallway
@ with VAR_PROLOGUE_STATE == 2. Todd is already visible at his perm position
@ (11, 3) from the moment the player enters. At the end the first Todd object
@ is despawned permanently and the second Todd object (Todd_Later, at (26, 25)
@ facing up) is revealed — same Wally-style two-object pattern used by vanilla
@ pokeemerald to relocate an NPC across cutscenes without in-script repositioning.
Vault42_2F_Hallway_EventScript_ToddCatchup1::
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
	setflag FLAG_HIDE_HALLWAY_TODD
	clearflag FLAG_HIDE_HALLWAY_TODD_LATER
	addobject LOCALID_HALLWAY_TODD_LATER
	setvar VAR_PROLOGUE_STATE, 3
	releaseall
	end

@ Fallback if player tries to A Todd somehow during the scene.
Vault42_2F_Hallway_EventScript_ToddIdle::
	end

@ Fallback for Mrs. Bauer (she's only visible during the door cutscene).
Vault42_2F_Hallway_EventScript_MrsBauerIdle::
	end

Vault42_2F_Hallway_EventScript_ToddCatchup2A::
	lockall
	playse SE_PIN
	applymovement LOCALID_HALLWAY_TODD_LATER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 25, Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol25
	call_if_eq VAR_TEMP_0, 26, Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol26
	call_if_eq VAR_TEMP_0, 27, Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol27
	goto Vault42_2F_Hallway_EventScript_ToddCatchup2A_Core

Vault42_2F_Hallway_EventScript_ToddCatchup2A_FromDetour::
	goto Vault42_2F_Hallway_EventScript_ToddCatchup2A_Core

Vault42_2F_Hallway_EventScript_ToddCatchup2A_Core::
	setspeaker SP_NAME_TODD
	msgbox Vault42_2F_Hallway_Text_ToddCatchup2A, MSGBOX_DEFAULT
	closemessage
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 25, Vault42_2F_Hallway_EventScript_BothSouthFromCol25
	call_if_eq VAR_TEMP_0, 26, Vault42_2F_Hallway_EventScript_BothSouthFromCol26
	call_if_eq VAR_TEMP_0, 27, Vault42_2F_Hallway_EventScript_BothSouthFromCol27
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_ToddCornerStep
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_PlayerCornerStep
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_WalkWestLong
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_WalkWestLong
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_FaceUp
	waitmovement 0

	@ Mrs. Bauer comes out of the door at (4, 23)
	opendoor 4, 23
	waitdooranim
	clearflag FLAG_HIDE_HALLWAY_MRSBAUER
	addobject LOCALID_HALLWAY_MRSBAUER
	applymovement LOCALID_HALLWAY_MRSBAUER, Vault42_2F_Hallway_Movement_BauerWalkOutOfDoor
	applymovement LOCALID_PLAYER, Common_Movement_FaceUp
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
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_ToddEnterDoor
	waitmovement 0
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_PlayerEnterDoor
	waitmovement 0
	hideplayer
	closedoor 4, 23
	waitdooranim

	@ Persist hidden state for both NPCs in the hallway
	removeobject LOCALID_HALLWAY_MRSBAUER
	removeobject LOCALID_HALLWAY_TODD_LATER
	setflag FLAG_HIDE_HALLWAY_MRSBAUER
	setflag FLAG_HIDE_HALLWAY_TODD_LATER

	@ Time-jump: fade to black, hold a beat, Mrs. Bauer narrates over black
	fadescreen FADE_TO_BLACK
	delay 60
	msgbox Vault42_2F_Hallway_Text_MrsBauerTimeJump, MSGBOX_DEFAULT
	closemessage

	@ Reveal classroom NPCs and warp player to the classroom
	clearflag FLAG_HIDE_CLASSROOM_MRSBAUER
	clearflag FLAG_HIDE_CLASSROOM_TODD
	setvar VAR_PROLOGUE_STATE, 5
	warp MAP_VAULT42_CLASSROOM, 4, 6
	waitstate
	end

Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol25::
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_ToddUpFromCol25
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol26::
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_ToddUpFromCol26
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_ToddWalkUpFromCol27::
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_ToddUpFromCol27
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_BothSouthFromCol25::
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_SouthFromCol25
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_SouthFromCol25
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_BothSouthFromCol26::
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_SouthFromCol26
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_SouthFromCol26
	waitmovement 0
	return

Vault42_2F_Hallway_EventScript_BothSouthFromCol27::
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_SouthFromCol27
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
	face_left
	walk_left
	walk_up
	walk_up
	step_end

Vault42_2F_Hallway_EventScript_DetourFromMedbay::
	lockall
	setvar VAR_TODD_HALLWAY_DETOUR, 9
	call Vault42_2F_Hallway_EventScript_DillyDallyCommonSetup
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_DetourFromMedbay
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_DetourFromMedbay_Player
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD_LATER, Common_Movement_FaceUp
	applymovement LOCALID_PLAYER, Common_Movement_FaceDown
	waitmovement 0
	goto Vault42_2F_Hallway_EventScript_ToddCatchup2A_FromDetour

Vault42_2F_Hallway_EventScript_DetourFromTrash::
	lockall
	setvar VAR_TODD_HALLWAY_DETOUR, 9
	call Vault42_2F_Hallway_EventScript_DillyDallyCommonSetup
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_DetourFromTrash
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_DetourFromTrash_Player
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD_LATER, Common_Movement_FaceUp
	applymovement LOCALID_PLAYER, Common_Movement_FaceDown
	waitmovement 0
	goto Vault42_2F_Hallway_EventScript_ToddCatchup2A_FromDetour

Vault42_2F_Hallway_EventScript_DetourFromDining::
	lockall
	setvar VAR_TODD_HALLWAY_DETOUR, 9
	call Vault42_2F_Hallway_EventScript_DillyDallyCommonSetup
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_DetourFromDining
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_DetourFromDining_Player
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD_LATER, Common_Movement_FaceUp
	applymovement LOCALID_PLAYER, Common_Movement_FaceDown
	waitmovement 0
	goto Vault42_2F_Hallway_EventScript_ToddCatchup2A_FromDetour

Vault42_2F_Hallway_EventScript_DetourFromElevator::
	lockall
	setvar VAR_TODD_HALLWAY_DETOUR, 9
	call Vault42_2F_Hallway_EventScript_DillyDallyCommonSetup
	applymovement LOCALID_HALLWAY_TODD_LATER, Vault42_2F_Hallway_Movement_DetourFromElevator
	applymovement LOCALID_PLAYER, Vault42_2F_Hallway_Movement_DetourFromElevator_Player
	waitmovement 0
	applymovement LOCALID_HALLWAY_TODD_LATER, Common_Movement_FaceUp
	applymovement LOCALID_PLAYER, Common_Movement_FaceDown
	waitmovement 0
	goto Vault42_2F_Hallway_EventScript_ToddCatchup2A_FromDetour

Vault42_2F_Hallway_EventScript_DillyDallyCommonSetup::
	playse SE_PIN
	applymovement LOCALID_HALLWAY_TODD_LATER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	setspeaker SP_NAME_TODD
	msgbox Vault42_2F_Hallway_Text_ToddDillyDally, MSGBOX_DEFAULT
	closemessage
	return

Vault42_2F_Hallway_Movement_DetourFromMedbay:
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
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Vault42_2F_Hallway_Movement_DetourFromTrash:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Vault42_2F_Hallway_Movement_DetourFromDining:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Vault42_2F_Hallway_Movement_DetourFromElevator:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

@ Player follows Todd's path with a leading walk_down (drop from row 2 / 12 to Todd's row)
@ and one fewer south step so she trails 1 tile north of Todd at the end (alignment for BothSouthFromCol26).
Vault42_2F_Hallway_Movement_DetourFromMedbay_Player:
	walk_fast_down        @ (7,2) -> (7,3) — onto Todd's row
	walk_fast_right       @ -> (8,3)
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
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right       @ -> (26,3)
	walk_fast_down        @ -> (26,4)
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down        @ -> (26,21)
	step_end

Vault42_2F_Hallway_Movement_DetourFromTrash_Player:
	walk_fast_down        @ (18,2) -> (18,3)
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right       @ -> (26,3)
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down        @ -> (26,21)
	step_end

Vault42_2F_Hallway_Movement_DetourFromDining_Player:
	walk_fast_down        @ (31,2) -> (31,3)
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left        @ -> (26,3)
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down        @ -> (26,21)
	step_end

Vault42_2F_Hallway_Movement_DetourFromElevator_Player:
	walk_fast_down        @ (30,12) -> (30,13)
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left        @ -> (26,13)
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down        @ -> (26,21)
	step_end

Vault42_2F_Hallway_Text_ToddDillyDally:
	.string "Are you lost?\p"
	.string "I'm not going to be late to\n"
	.string "the most important test of my life\l"
	.string "because of you.\p"
	.string "Come on!$"

Vault42_2F_Hallway_Text_ToddCatchup2A:
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

Vault42_2F_Hallway_Text_MrsBauerTimeJump:
	.string "Time's up!\p"
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
