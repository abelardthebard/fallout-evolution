@ Shared Vault-Tec propaganda poster scripts.
@ Used across all vault maps via bg_sign_event references.
@
@ Each poster has its own EventScript and Text label.
@ To add a new poster:
@   1. Add VaultTec_EventScript_Poster<Name>:: and VaultTec_Text_Poster<Name>: here
@   2. Reference VaultTec_EventScript_Poster<Name> in any map's sign event

VaultTec_EventScript_PosterMedication::
	msgbox VaultTec_Text_PosterMedication, MSGBOX_SIGN
	end

VaultTec_Text_PosterMedication:
	.string "HAVE YOU TAKEN YOUR MEDICATION TODAY?$"


VaultTec_EventScript_PosterAmericaLivesOn::
	msgbox VaultTec_Text_PosterAmericaLivesOn, MSGBOX_SIGN
	end

VaultTec_Text_PosterAmericaLivesOn:
	.string "AMERICA LIVES ON... IN YOU$"


VaultTec_EventScript_PosterCleanliness::
	msgbox VaultTec_Text_PosterCleanliness, MSGBOX_SIGN
	end

VaultTec_Text_PosterCleanliness:
	.string "CLEANLINESS IS GODLINESS$"


VaultTec_EventScript_PosterExtermination::
	msgbox VaultTec_Text_PosterExtermination, MSGBOX_SIGN
	end

VaultTec_Text_PosterExtermination:
	.string "EXTERMINATION IS EVERYONE'S JOB$"


VaultTec_EventScript_PosterSurfaceNever::
	msgbox VaultTec_Text_PosterSurfaceNever, MSGBOX_SIGN
	end

VaultTec_Text_PosterSurfaceNever:
	.string "SURFACE -- NEVER!\n"
	.string "VAULT -- FOREVER!$"


VaultTec_EventScript_PosterOutsideWorld::
	msgbox VaultTec_Text_PosterOutsideWorld, MSGBOX_SIGN
	end

VaultTec_Text_PosterOutsideWorld:
	.string "THE OUTSIDE WORLD\n"
	.string "CAN NEVER HURT YOU$"


VaultTec_EventScript_PosterSafetyFirst::
	msgbox VaultTec_Text_PosterSafetyFirst, MSGBOX_SIGN
	end

VaultTec_Text_PosterSafetyFirst:
	.string "SAFETY FIRST$"


VaultTec_EventScript_PosterHardWork::
	msgbox VaultTec_Text_PosterHardWork, MSGBOX_SIGN
	end

VaultTec_Text_PosterHardWork:
	.string "HARD WORK IS HAPPY WORK$"


VaultTec_EventScript_PosterEatWell::
	msgbox VaultTec_Text_PosterEatWell, MSGBOX_SIGN
	end

VaultTec_Text_PosterEatWell:
	.string "EAT WELL, LIVE WELL$"


VaultTec_EventScript_PosterStayFit::
	msgbox VaultTec_Text_PosterStayFit, MSGBOX_SIGN
	end

VaultTec_Text_PosterStayFit:
	.string "STAY FIT, STAY FOCUSED$"


VaultTec_EventScript_PosterLoyaltyCorps::
	msgbox VaultTec_Text_PosterLoyaltyCorps, MSGBOX_SIGN
	end

VaultTec_Text_PosterLoyaltyCorps:
	.string "THE OVERSEER NEEDS YOU!\n"
	.string "JOIN THE LOYALTY CORPS TODAY$"


VaultTec_EventScript_PosterInThisTogether::
	msgbox VaultTec_Text_PosterInThisTogether, MSGBOX_SIGN
	end

VaultTec_Text_PosterInThisTogether:
	.string "WE'RE ALL IN THIS TOGETHER!\n"
	.string "WE MUST WIN!$"
