@ Power-outage / emergency-lighting building blocks.
@
@ EventScript_DoPowerOutage / EventScript_DoPowerRestore are CALLABLE
@ subroutines (they `return`). The caller must `lockall` first -- that freezes
@ object events so the flicker's palette blink holds cleanly -- and is
@ responsible for the VAR_POWER_STATE bookkeeping. These run the flicker
@ special (StartPowerOutage / StartPowerRestore), wait for it to finish, then
@ show the alert. The specials toggle FLAG_EMERGENCY_LIGHTING and refresh the
@ overworld palettes, so the red wash persists as the player walks.

EventScript_DoPowerOutage::
	special StartPowerOutage
	waitstate
	msgbox Text_PowerOutageAlert, MSGBOX_DEFAULT
	closemessage
	return

EventScript_DoPowerRestore::
	special StartPowerRestore
	waitstate
	msgbox Text_PowerRestoredAlert, MSGBOX_DEFAULT
	closemessage
	return

Text_PowerOutageAlert:
	.string "WARNING: Primary power failure.\p"
	.string "Switching to emergency lighting.$"

Text_PowerRestoredAlert:
	.string "Primary power restored.$"
