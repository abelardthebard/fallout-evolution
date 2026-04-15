"""
Structural cleanup: split Track 4 (pedal piano prog 3) low-register notes into
a new dedicated bass track (Fingered Bass prog 33).

Rule: any note with pitch < C3 (MIDI 48) moves from T4 to the new bass track.
Higher notes stay on T4 as left-hand piano chord texture.

Backup: mus_fallout_main_theme.mid.cleanup-bak
"""
import mido, shutil, os

SRC = os.path.join(os.path.dirname(__file__), '..', 'sound', 'songs', 'midi', 'mus_fallout_main_theme.mid')
BAK = SRC + '.cleanup-bak'
THRESHOLD = 48  # C3
BASS_PROG = 33  # Fingered Bass

def main():
    if not os.path.exists(BAK):
        shutil.copy(SRC, BAK); print(f'Backup: {BAK}')

    mid = mido.MidiFile(SRC)
    tpb = mid.ticks_per_beat

    # Find T4: the pedal piano track (program 3)
    target_idx = None
    for i, tr in enumerate(mid.tracks):
        for m in tr:
            if m.type == 'program_change' and m.program == 3:
                target_idx = i; break
        if target_idx is not None: break
    if target_idx is None:
        print('ERROR: no track with program=3 found'); return
    print(f'Source track: {target_idx} (pedal piano)')

    src = mid.tracks[target_idx]

    # Convert delta-time to absolute; then walk messages deciding which go to bass vs stay
    abs_events = []
    t = 0
    for m in src:
        t += m.time
        abs_events.append((t, m))

    bass_events = []
    keep_events = []
    moved = 0; kept = 0

    # note_on/note_off come in pairs. Track which channel a note is sent on.
    # Rule: if note_on pitch < THRESHOLD → bass track; matching note_off follows.
    note_destination = {}  # (channel, note) -> 'bass' or 'keep' (FIFO if repeats)
    note_dest_queue = {}

    for t, m in abs_events:
        if m.type == 'note_on' and m.velocity > 0:
            dest = 'bass' if m.note < THRESHOLD else 'keep'
            note_dest_queue.setdefault((m.channel, m.note), []).append(dest)
            if dest == 'bass':
                bass_events.append((t, m)); moved += 1
            else:
                keep_events.append((t, m)); kept += 1
        elif m.type == 'note_off' or (m.type == 'note_on' and m.velocity == 0):
            q = note_dest_queue.get((m.channel, m.note), [])
            dest = q.pop(0) if q else ('bass' if m.note < THRESHOLD else 'keep')
            if dest == 'bass': bass_events.append((t, m))
            else: keep_events.append((t, m))
        else:
            # meta/control events — keep on original track
            keep_events.append((t, m))

    print(f'Moved {moved} notes to bass, kept {kept} on pedal piano')

    def to_track(events, program=None):
        events.sort(key=lambda x: x[0])
        tr = mido.MidiTrack()
        if program is not None:
            tr.append(mido.Message('program_change', program=program, channel=0, time=0))
        prev_t = 0
        for t, m in events:
            if m.type == 'program_change' and program is not None:
                continue  # we already wrote our own
            nm = m.copy(time=t - prev_t)
            tr.append(nm)
            prev_t = t
        return tr

    new_piano = to_track(keep_events)
    bass_track = to_track(bass_events, program=BASS_PROG)

    mid.tracks[target_idx] = new_piano
    mid.tracks.append(bass_track)

    mid.save(SRC)
    print(f'Wrote: {SRC} ({len(mid.tracks)} tracks, new bass at index {len(mid.tracks)-1})')

if __name__ == '__main__':
    main()
