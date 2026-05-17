@ MB_TERMINAL tiles without a bg_script_event fall through to EventScript_TerminalOffline.
@ The optional `animMode` parameter (default 1 = on) controls whether the
@ camera-centred turn-on/turn-off VFX play. Set to 0 for side-facing terminals
@ where the spotlight effect would aim past the player and look wrong.

.macro terminal flag:req, contentId:req, tier:req, animMode=1
	lockall
	setvar VAR_0x8003, \animMode
.if \animMode != 0
	special DoTerminalTurnOnEffect
.endif
	playse SE_PC_ON
	message Terminal_Text_BootUp
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	goto_if_set \flag, Terminal_SkipToContent_\@
.if \tier == TERMINAL_TIER_0
	message Terminal_Text_AccessGranted
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	setflag \flag
	incrementgamestat GAME_STAT_TERMINALS_ACCESSED
	incrementgamestat GAME_STAT_TERMINALS_AUTO
	goto Terminal_SkipToContent_\@
.else
	setvar VAR_0x8000, \flag
	specialvar VAR_RESULT, IsTerminalLockedOut
	goto_if_eq VAR_RESULT, 1, Terminal_ShowLockedOut
	message Terminal_Text_PasswordProtected
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	setvar VAR_0x8000, \tier
	setvar VAR_0x8001, \flag
	setvar VAR_0x8002, \contentId
	special StartTerminalHack
	waitstate
	goto_if_eq VAR_RESULT, 1, Terminal_HackSuccess_\@
	@ Failure: distinguish lockout-triggered (attempts exhausted) from user-abort
	setvar VAR_0x8000, \flag
	specialvar VAR_RESULT, IsTerminalLockedOut
	goto_if_eq VAR_RESULT, 1, Terminal_HackFailed_\@
	goto Terminal_Shutdown
Terminal_HackFailed_\@:
	incrementgamestat GAME_STAT_TERMINAL_HACK_FAILURES
	goto Terminal_Shutdown
Terminal_HackSuccess_\@:
	setflag \flag
	incrementgamestat GAME_STAT_TERMINALS_ACCESSED
.if \tier == TERMINAL_TIER_1
	incrementgamestat GAME_STAT_HACKED_TIER1
.elseif \tier == TERMINAL_TIER_2
	incrementgamestat GAME_STAT_HACKED_TIER2
.elseif \tier == TERMINAL_TIER_3
	incrementgamestat GAME_STAT_HACKED_TIER3
.endif
	goto Terminal_Shutdown
.endif
Terminal_SkipToContent_\@:
	setvar VAR_0x8000, \contentId
	special ShowTerminalContent
	waitstate
	goto Terminal_Shutdown
.endm


Terminal_Shutdown::
	playse SE_PC_OFF
	goto_if_eq VAR_0x8003, 0, Terminal_Shutdown_NoAnim
	special DoTerminalTurnOffEffect
Terminal_Shutdown_NoAnim::
	releaseall
	end


Terminal_ShowLockedOut::
	incrementgamestat GAME_STAT_TERMINAL_LOCKOUTS_HIT
	buffernumberstring STR_VAR_1, VAR_TERMINAL_LOCKOUT_STEPS
	message Terminal_Text_LockedOut
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	goto Terminal_Shutdown


Terminal_Text_BootUp:
	.string "{PLAYER} booted up the terminal.$"

Terminal_Text_AccessGranted:
	.string "Access granted!$"

Terminal_Text_PasswordProtected:
	.string "This machine is password protected.$"

Terminal_Text_LockedOut:
	.string "The terminal is temporarily\n"
	.string "locked. Cooldown {STR_VAR_1}.$"

Terminal_Text_Offline:
	.string "TERMINAL OFFLINE.\p"
	.string "Please contact support.$"


EventScript_TerminalOffline::
	lockall
	message Terminal_Text_Offline
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	releaseall
	end

EventScript_Terminal_MedicalRecords::
	terminal FLAG_TERMINAL_MEDICAL_RECORDS, TERMINAL_CONTENT_MEDICAL_RECORDS, TERMINAL_TIER_1
	end

@ Classroom test scores terminal — side-facing, only usable when the player
@ approaches from the east (i.e. is standing at (3, 4) facing west).
@ Other approach directions show a Fallout-flavoured "wrong angle" reject.
EventScript_Terminal_TestScores::
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	goto_if_ne VAR_TEMP_0, 3, EventScript_Terminal_TestScores_Reject
	goto_if_ne VAR_TEMP_1, 4, EventScript_Terminal_TestScores_Reject
	terminal FLAG_TERMINAL_TEST_SCORES, TERMINAL_CONTENT_TEST_SCORES, TERMINAL_TIER_0, animMode=0
	end

EventScript_Terminal_TestScores_Reject::
	lockall
	msgbox Terminal_Text_TestScoresWrongAngle, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Terminal_Text_TestScoresWrongAngle:
	.string "From this angle all you see\n"
	.string "is the side of the terminal.\p"
	.string "You'll have to stand in front\n"
	.string "of the screen to use it.$"
