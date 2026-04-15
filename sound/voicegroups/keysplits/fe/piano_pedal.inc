@ Fallout Evolution — Pedal-down piano keysplit.
@ Same samples as fe_piano_keysplit but with maxed release (255) for pedal-sustain bleed.
@ Decay values match vanilla piano_keysplit for consistent natural decay.
@ Root=60 per Kurausukun guide.
voice_group fe_piano_pedal_keysplit
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_piano1_48, 255, 252, 0, 255
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_piano1_60, 255, 250, 0, 255
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_piano1_72, 255, 250, 0, 255
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_piano1_84, 255, 247, 0, 255
