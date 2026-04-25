@ MB_TERMINAL tiles without a bg_script_event fall through to EventScript_TerminalOffline.

.macro terminal flag:req, contentId:req, tier:req
	lockall
	special DoTerminalTurnOnEffect
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
	addvar VAR_TERMINALS_ACCESSED_COUNT, 1
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
	goto Terminal_Shutdown
Terminal_HackSuccess_\@:
	setflag \flag
	addvar VAR_TERMINALS_ACCESSED_COUNT, 1
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
	special DoTerminalTurnOffEffect
	releaseall
	end


Terminal_ShowLockedOut::
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
