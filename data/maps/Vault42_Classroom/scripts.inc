@ Local IDs (array order in map.json, 1-indexed):
@   1 = Mrs. Bauer
@   2 = Todd
.set LOCALID_CLASSROOM_MRSBAUER, 1
.set LOCALID_CLASSROOM_TODD, 2

Vault42_Classroom_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Vault42_Classroom_OnFrame
	.byte 0

Vault42_Classroom_OnFrame:
	map_script_2 VAR_PROLOGUE_STATE, 5, Vault42_Classroom_EventScript_AfterTimeJump
	.2byte 0

@ Fires once on classroom arrival after the time-jump (VAR_PROLOGUE_STATE == 5).
@ Todd realizes time was lost, blames the player, exits south.
Vault42_Classroom_EventScript_AfterTimeJump::
	lockall

	@ Todd: "!!" — still facing left, reacts to the lost time
	playse SE_PIN
	applymovement LOCALID_CLASSROOM_TODD, Vault42_Classroom_Movement_ToddDoubleExclaim
	waitmovement 0
	delay 30

	@ Todd and Bauer turn to face each other
	applymovement LOCALID_CLASSROOM_TODD, Common_Movement_FaceRight
	applymovement LOCALID_CLASSROOM_MRSBAUER, Common_Movement_FaceLeft
	waitmovement 0
	delay 15

	@ Todd: "?"
	playse SE_PIN
	applymovement LOCALID_CLASSROOM_TODD, Vault42_Classroom_Movement_ToddQuestion
	waitmovement 0
	delay 30

	@ Bauer: "..."
	applymovement LOCALID_CLASSROOM_MRSBAUER, Vault42_Classroom_Movement_BauerEllipsis
	waitmovement 0
	waitfieldeffect FLDEFF_ELLIPSIS_ICON

	@ Todd's path: (3,4) -> (3,3) -> (7,3) -> (7,6) -> (5,6) -> (5,9)
	applymovement LOCALID_CLASSROOM_TODD, Vault42_Classroom_Movement_ToddApproachPlayer
	waitmovement 0

	@ Todd and player face each other for the accusation
	applymovement LOCALID_CLASSROOM_TODD, Common_Movement_FaceLeft
	applymovement LOCALID_PLAYER, Common_Movement_FaceRight
	waitmovement 0

	setspeaker SP_NAME_TODD
	msgbox Vault42_Classroom_Text_ToddYourFault, MSGBOX_DEFAULT
	closemessage

	@ Todd exits south to (5,18) and vanishes
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
	release
	end

Vault42_Classroom_EventScript_BauerCheckScoresPrompt::
	msgbox Vault42_Classroom_Text_BauerCheckScores, MSGBOX_DEFAULT
	release
	end

Vault42_Classroom_EventScript_ToddIdle::
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
	walk_down             @ -> (5,7)
	walk_down             @ -> (5,8)
	walk_down             @ -> (5,9)
	step_end

Vault42_Classroom_Movement_ToddExit:
	walk_down             @ (5,9) -> (5,10)
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
