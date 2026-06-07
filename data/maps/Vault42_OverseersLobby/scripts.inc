.set LOCALID_LOBBY_SECRETARY, 1

@ Time-of-day buckets written by `gettimeofday` into VAR_0x8000. These mirror
@ enum TimeOfDay (constants/rtc.h), which is C-only and can't reach the script
@ assembler -- keep in sync if that enum is ever reordered.
.set TOD_MORNING, 0
.set TOD_DAY,     1
.set TOD_EVENING, 2
.set TOD_NIGHT,   3

Vault42_OverseersLobby_MapScripts::
	.byte 0

@ ============================================================================
@ Regine, the Overseer's receptionist.
@   * Talked from her RIGHT (4,2 -> facing WEST), her only adjacent tile: the
@     escalating "wrong side" brush-off gag (per-visit, VAR_TEMP_5).
@   * Talked ACROSS THE DESK (3,4 facing NORTH, over the MB_COUNTER desk tile
@     at 3,3): the real reception flow -- time-of-day greeting, a dynamic
@     multichoice menu, and the appointment check that unlocks the Overseer's
@     door via VAR_OVERSEER_DOOR (fail-closed: 0 locked / 1 open).
@ ============================================================================
Vault42_OverseersLobby_EventScript_SecretaryIdle::
	lock
	faceplayer
	setspeaker SP_NAME_REGINE
	goto_if_eq VAR_FACING, DIR_NORTH, Vault42_OverseersLobby_EventScript_RegineDesk
	@ --- side approach: escalating brush-off ---
	goto_if_eq VAR_TEMP_5, 0, Vault42_OverseersLobby_EventScript_RegineRebuff1
	goto_if_eq VAR_TEMP_5, 1, Vault42_OverseersLobby_EventScript_RegineRebuff2
	goto Vault42_OverseersLobby_EventScript_RegineRebuff3

Vault42_OverseersLobby_EventScript_RegineRebuff1::
	setvar VAR_TEMP_5, 1
	msgbox Vault42_OverseersLobby_Text_RegineRebuff1, MSGBOX_DEFAULT
	release
	end

Vault42_OverseersLobby_EventScript_RegineRebuff2::
	setvar VAR_TEMP_5, 2
	msgbox Vault42_OverseersLobby_Text_RegineRebuff2, MSGBOX_DEFAULT
	release
	end

Vault42_OverseersLobby_EventScript_RegineRebuff3::
	msgbox Vault42_OverseersLobby_Text_RegineRebuff3, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------------------
@ Desk reception (talked across the counter).
@ ---------------------------------------------------------------------------
Vault42_OverseersLobby_EventScript_RegineDesk::
	@ Power's out -> a brief reassurance that supersedes the normal reception flow.
	goto_if_set FLAG_EMERGENCY_LIGHTING, Vault42_OverseersLobby_EventScript_ReginePowerOff
	@ Door already unlocked (appointment confirmed, not gone in yet): wave through.
	goto_if_eq VAR_OVERSEER_DOOR, OVERSEER_DOOR_OPEN, Vault42_OverseersLobby_EventScript_RegineGoOnIn
	call Vault42_OverseersLobby_EventScript_RegineBufferGreeting
	msgbox Vault42_OverseersLobby_Text_RegineWelcome, MSGBOX_DEFAULT
	dynmultipush Vault42_OverseersLobby_Text_RegineMenuVisit, 0
	dynmultipush Vault42_OverseersLobby_Text_RegineMenuNevermind, 1
	dynmultistack 20, 8, FALSE, 2, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, Vault42_OverseersLobby_EventScript_RegineVisitOverseer
	goto Vault42_OverseersLobby_EventScript_RegineGoodbye

@ Door already open -> brief wave-in.
Vault42_OverseersLobby_EventScript_RegineGoOnIn::
	msgbox Vault42_OverseersLobby_Text_RegineGoOnIn, MSGBOX_DEFAULT
	release
	end

@ Power-out reaction (desk only; the side approach keeps the rebuff gag).
Vault42_OverseersLobby_EventScript_ReginePowerOff::
	msgbox Vault42_OverseersLobby_Text_ReginePowerOff, MSGBOX_DEFAULT
	release
	end

