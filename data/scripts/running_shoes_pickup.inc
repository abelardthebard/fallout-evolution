Common_EventScript_RunningShoesPickup::
	lock
	faceplayer
	playfanfare MUS_OBTAIN_ITEM
	removeobject VAR_LAST_TALKED
	delay 10
	message Text_RunningShoesObtained
	waitfanfare
	waitmessage
	setflag FLAG_RECEIVED_RUNNING_SHOES
	setflag FLAG_SYS_B_DASH
	setvar VAR_PROLOGUE_STATE, PROLOGUE_GOT_SHOES
	msgbox Text_RunningShoesExplain, MSGBOX_DEFAULT
	closemessage
	delay 30
	release
	end

Text_RunningShoesObtained:
	.string "{PLAYER} obtained VAULT-TEC SNEAKERS!$"

Text_RunningShoesExplain:
	.string "Why walk when you can RUN?\n"
	.string "Hold B and feel the difference that\l"
	.string "Vault-Tec engineering can make!$"
