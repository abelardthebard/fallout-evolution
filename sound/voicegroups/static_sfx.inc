@ Fallout Evolution — static SFX voicegroup for "Please Stand By" ambience.
@ Slot 0: reserved drumkit (GM ch10).
@ Slot 1: looping TV static sample (uses internal SMPL loop points).
voice_group static_sfx
	voice_keysplit_all voicegroup_rs_drumset                                          @ 0 Drumset (reserved)
	voice_directsound_alt 60, 0, DirectSoundWaveData_static, 255, 0, 255, 0           @ 1 TV Static (looping)