@ "Visit Overseer" -> appointment check.
Vault42_OverseersLobby_EventScript_RegineVisitOverseer::
	msgbox Vault42_OverseersLobby_Text_RegineAppointmentQ, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Vault42_OverseersLobby_EventScript_RegineNoAppointment
	@ Claims to have one -> only valid once Bauer has actually sent them.
	goto_if_eq VAR_PROLOGUE_STATE, PROLOGUE_SENT_TO_OVERSEER, Vault42_OverseersLobby_EventScript_RegineConfirm
	@ Claims yes, but nothing on file.
	msgbox Vault42_OverseersLobby_Text_RegineNoRecord, MSGBOX_DEFAULT
	goto Vault42_OverseersLobby_EventScript_RegineOfferBooking

@ Said "no".
Vault42_OverseersLobby_EventScript_RegineNoAppointment::
	msgbox Vault42_OverseersLobby_Text_RegineCantLetYouThrough, MSGBOX_DEFAULT
	goto Vault42_OverseersLobby_EventScript_RegineOfferBooking

@ Shared booking offer (reached from "no" and from "yes, but no record").
Vault42_OverseersLobby_EventScript_RegineOfferBooking::
	msgbox Vault42_OverseersLobby_Text_RegineOfferBooking, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Vault42_OverseersLobby_EventScript_RegineBookIt
	goto Vault42_OverseersLobby_EventScript_RegineGoodbye

Vault42_OverseersLobby_EventScript_RegineBookIt::
	msgbox Vault42_OverseersLobby_Text_RegineBooked, MSGBOX_DEFAULT
	release
	end

@ Appointment confirmed -> unlock the door. The Overseer scene re-locks it later.
Vault42_OverseersLobby_EventScript_RegineConfirm::
	msgbox Vault42_OverseersLobby_Text_RegineConfirmed, MSGBOX_DEFAULT
	setvar VAR_OVERSEER_DOOR, OVERSEER_DOOR_OPEN
	release
	end

@ Time-of-day sign-off.
Vault42_OverseersLobby_EventScript_RegineGoodbye::
	call Vault42_OverseersLobby_EventScript_RegineBufferGoodbye
	msgbox Vault42_OverseersLobby_Text_RegineGoodbye, MSGBOX_DEFAULT
	release
	end

@ Buffer the greeting word into {STR_VAR_1}: morning / afternoon / evening(+night).
Vault42_OverseersLobby_EventScript_RegineBufferGreeting::
	gettimeofday
	goto_if_eq VAR_0x8000, TOD_MORNING, Vault42_OverseersLobby_EventScript_RegineGreetMorning
	goto_if_eq VAR_0x8000, TOD_DAY, Vault42_OverseersLobby_EventScript_RegineGreetAfternoon
	bufferstring STR_VAR_1, Vault42_OverseersLobby_Text_GreetEvening
	return
Vault42_OverseersLobby_EventScript_RegineGreetMorning::
	bufferstring STR_VAR_1, Vault42_OverseersLobby_Text_GreetMorning
	return
Vault42_OverseersLobby_EventScript_RegineGreetAfternoon::
	bufferstring STR_VAR_1, Vault42_OverseersLobby_Text_GreetAfternoon
	return

@ Buffer the sign-off word into {STR_VAR_1}: day / afternoon / night(+evening).
Vault42_OverseersLobby_EventScript_RegineBufferGoodbye::
	gettimeofday
	goto_if_eq VAR_0x8000, TOD_MORNING, Vault42_OverseersLobby_EventScript_RegineByeDay
	goto_if_eq VAR_0x8000, TOD_DAY, Vault42_OverseersLobby_EventScript_RegineByeAfternoon
	bufferstring STR_VAR_1, Vault42_OverseersLobby_Text_ByeNight
	return
Vault42_OverseersLobby_EventScript_RegineByeDay::
	bufferstring STR_VAR_1, Vault42_OverseersLobby_Text_ByeDay
	return
Vault42_OverseersLobby_EventScript_RegineByeAfternoon::
	bufferstring STR_VAR_1, Vault42_OverseersLobby_Text_ByeAfternoon
	return

