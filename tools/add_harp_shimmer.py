"""
Add sparse harp accents on piano melodic peaks, ONLY where current polyphony
allows it without exceeding the 12-voice ceiling.

Strategy:
- Find the top note of each chord in the main piano track (T5, prog 2)
- Filter to "melodic peaks": notes that are higher than both their neighbors OR are
  at the start of a new phrase (preceded by a rest >= 240 ticks)
- For each candidate, check total polyphony at its tick — skip if >= 11
- Emit short harp note one octave above the piano peak (sparkle)

Backup: mus_fallout_main_theme.mid.harp-bak
"""
import mido, shutil, os

SRC = os.path.join(os.path.dirname(__file__), '..', 'sound', 'songs', 'midi', 'mus_fallout_main_theme.mid')
BAK = SRC + '.harp-bak'
HARP_CHANNEL = 7
HARP_PROGRAM = 46  # Orchestral Harp (GM 47 = index 46)
HARP_VELOCITY = 70
HARP_DURATION = 360  # ~eighth-note sparkle
POLY_CEILING = 11
OCTAVE_UP = 12

def main():
    if not os.path.exists(BAK):
        shutil.copy(SRC, BAK); print(f'Backup: {BAK}')
    mid = mido.MidiFile(SRC)

    # Collect all notes with absolute time
    all_notes = []
    piano_notes = []
    for tr in mid.tracks:
        t=0; active={}; prog=None
        for m in tr:
            t+=m.time
            if m.type=='program_change': prog=m.program
            elif m.type=='note_on' and m.velocity>0: active.setdefault(m.note,[]).append(t)
            elif m.type=='note_off' or (m.type=='note_on' and m.velocity==0):
                if active.get(m.note):
                    s=active[m.note].pop(0)
                    all_notes.append((s,t,m.note))
                    if prog == 2: piano_notes.append((s,t,m.note))

    def poly_at(tick):
        return sum(1 for s,e,_ in all_notes if s <= tick < e)

    # Group piano notes by simultaneous attack, pick top of each chord
    from collections import defaultdict
    chords = defaultdict(list)
    for s,e,n in piano_notes: chords[s].append((n,e))
    chord_peaks = sorted([(s, max(chords[s], key=lambda x:x[0])) for s in chords])

    # Find melodic peaks and/or phrase starts
    candidates = []
    prev_peak = None
    prev_end = -10000
    for i,(s,(n,e)) in enumerate(chord_peaks):
        is_phrase_start = (s - prev_end) >= 240
        is_higher_than_prev = prev_peak is None or n > prev_peak
        is_higher_than_next = i+1 >= len(chord_peaks) or n > chord_peaks[i+1][1][0]
        is_local_max = is_higher_than_prev and is_higher_than_next
        if is_phrase_start or is_local_max:
            candidates.append((s, n, e))
        prev_peak = n
        prev_end = max(prev_end, e)

    # Filter by polyphony headroom
    placed = []
    for s, n, e in candidates:
        if poly_at(s) < POLY_CEILING:
            placed.append((s, n + OCTAVE_UP))

    print(f'Harp candidates: {len(candidates)}, placed after poly filter: {len(placed)}')

    # Build harp track
    harp = mido.MidiTrack()
    harp.append(mido.MetaMessage('track_name', name='Harp Shimmer', time=0))
    harp.append(mido.Message('program_change', channel=HARP_CHANNEL, program=HARP_PROGRAM, time=0))
    events = [(s, mido.Message('note_on', channel=HARP_CHANNEL, note=min(108,n), velocity=HARP_VELOCITY, time=0)) for s,n in placed]
    events += [(s+HARP_DURATION, mido.Message('note_off', channel=HARP_CHANNEL, note=min(108,n), velocity=0, time=0)) for s,n in placed]
    events.sort(key=lambda x:(x[0], 0 if x[1].type=='note_off' else 1))
    prev = 0
    for t, m in events:
        harp.append(m.copy(time=max(0, t-prev)))
        prev = t

    mid.tracks.append(harp)
    mid.save(SRC)
    print(f'Wrote: {SRC} ({len(mid.tracks)} tracks)')

if __name__ == '__main__':
    main()
