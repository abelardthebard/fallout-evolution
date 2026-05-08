@ Local IDs (array order in map.json, 1-indexed):
@   1 = Mrs. Bauer
@   2 = Todd
.set LOCALID_CLASSROOM_MRSBAUER, 1
.set LOCALID_CLASSROOM_TODD, 2

Vault42_Classroom_MapScripts::
	.byte 0

@ Fallbacks — no-op for now. Will branch on VAR_PROLOGUE_STATE later for varied dialogue.
Vault42_Classroom_EventScript_MrsBauerIdle::
	end

Vault42_Classroom_EventScript_ToddIdle::
	end
