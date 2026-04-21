@ Fallout Evolution terminal system.
@
@ Architecture:
@   - Each terminal tile has MB_TERMINAL metatile behavior.
@   - Each terminal ALSO has a bg_script_event at its tile position, pointing
@     at a per-terminal EventScript_Terminal_<Name> script.
@   - BG events fire before metatile dispatch, so tiles with both work via
@     the BG script. Tiles with MB_TERMINAL but no BG event fall through to
@     EventScript_TerminalOffline as a helpful misconfiguration fallback.
@
@ Adding a new terminal:
@   1. Reserve a flag in include/constants/flags.h: FLAG_TERMINAL_<Name>
@   2. Add EventScript_Terminal_<Name> and Terminal_Content_<Name> below,
@      using one of the two macros.
@   3. In porymap, place the MB_TERMINAL tile + a Sign Event pointing at
@      EventScript_Terminal_<Name>.


@ -------- Macros --------

@ Terminal with no password — boot, announce "access granted" on first visit
@ (silently on subsequent visits), run content, shut down.
.macro terminal_no_password flag:req, contentscript:req
	lockall
	special DoTerminalTurnOnEffect
	playse SE_PC_ON
	message Terminal_Text_BootUp
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	@ If already accessed, skip "access granted" and jump to content.
	goto_if_set \flag, \contentscript
	@ First access ceremony.
	message Terminal_Text_AccessGranted
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	setflag \flag
	addvar VAR_TERMINALS_ACCESSED_COUNT, 1
	goto \contentscript
.endm

@ Terminal with password — if unlocked (flag set), show content.
@ If still locked, show "password protected" warning and shut down.
@ Phase 3/4 will replace the "shut down" branch with the hacking minigame.
.macro terminal_password_protected flag:req, contentscript:req
	lockall
	special DoTerminalTurnOnEffect
	playse SE_PC_ON
	message Terminal_Text_BootUp
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	@ If already unlocked, skip warning and jump to content.
	goto_if_set \flag, \contentscript
	@ Still locked — warn and exit. Minigame hook goes here later.
	message Terminal_Text_PasswordProtected
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	goto Terminal_Shutdown
.endm


@ -------- Shared shutdown --------

Terminal_Shutdown::
	playse SE_PC_OFF
	special DoTerminalTurnOffEffect
	releaseall
	end


@ -------- Shared text --------

Terminal_Text_BootUp:
	.string "{PLAYER} booting the\n"
	.string "Vault-Tec Terminal...$"

Terminal_Text_AccessGranted:
	.string "Access granted!$"

Terminal_Text_PasswordProtected:
	.string "This machine is\n"
	.string "password protected.$"

Terminal_Text_Offline:
	.string "TERMINAL OFFLINE.\p"
	.string "Please contact Vault-Tec support.$"


@ -------- Fallback for MB_TERMINAL tiles with no bg_script_event --------

EventScript_TerminalOffline::
	lockall
	message Terminal_Text_Offline
	waitmessage
	waitbuttonpress
	playse SE_SELECT
	closemessage
	releaseall
	end


@ -------- Test terminal (no password) --------

EventScript_Terminal_Test::
	terminal_no_password FLAG_TERMINAL_TEST, Terminal_Content_Test
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
