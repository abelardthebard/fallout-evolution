@ Fallout Evolution — Cinematic voicegroup
@ Built for Inon Zur-inspired orchestral scoring (main theme, cutscenes, intro).
@ Uses only vanilla pokeemerald samples; approximations noted inline for
@ instruments not available in the vanilla sample set.
@ Slot numbers follow General MIDI program numbering (0-indexed).

voice_group fe_cinematic
	voice_keysplit_all voicegroup_rs_drumset                                                    @ 0 Drumset (reserved — GM ch10)
	voice_keysplit voicegroup_fe_piano_keysplit, keysplit_fe_piano                                    @ 1 Acoustic Grand Piano
	voice_keysplit voicegroup_fe_piano_keysplit, keysplit_fe_piano                                    @ 2 Bright Acoustic Piano
	voice_keysplit voicegroup_fe_piano_pedal_keysplit, keysplit_fe_piano_pedal                        @ 3 Electric Grand Piano (pedal-down ADSR)
	voice_keysplit voicegroup_fe_piano_keysplit, keysplit_fe_piano                                    @ 4 Honky-tonk Piano
	voice_directsound 60, 0, DirectSoundWaveData_steinway_b_piano, 128, 204, 51, 242            @ 5 Electric Piano 1 (Steinway — warmer)
	voice_directsound 60, 0, DirectSoundWaveData_steinway_b_piano, 128, 204, 51, 242            @ 6 Electric Piano 2 (Steinway)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 7 Harpsichord
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 8 Clavi
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 9 Celesta
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 10 Glockenspiel
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 11 Music Box
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 12 Vibraphone
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 13 Marimba
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_tubular_bell, 255, 165, 90, 216        @ 14 Tubular Bells
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 15 Dulcimer
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 16 Drawbar Organ
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 17 Percussive Organ
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 18 Rock Organ
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 19 Church Organ
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 20 Reed Organ
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 21 Accordion
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 22 Harmonica (NO vanilla sample — square placeholder)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 23 Tango Accordion
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_nylon_str_guitar, 128, 204, 103, 226   @ 24 Acoustic Guitar (nylon)
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_nylon_str_guitar, 128, 204, 103, 226   @ 25 Acoustic Guitar (steel — nylon approx)
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_overdrive_guitar, 128, 0, 255, 214 @ 26 Electric Guitar (jazz — overdrive approx)
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_overdrive_guitar, 128, 0, 255, 214 @ 27 Electric Guitar (clean)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 28 Electric Guitar (muted)
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_overdrive_guitar, 128, 0, 255, 214 @ 29 Overdriven Guitar
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_overdrive_guitar, 128, 0, 255, 214 @ 30 Distortion Guitar
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 31 Guitar Harmonics
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fretless_bass, 255, 253, 0, 149        @ 32 Acoustic Bass (fretless approx)
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fingered_bass, 255, 253, 0, 149        @ 33 Electric Bass (finger) — plucked tone
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fretless_bass, 255, 253, 0, 149        @ 34 Electric Bass (pick)
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fretless_bass, 255, 253, 0, 149        @ 35 Fretless Bass
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 36 Slap Bass 1
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 37 Slap Bass 2
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 38 Synth Bass 1
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 39 Synth Bass 2
	voice_keysplit voicegroup_fe_violin_keysplit, keysplit_fe_violin                            @ 40 Violin (solo violin samples)
	voice_keysplit voicegroup_fe_strings_keysplit, keysplit_fe_strings                          @ 41 Viola (extended range)
	voice_keysplit voicegroup_fe_cello_keysplit, keysplit_fe_cello                              @ 42 Cello (solo cello samples)
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fretless_bass, 255, 253, 0, 149        @ 43 Contrabass (fretless approx)
	voice_keysplit voicegroup_fe_strings_keysplit, keysplit_fe_strings                          @ 44 Tremolo Strings
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_pizzicato_strings, 128, 226, 0, 38     @ 45 Pizzicato Strings
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_harp, 255, 242, 51, 242                @ 46 Orchestral Harp
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_timpani, 255, 0, 180, 239              @ 47 Timpani
	voice_keysplit voicegroup_fe_strings_keysplit, keysplit_fe_strings                          @ 48 String Ensemble 1
	voice_keysplit voicegroup_fe_strings_keysplit, keysplit_fe_strings                          @ 49 String Ensemble 2
	voice_keysplit voicegroup_fe_strings_keysplit, keysplit_fe_strings                          @ 50 Synth Strings 1
	voice_keysplit voicegroup_fe_strings_keysplit, keysplit_fe_strings                          @ 51 Synth Strings 2
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 128, 165, 128, 188 @ 52 Choir Aahs
	voice_directsound 60, 0, DirectSoundWaveData_unknown_female_voice, 128, 165, 128, 204       @ 53 Voice Oohs (female)
	voice_directsound 60, 0, DirectSoundWaveData_unknown_female_voice, 128, 165, 128, 204       @ 54 Synth Voice
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 55 Orchestra Hit
	voice_keysplit voicegroup_trumpet_keysplit, keysplit_trumpet                                @ 56 Trumpet
	voice_keysplit voicegroup_french_horn_keysplit, keysplit_french_horn                        @ 57 Trombone (french horn approx)
	voice_keysplit voicegroup_tuba_keysplit, keysplit_tuba                                      @ 58 Tuba
	voice_keysplit voicegroup_trumpet_keysplit, keysplit_trumpet                                @ 59 Muted Trumpet (trumpet approx)
	voice_keysplit voicegroup_french_horn_keysplit, keysplit_french_horn                        @ 60 French Horn
	voice_keysplit voicegroup_french_horn_keysplit, keysplit_french_horn                        @ 61 Brass Section (FH approx)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 62 Synth Brass 1
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 63 Synth Brass 2
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 64 Soprano Sax
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 65 Alto Sax
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 66 Tenor Sax
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 67 Baritone Sax
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_oboe, 255, 165, 154, 127        @ 68 Oboe
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_oboe, 255, 165, 154, 127        @ 69 English Horn (oboe approx)
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fretless_bass, 255, 253, 0, 149        @ 70 Bassoon (fretless approx — rough)
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_oboe, 255, 165, 154, 127        @ 71 Clarinet (oboe approx)
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_flute, 255, 0, 255, 165                @ 72 Piccolo (flute approx)
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_flute, 255, 0, 255, 165                @ 73 Flute
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_flute, 255, 0, 255, 165                @ 74 Recorder (flute approx)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 75 Pan Flute
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 76 Blown Bottle
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 77 Shakuhachi
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_whistle, 255, 0, 255, 127       @ 78 Whistle
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 79 Ocarina
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 80 Lead 1 (square)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 81 Lead 2 (sawtooth)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 82 Lead 3 (calliope)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 83 Lead 4 (chiff)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 84 Lead 5 (charang)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 85 Lead 6 (voice)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 86 Lead 7 (fifths)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 87 Lead 8 (bass + lead)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 88 Pad 1 (new age)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 89 Pad 2 (warm)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 90 Pad 3 (polysynth)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 91 Pad 4 (choir)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 92 Pad 5 (bowed)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 93 Pad 6 (metallic)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 94 Pad 7 (halo)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 95 Pad 8 (sweep)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 96 FX 1 (rain)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 97 FX 2 (soundtrack)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 98 FX 3 (crystal)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 99 FX 4 (atmosphere)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 100 FX 5 (brightness)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 101 FX 6 (goblins)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 102 FX 7 (echoes)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 103 FX 8 (sci-fi)
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 104 Sitar
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 105 Banjo
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 106 Shamisen
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 107 Koto
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 108 Kalimba
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 109 Bagpipe
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 110 Fiddle
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 111 Shanai
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 112 Tinkle Bell
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 113 Agogo
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 114 Steel Drums
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 115 Woodblock
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 116 Taiko Drum
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 117 Melodic Tom
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 118 Synth Drum
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 119 Reverse Cymbal
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 120 Guitar Fret Noise
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 121 Breath Noise
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 122 Seashore
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 123 Bird Tweet
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 124 Telephone Ring
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 125 Helicopter
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 126 Applause
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0                                                     @ 127 Gunshot
