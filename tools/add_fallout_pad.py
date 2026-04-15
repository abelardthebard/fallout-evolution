"""
Pedal-tone pad: a single sustained note held throughout each pad section.

Classical pedal-point technique. Cannot violate voice-leading rules:
  - Only ONE voice -> no parallel 5ths/8ves possible
  - No motion -> no voice leaps
  - Note chosen from most-weighted pitch class in the section -> guaranteed consonant
    (if a PC is played a lot by piano+bass, holding that PC can't clash with itself)

Used by Hans Zimmer, Inon Zur, Mark Morgan, etc. Standard cinematic technique.

Backup: mus_fallout_main_theme.mid.pad-bak
"""
import mido, shutil, os, collections

SRC = os.path.join(os.path.dirname(__file__), '..', 'sound', 'songs', 'midi', 'mus_fallout_main_theme.mid')
BAK = SRC + '.pad-bak'

PAD_CHANNEL = 3
PAD_PROGRAM = 48
PAD_VELOCITY = 35
POLY_CEILING = 12
PEDAL_MIDI_RANGE = (36, 47)  # C2 to B2 — low register, masks noise floor

PAD_SEGMENTS = [
    (2, 14, None),    # intro: stable pedal throughout
    (36, 999, None),  # outro: stable pedal throughout
]
PAD_SECTIONS = [(s, e) for s, e, _ in PAD_SEGMENTS]

NAMES = ['C','C#','D','D#','E','F','F#','G','G#','A','A#','B']

def section_pc_weights(notes, sec_start, sec_end):
    """Duration-weighted pitch class histogram over a tick range."""
    w = collections.defaultdict(float)
    for s, e, n, mult in notes:
        a, b = max(s, sec_start), min(e, sec_end)
        if b > a: w[n % 12] += (b - a) * mult
    return w

def peak_polyphony_in_range(all_notes, s0, s1):
    events = []
    for s, e, _ in all_notes:
        if e <= s0 or s >= s1: continue
        events.append((max(s, s0), 1))
        events.append((min(e, s1), -1))
    events.sort()
    cur = peak = 0
    for _, d in events:
        cur += d
        if cur > peak: peak = cur
    return peak

def pick_pedal_pc(w):
    """Most-weighted PC that's not a half-step neighbor-dominant."""
    # Filter: prefer PCs that don't have strong half-step neighbors
    candidates = []
    for pc, wt in w.items():
        neighbor = w.get((pc-1)%12, 0) + w.get((pc+1)%12, 0)
        candidates.append((pc, wt, neighbor))
    # Score: high weight, low neighbor conflict
    candidates.sort(key=lambda x: (-(x[1] - 0.5 * x[2])))
    return candidates[0][0] if candidates else 0

def main():
    if not os.path.exists(BAK):
        shutil.copy(SRC, BAK); print(f'Backup: {BAK}')
    mid = mido.MidiFile(SRC)
    TPB = mid.ticks_per_beat
    BAR = TPB * 4

    mid.tracks = [tr for tr in mid.tracks
                  if not any(m.type=='program_change' and m.program==PAD_PROGRAM
                             and m.channel==PAD_CHANNEL for m in tr)]

    all_notes = []
    harm_notes = []
    song_end = 0
    for tr in mid.tracks:
        t=0; active={}; prog=None
        for m in tr:
            t+=m.time
            if m.type=='program_change': prog=m.program
            elif m.type=='note_on' and m.velocity>0: active.setdefault(m.note,[]).append(t)
            elif m.type=='note_off' or (m.type=='note_on' and m.velocity==0):
                if active.get(m.note):
                    s=active[m.note].pop(0)
                    all_notes.append((s, t, m.note))
                    if prog in (2, 3): harm_notes.append((s, t, m.note, 1.0))
                    elif prog == 33: harm_notes.append((s, t, m.note, 3.0))
                    song_end = max(song_end, t)

    n_bars = (song_end + BAR - 1) // BAR
    events_out = [
        (0, mido.MetaMessage('track_name', name='Strings Pad', time=0)),
        (0, mido.Message('program_change', channel=PAD_CHANNEL, program=PAD_PROGRAM, time=0)),
    ]

    # For each PAD_SEGMENT, pick a pedal pitch (auto-detect or inherit octave-up)
    prev_segment_pc = None
    for lo, hi, mode in PAD_SEGMENTS:
        sec_start = lo * BAR
        sec_end = min(hi, n_bars) * BAR
        if sec_start >= song_end: continue

        if mode == 'A3':
            pedal_midi = 57; pedal_pc = 9
        elif mode == 'C3':
            pedal_midi = 48; pedal_pc = 0
        elif mode == 'octave_up' and prev_segment_pc is not None:
            pedal_pc = prev_segment_pc
            pedal_midi = pedal_pc + 60
            if pedal_midi > 71: pedal_midi -= 12
        else:
            w = section_pc_weights(harm_notes, sec_start, sec_end)
            if not w: continue
            pedal_pc = pick_pedal_pc(w)
            for candidate in range(PEDAL_MIDI_RANGE[0], PEDAL_MIDI_RANGE[1]+1):
                if candidate % 12 == pedal_pc:
                    pedal_midi = candidate; break
            else:
                pedal_midi = 48 + pedal_pc
            prev_segment_pc = pedal_pc
        # Split long notes into chunks to keep the engine's envelope fresh
        # (prevents gradual volume decay on very long sustains)
        CHUNK = BAR * 2  # re-attack every 2 bars
        t = sec_start
        chunks_emitted = 0
        while t < sec_end:
            # Skip chunk if polyphony would exceed cap
            peak = peak_polyphony_in_range(all_notes, t, min(t+CHUNK, sec_end))
            if peak >= POLY_CEILING:
                t += CHUNK
                continue
            chunk_end = min(t + CHUNK, sec_end)
            events_out.append((t, mido.Message('note_on', channel=PAD_CHANNEL, note=pedal_midi, velocity=PAD_VELOCITY, time=0)))
            events_out.append((chunk_end, mido.Message('note_off', channel=PAD_CHANNEL, note=pedal_midi, velocity=0, time=0)))
            chunks_emitted += 1
            t = chunk_end
        print(f'Section bars {lo}-{hi if hi<999 else n_bars-1}: pedal tone {NAMES[pedal_pc]} (MIDI {pedal_midi}), {chunks_emitted} chunks')

    events_out.sort(key=lambda x:(x[0], 0 if (x[1].type=='note_off' or (x[1].type=='note_on' and x[1].velocity==0)) else 1))

    pad = mido.MidiTrack()
    prev = 0
    for t, m in events_out:
        pad.append(m.copy(time=max(0, t-prev)))
        prev = t
    mid.tracks.insert(1, pad)
    mid.save(SRC)
    print(f'\nWrote: {SRC} ({len(mid.tracks)} tracks)')

if __name__ == '__main__':
    main()