@ ---------------------------------------------------------------------------
@ Door guard. The office door (6,1) is a press-into-from-below animated door,
@ so the player stands on the threshold tile (6,2) and presses up. This trigger
@ sits on (6,2), gated to VAR_OVERSEER_DOOR == 0 (locked) -- fail-closed, so it
@ blocks in every state except the confirmed-appointment window.
@ ---------------------------------------------------------------------------
Vault42_OverseersLobby_EventScript_SecretaryBlock::
	lockall
	playse SE_PIN
	applymovement LOCALID_LOBBY_SECRETARY, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_LOBBY_SECRETARY, Common_Movement_FaceRight
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_FaceLeft
	waitmovement 0
	setspeaker SP_NAME_REGINE
	msgbox Vault42_OverseersLobby_Text_SecretaryBlock, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Vault42_OverseersLobby_Movement_PlayerLeaveDoor
	waitmovement 0
	releaseall
	end

Vault42_OverseersLobby_Movement_PlayerLeaveDoor:
	walk_down
	step_end

@ ===========================================================================
@ Text
@ ===========================================================================
Vault42_OverseersLobby_Text_SecretaryBlock:
	.string "Hold it right there.\n"
	.string "The Overseer sees residents\l"
	.string "by appointment only.\p"
	.string "If you have one, check\n"
	.string "in with me at the desk.$"

Vault42_OverseersLobby_Text_RegineRebuff1:
	.string "Nope. Wrong side.\n"
	.string "Around the front.\p"
	.string "I'm not saying it twice.$"

Vault42_OverseersLobby_Text_RegineRebuff2:
	.string "Nope. Wrong side.\n"
	.string "Around the front.\p"
	.string "I'm not saying it thrice.$"

Vault42_OverseersLobby_Text_RegineRebuff3:
	.string "This conversation happens\n"
	.string "across the desk or it\l"
	.string "doesn't happen at all.$"

@ Greeting template; {STR_VAR_1} = the time-of-day greeting word.
Vault42_OverseersLobby_Text_RegineWelcome:
	.string "{STR_VAR_1}\p"
	.string "Welcome to the Overseer's\n"
	.string "office. How can I help you?$"

Vault42_OverseersLobby_Text_GreetMorning:
	.string "Good morning!$"
Vault42_OverseersLobby_Text_GreetAfternoon:
	.string "Good afternoon!$"
Vault42_OverseersLobby_Text_GreetEvening:
	.string "Good evening!$"

Vault42_OverseersLobby_Text_RegineMenuVisit:
	.string "Visit Overseer$"
Vault42_OverseersLobby_Text_RegineMenuNevermind:
	.string "Nevermind$"

Vault42_OverseersLobby_Text_RegineGoOnIn:
	.string "Go on in.\n"
	.string "He's expecting you.$"

Vault42_OverseersLobby_Text_ReginePowerOff:
	.string "Don't worry. The Overseer\n"
	.string "is aware of the power\l"
	.string "situation.\p"
	.string "It's being handled.$"

Vault42_OverseersLobby_Text_RegineAppointmentQ:
	.string "Of course.\n"
	.string "Do you have an appointment?$"

Vault42_OverseersLobby_Text_RegineConfirmed:
	.string "Let me check…\p"
	.string "Oh, {PLAYER}, there\n"
	.string "you are!\p"
	.string "Go on in.\n"
	.string "He's expecting you.$"

Vault42_OverseersLobby_Text_RegineNoRecord:
	.string "Let me check…\p"
	.string "Hmm… I'm not seeing\n"
	.string "anything…$"

Vault42_OverseersLobby_Text_RegineCantLetYouThrough:
	.string "Then I can't let you\n"
	.string "through, I'm afraid.$"

Vault42_OverseersLobby_Text_RegineOfferBooking:
	.string "Want me to grab the soonest\n"
	.string "opening and put your name\l"
	.string "down?$"

Vault42_OverseersLobby_Text_RegineBooked:
	.string "Let's see…\p"
	.string "Booked through the next\n"
	.string "4 weeks…\p"
	.string "New appointments open every\n"
	.string "Monday. Check back after\l"
	.string "the weekend.$"

@ Sign-off template; {STR_VAR_1} = the time-of-day word.
Vault42_OverseersLobby_Text_RegineGoodbye:
	.string "Have a great {STR_VAR_1}!$"

Vault42_OverseersLobby_Text_ByeDay:
	.string "day$"
Vault42_OverseersLobby_Text_ByeAfternoon:
	.string "afternoon$"
Vault42_OverseersLobby_Text_ByeNight:
	.string "night$"
