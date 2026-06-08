@ Local IDs (array order in map.json, 1-indexed):
@   1 = Overseer Clay
@   2 = Todd (guest -- only present for the Overseer scene)
.set LOCALID_OVERSEER, 1
.set LOCALID_OFFICE_TODD, 2

Vault42_OverseersOffice_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Vault42_OverseersOffice_OnTransition
	.byte 0

@ Todd doesn't live here -- he's only staged in for the Overseer scene, which
@ plays while VAR_PROLOGUE_STATE == PROLOGUE_SENT_TO_OVERSEER (the only state at
@ which the player can be in the office: appointment confirmed, scene not yet
@ done). Hidden in every other state. The scene itself removes him at the end.
Vault42_OverseersOffice_OnTransition::
	call_if_eq VAR_PROLOGUE_STATE, PROLOGUE_SENT_TO_OVERSEER, Vault42_OverseersOffice_EventScript_ShowTodd
	call_if_ne VAR_PROLOGUE_STATE, PROLOGUE_SENT_TO_OVERSEER, Vault42_OverseersOffice_EventScript_HideTodd
	end

Vault42_OverseersOffice_EventScript_ShowTodd::
	clearflag FLAG_HIDE_OFFICE_TODD
	return

Vault42_OverseersOffice_EventScript_HideTodd::
	setflag FLAG_HIDE_OFFICE_TODD
	return

@ Placeholder so the map builds and Clay is interactive. The real Overseer scene
@ (meet Clay -> starter -> advance VAR_PROLOGUE_STATE to PROLOGUE_MET_OVERSEER and
@ re-lock the door via VAR_OVERSEER_DOOR) goes here.
Vault42_OverseersOffice_EventScript_Overseer::
	lock
	faceplayer
	msgbox Vault42_OverseersOffice_Text_OverseerPlaceholder, MSGBOX_DEFAULT
	release
	end

@ Fallback -- the scene locks the player, so this rarely fires.
Vault42_OverseersOffice_EventScript_OfficeTodd::
	end

Vault42_OverseersOffice_Text_OverseerPlaceholder:
	.string "(Overseer Clay placeholder.)$"
