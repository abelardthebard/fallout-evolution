"""
Add light timpani punctuation at structural downbeats:
  - Bar 4: bass entry (D)
  - Bar 14: trumpet entry (Ab)
  - Bar 36: outro start (F#)

Each hit matches the bass pitch class at that downbeat so it cannot clash,
and reinforces the harmonic foundation.

Uses vanilla sc88pro_timpani via fe_cinematic slot 47.
"""
import mido, shutil, os

SRC = os.path.join(os.path.dirname(__file__), '..', 'sound', 'songs', 'midi', 'mus_fallout_main_theme.mid')
BAK = SRC + '.timpani-bak'

CHANNEL = 8
PROGRAM = 47  # Timpani slot in fe_cinematic
VELOCITY = 70
DURATION = 3840  # 2 bars — let the tail ring

# (tick, pitch) — pitch is octave-above-bass at that downbeat
HITS = [
    (7680, 50),   # bar 4, D3
    (26880, 44),  # bar 14, Ab2
    (69120, 54),  # bar 36, F#3
]

def main():
    if not os.path.exists(BAK):
        shutil.copy(SRC, BAK); print(f'Backup: {BAK}')
    mid = mido.MidiFile(SRC)

    # Remove any existing timpani track
    mid.tracks = [tr for tr in mid.tracks
                  if not any(m.type=='program_change' and m.program==PROGRAM
                             and m.channel==CHANNEL for m in tr)]

    timp = mido.MidiTrack()
    timp.append(mido.MetaMessage('track_name', name='Timpani', time=0))
    timp.append(mido.Message('program_change', channel=CHANNEL, program=PROGRAM, time=0))

    events = []
    for tick, pitch in HITS:
        events.append((tick, mido.Message('note_on', channel=CHANNEL, note=pitch, velocity=VELOCITY, time=0)))
        events.append((tick + DURATION, mido.Message('note_off', channel=CHANNEL, note=pitch, velocity=0, time=0)))
    events.sort(key=lambda x:(x[0], 0 if (x[1].type=='note_off' or (x[1].type=='note_on' and x[1].velocity==0)) else 1))

    prev = 0
    for t, m in events:
        timp.append(m.copy(time=max(0, t-prev)))
        prev = t

    # Add as last track (lowest priority — first stolen if polyphony overflows)
    mid.tracks.append(timp)
    mid.save(SRC)
    print(f'Added {len(HITS)} timpani hits')

if __name__ == '__main__':
    main()
