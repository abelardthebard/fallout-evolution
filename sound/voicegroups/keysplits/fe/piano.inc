@ Fallout Evolution — Piano keysplit using vanilla sc88pro_piano1 samples.
@ ADSR matches vanilla pokeemerald piano_keysplit exactly for correct sustain/release behavior.
@ All samples at engine rate 13379 Hz (no aliasing). Root=60 per Kurausukun guide.
voice_group fe_piano_keysplit
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_piano1_48, 255, 252, 0, 239
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_piano1_60, 255, 250, 0, 221
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_piano1_72, 255, 250, 0, 221
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_piano1_84, 255, 247, 0, 221
