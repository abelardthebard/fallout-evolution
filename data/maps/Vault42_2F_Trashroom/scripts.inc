Vault42_2F_Trashroom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Vault42_2F_Trashroom_OnTransition
	.byte 0

@ Arms the hallway dilly-dally detour if the player wandered in here
@ during VAR_PROLOGUE_STATE 3 (post-first-catchup, pre-second-catchup).
Vault42_2F_Trashroom_OnTransition::
	goto_if_ne VAR_PROLOGUE_STATE, 3, Vault42_2F_Trashroom_OnTransition_End
	goto_if_eq VAR_TODD_HALLWAY_DETOUR, 9, Vault42_2F_Trashroom_OnTransition_End
	setvar VAR_TODD_HALLWAY_DETOUR, 3
Vault42_2F_Trashroom_OnTransition_End::
	end
