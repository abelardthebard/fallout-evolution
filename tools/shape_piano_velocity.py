"""
Vanilla-standard velocity shaping for all tracks in mus_fallout_main_theme.mid.

Per-track targets derived from analysis of 10 representative vanilla Emerald songs:
  prog 2  (piano lead):        80-120, stdev ~20
  prog 3  (pedal piano bed):   65-95,  stdev ~12
  prog 33 (bass):              85-110, stdev ~10
  prog 48 (strings pad):       75-95,  stdev ~8
  prog 46 (harp):              85-110, stdev ~10
  prog 56 (trumpet lead):      100-120, stdev ~8

Shaping rules:
  - Base = (min+max)/2
  - Pitch weighting: higher notes get upper half of range
  - Top-of-chord +boost on polyphonic tracks
  - Phrase starts (>=240 tick rest before) +boost
  - Short notes (<120 ticks) -reduce (filler)
  - Small random jitter for humanization (seed fixed for reproducibility)

Backup: mus_fallout_main_theme.mid.velshape-bak
"""
import mido, shutil, os, random
from collections import defaultdict

SRC = os.path.join(os.path.dirname(__file__), '..', 'sound', 'songs', 'midi', 'mus_fallout_main_theme.mid')
BAK = SRC + '.velshape-bak'

# prog -> (vmin, vmax, jitter, phrase_boost, top_boost, short_penalty, polyphonic)
TARGETS = {
    2:  (80, 120, 6, 10, 10, 8, True),   # piano lead — wide dynamic
    3:  (65, 95,  4, 6,  6,  6, True),   # pedal piano bed — softer
    33: (85, 110, 4, 5,  0,  4, False),  # bass
    48: (75, 95,  3, 0,  0,  0, True),   # pad — minimal variation
    46: (85, 110, 5, 8,  0,  5, False),  # harp — sparkle accents
    56: (100, 120, 3, 5,  0,  3, False), # trumpet lead
}

random.seed(42)

def shape_track(tr, pitch_low, pitch_high):
    prog = next((m.program for m in tr if m.type=='program_change'), None)
    if prog not in TARGETS: return None, prog
    vmin, vmax, jitter, phrase_boost, top_boost, short_penalty, polyphonic = TARGETS[prog]
    base = (vmin + vmax) / 2
    rng = (vmax - vmin) / 2

    # Gather absolute-time notes with durations
    t = 0
    records = []  # (abs_on, dur, note, msg_ref)
    active = {}
    for m in tr:
        t += m.time
        if m.type == 'note_on' and m.velocity > 0:
            active.setdefault(m.note, []).append((t, m))
        elif m.type == 'note_off' or (m.type == 'note_on' and m.velocity == 0):
            if active.get(m.note):
                s, on_msg = active[m.note].pop(0)
                records.append((s, t - s, m.note, on_msg))

    # Group by tick for top-of-chord detection
    by_tick = defaultdict(list)
    for s, d, n, msg in records:
        by_tick[s].append((n, d, msg))

    prev_end = -10000
    for tick in sorted(by_tick):
        group = sorted(by_tick[tick], key=lambda x: x[0])
        top_note = group[-1][0]
        max_end = max(tick + d for _, d, _ in group)
        is_phrase_start = (tick - prev_end) >= 240

        for note, dur, msg in group:
            # Pitch weighting: scale [pitch_low..pitch_high] to [-1..+1]
            if pitch_high > pitch_low:
                pfrac = 2 * (note - pitch_low) / (pitch_high - pitch_low) - 1
                pfrac = max(-1, min(1, pfrac))
            else:
                pfrac = 0
            vel = base + pfrac * rng * 0.6

            if polyphonic and note == top_note and len(group) > 1:
                vel += top_boost
            if is_phrase_start:
                vel += phrase_boost
            if dur < 120:
                vel -= short_penalty
            # Humanize
            vel += random.uniform(-jitter, jitter)

            msg.velocity = max(vmin, min(vmax, int(round(vel))))
        prev_end = max(prev_end, max_end)
    return True, prog

def main():
    if not os.path.exists(BAK):
        shutil.copy(SRC, BAK); print(f'Backup: {BAK}')
    mid = mido.MidiFile(SRC)

    # Compute global pitch range per prog for weighting
    prog_pitch = defaultdict(list)
    for tr in mid.tracks:
        t=0; prog=None
        for m in tr:
            t+=m.time
            if m.type=='program_change': prog=m.program
            elif m.type=='note_on' and m.velocity>0 and prog is not None:
                prog_pitch[prog].append(m.note)

    for i, tr in enumerate(mid.tracks):
        prog = next((m.program for m in tr if m.type=='program_change'), None)
        if prog not in TARGETS: continue
        pl, ph = min(prog_pitch[prog]), max(prog_pitch[prog])
        shape_track(tr, pl, ph)
        vels = [m.velocity for m in tr if m.type=='note_on' and m.velocity>0]
        if vels:
            import statistics
            sd = statistics.stdev(vels) if len(vels) > 1 else 0
            print(f'T{i} prog {prog}: range {min(vels)}-{max(vels)} avg {sum(vels)//len(vels)} stdev {sd:.1f} (target: {TARGETS[prog][0]}-{TARGETS[prog][1]})')

    mid.save(SRC)
    print(f'Wrote: {SRC}')

if __name__ == '__main__':
    main()
