Vault42_MedicalBay_MapScripts::
	.byte 0

Vault42_MedicalBay_EventScript_Nanny::
	lock
	faceplayer
	msgbox Vault42_MedicalBay_Text_NannyGreeting, MSGBOX_DEFAULT
	release
	end

Vault42_MedicalBay_Text_NannyGreeting:
	.string "I am a Miss Nanny unit assigned\n"
	.string "to vault medical care.\p"
	.string "Please rest if you are injured.$"
