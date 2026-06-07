Vault42_DiningHall_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Vault42_DiningHall_OnTransition
	.byte 0

@ Arms the hallway dilly-dally detour if the player wandered in here
@ during VAR_PROLOGUE_STATE 3 (post-first-catchup, pre-second-catchup).
Vault42_DiningHall_OnTransition::
	goto_if_ne VAR_PROLOGUE_STATE, PROLOGUE_CATCHUP_1, Vault42_DiningHall_OnTransition_End
	goto_if_eq VAR_TODD_HALLWAY_DETOUR, 9, Vault42_DiningHall_OnTransition_End
	setvar VAR_TODD_HALLWAY_DETOUR, 4
Vault42_DiningHall_OnTransition_End::
	end

Vault42_DiningHall_EventScript_ChefIdle::
	end
