@ Adding a new terminal:
@   1. Reserve FLAG_TERMINAL_<Name> in include/constants/flags.h
@   2. Add EventScript_Terminal_<Name> + Terminal_Content_<Name> below,
@      using the `terminal` macro.
@   3. In porymap, place the MB_TERMINAL tile + a Sign Event pointing at
@      EventScript_Terminal_<Name>.
@
@ MB_TERMINAL tiles without a bg_script_event fall through to
@ EventScript_TerminalOffline.

@ Usage: terminal <flag>, <contentscript>, <tier>
@   tier = TERMINAL_TIER_0 (no password) or TERMINAL_TIER_1..3 (hack).
.macro terminal flag:req, contentscript:req, tier:req
	lockall
	special DoTerminalTurnOnEffect
	playse SE_PC_ON
	message Terminal_Text_BootUp
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	goto_if_set \flag, \contentscript
.if \tier == TERMINAL_TIER_0
	message Terminal_Text_AccessGranted
	waitmessage
	waitbuttonpress
	playse SE_SELECT
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
	special StartTerminalHack
	waitstate
	goto_if_eq VAR_RESULT, 1, Terminal_HackSuccess_\@
	goto Terminal_Shutdown
Terminal_HackSuccess_\@:
.endif
	setflag \flag
	addvar VAR_TERMINALS_ACCESSED_COUNT, 1
	goto \contentscript
.endm


Terminal_Shutdown::
	playse SE_PC_OFF
	special DoTerminalTurnOffEffect
	releaseall
	end


Terminal_ShowLockedOut::
	message Terminal_Text_LockedOut
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	goto Terminal_Shutdown


Terminal_Text_BootUp:
	.string "{PLAYER} booted the\n"
	.string "Vault-Tec Terminal...$"

Terminal_Text_AccessGranted:
	.string "Access granted!$"

Terminal_Text_PasswordProtected:
	.string "This machine is\n"
	.string "password protected.$"

Terminal_Text_LockedOut:
	.string "Security system engaged.\p"
	.string "Walk it off and try\n"
	.string "again later.$"

Terminal_Text_Offline:
	.string "TERMINAL OFFLINE.\p"
	.string "Please contact Vault-Tec support.$"


EventScript_TerminalOffline::
	lockall
	message Terminal_Text_Offline
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	releaseall
	end


@ TODO: swap to TERMINAL_TIER_0 (or remove) once real terminals ship.
EventScript_Terminal_Test::
	terminal FLAG_TERMINAL_TEST, Terminal_Content_Test, TERMINAL_TIER_1
	end

Terminal_Content_Test::
	message Terminal_Text_TestContent
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	goto Terminal_Shutdown
	end

Terminal_Text_TestContent:
	.string "TEST TRANSMISSION$"
