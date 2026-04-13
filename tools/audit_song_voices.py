#!/usr/bin/env python3
"""
Audit a pokeemerald song .s file for voice-stealing and silent-track risks.

Usage:
    python3 tools/audit_song_voices.py sound/songs/midi/mus_fallout_main_theme.s

Reports:
  - Track count vs m4a BGM pool capacity (NUM_TRACKS_BGM = 10)
  - Voicegroup each track uses and whether every VOICE directive points at
    a populated slot (silent-track detector)
  - Peak simultaneous note count across all tracks, per tick
  - Bars where simultaneous notes exceed the BGM pool (voice-stealing risk)
"""

import os
import re
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
# Two distinct limits in m4a:
#   NUM_TRACKS_BGM (sequencer track slots, one playing note per track)
#   MAX_DIRECTSOUND_CHANNELS (hardware sample mixer voices — the real polyphony ceiling)
# Voice stealing happens at the mixer layer, so polyphony must be compared to MAX_DIRECTSOUND_CHANNELS.
NUM_TRACKS_BGM = 10           # from sound/music_player_table.inc
MAX_DIRECTSOUND_CHANNELS = 12 # from include/gba/m4a_internal.h / constants/m4a_constants.inc

# --- helpers ---------------------------------------------------------------

def load_lines(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.readlines()

def find_voicegroup(song_lines):
    """Return the voicegroup name declared at the top of a song .s file."""
    for line in song_lines:
        m = re.search(r"\.equ\s+\S+_grp,\s*(voicegroup_\w+)", line)
        if m:
            return m.group(1)
    return None

def parse_voicegroup(name):
    """
    Return a dict: slot_index -> (voice_macro, is_populated).
    Empty slots are voices pointing at null samples or voice_keysplit_all to
    an empty sub-table. We use a light heuristic: any voice_* entry counts as
    populated unless it's a literal voice_directsound/square/noise with all
    zero args, or the shared "dummy" placeholder patterns.
    """
    path = os.path.join(REPO_ROOT, "sound", "voicegroups", name.replace("voicegroup_", "") + ".inc")
    if not os.path.exists(path):
        return None, f"voicegroup file not found: {path}"
    slots = {}
    idx = 0
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            s = line.strip()
            if not s or s.startswith("@") or s.startswith(".") or s.endswith("::"):
                continue
            # Skip the "voice_group <name>" header; count only actual voice slot entries.
            if s.startswith("voice_group"):
                continue
            if s.startswith("voice_"):
                macro = s.split()[0]
                # Heuristic: dummy / null entries
                is_populated = True
                if "DummyVoice" in s or ",0,0,0,0,0,0,0" in s.replace(" ", ""):
                    is_populated = False
                slots[idx] = (macro, is_populated)
                idx += 1
    return slots, None

# --- parsing a track -------------------------------------------------------

NOTE_DURATIONS = {
    "N01": 1, "N02": 2, "N03": 3, "N04": 4, "N05": 5, "N06": 6, "N07": 7, "N08": 8,
    "N09": 9, "N10": 10, "N11": 11, "N12": 12, "N13": 13, "N14": 14, "N15": 15,
    "N16": 16, "N17": 17, "N18": 18, "N19": 19, "N20": 20, "N21": 21, "N22": 22,
    "N23": 23, "N24": 24, "N28": 28, "N30": 30, "N32": 32, "N36": 36, "N40": 40,
    "N42": 42, "N44": 44, "N48": 48, "N52": 52, "N54": 54, "N56": 56, "N60": 60,
    "N64": 64, "N66": 66, "N68": 68, "N72": 72, "N76": 76, "N78": 78, "N80": 80,
    "N84": 84, "N88": 88, "N90": 90, "N92": 92, "N96": 96,
}
WAIT_DURATIONS = {
    "W01": 1, "W02": 2, "W03": 3, "W04": 4, "W05": 5, "W06": 6, "W07": 7, "W08": 8,
    "W09": 9, "W10": 10, "W11": 11, "W12": 12, "W13": 13, "W14": 14, "W15": 15,
    "W16": 16, "W17": 17, "W18": 18, "W19": 19, "W20": 20, "W21": 21, "W22": 22,
    "W23": 23, "W24": 24, "W28": 28, "W30": 30, "W32": 32, "W36": 36, "W40": 40,
    "W42": 42, "W44": 44, "W48": 48, "W52": 52, "W54": 54, "W56": 56, "W60": 60,
    "W64": 64, "W66": 66, "W68": 68, "W72": 72, "W76": 76, "W78": 78, "W80": 80,
    "W84": 84, "W88": 88, "W90": 90, "W92": 92, "W96": 96,
}

NOTE_LETTERS = {"Cn": 0, "Cs": 1, "Dn": 2, "Ds": 3, "En": 4, "Fn": 5,
                "Fs": 6, "Gn": 7, "Gs": 8, "An": 9, "As": 10, "Bn": 11}

def parse_note_pitch(tok):
    """Parse a mid2agb note token like 'Ds4' or 'Gn5' into a MIDI-ish value.
    Returns None if not a note. Cn3 = 60 by convention (middle C)."""
    m = re.match(r"^([A-G][sn])(\d+)$", tok)
    if not m:
        return None
    letter, octave = m.group(1), int(m.group(2))
    if letter not in NOTE_LETTERS:
        return None
    # mid2agb uses Cn3 as middle C (MIDI 60)
    return (octave + 2) * 12 + NOTE_LETTERS[letter]

def parse_track(lines):
    """
    Returns: (voices_used, note_events)
      voices_used: set of VOICE directive values seen
      note_events: list of (start_tick, duration, pitch) tuples
    """
    voices = set()
    events = []
    tick = 0
    last_note_dur = 24
    for raw in lines:
        s = raw.strip()
        if not s or s.startswith("@"):
            continue
        # strip .byte prefix
        if s.startswith(".byte"):
            s = s[5:].strip()
        # VOICE directive
        m = re.match(r"VOICE\s*,\s*(\d+)", s)
        if m:
            voices.add(int(m.group(1)))
            continue
        # wait
        tok = s.split(",")[0].strip().split()[0] if s else ""
        if tok in WAIT_DURATIONS:
            tick += WAIT_DURATIONS[tok]
            continue
        # The note pitch is usually the second comma-separated value: "N24, Ds4, v028"
        # or for explicit-duration events. For bare notes, the pitch is the tok itself.
        parts = [p.strip() for p in s.split(",")]
        pitch = None
        if tok in NOTE_DURATIONS:
            last_note_dur = NOTE_DURATIONS[tok]
            if len(parts) >= 2:
                pitch = parse_note_pitch(parts[1].split()[0])
            events.append((tick, last_note_dur, pitch))
            continue
        # bare note (inherits last N duration)
        pitch = parse_note_pitch(tok)
        if pitch is not None:
            events.append((tick, last_note_dur, pitch))
            continue
    return voices, events

def classify_track(events):
    """Infer the musical role of a track from its notes.
    Returns a dict with pitch stats and a suggested role label."""
    if not events:
        return {"role": "EMPTY", "count": 0}
    pitches = [e[2] for e in events if e[2] is not None]
    if not pitches:
        return {"role": "UNKNOWN", "count": len(events)}

    avg = sum(pitches) / len(pitches)
    lo, hi = min(pitches), max(pitches)
    rng = hi - lo

    # Estimate within-track polyphony: count overlaps of this track's own notes
    active_ends = []
    peak_poly = 0
    for (start, dur, _) in sorted(events, key=lambda x: x[0]):
        active_ends = [e for e in active_ends if e > start]
        active_ends.append(start + dur)
        peak_poly = max(peak_poly, len(active_ends))

    # Heuristic classification
    if peak_poly >= 3:
        role = "HARMONY/CHORDS"
    elif peak_poly == 2:
        role = "HARMONY (dyads)"
    else:
        # monophonic — classify by pitch
        if avg < 48:
            role = "BASS"
        elif avg > 66:
            role = "MELODY"
        else:
            role = "INNER VOICE"

    # Wide range + chord-capable = likely piano (both hands)
    if rng > 36 and peak_poly >= 2:
        role += " (piano-like — wide range + chords)"

    return {
        "role": role,
        "count": len(events),
        "avg_pitch": round(avg, 1),
        "min_pitch": lo,
        "max_pitch": hi,
        "range": rng,
        "max_self_polyphony": peak_poly,
    }

def split_tracks(lines):
    """
    Return {track_name: [lines]} for each mus_*_N: label block.
    Excludes pattern subroutines (labels like mus_*_N_NNN — called via PATT,
    not parallel tracks). Pattern-subroutine bodies are inlined into their
    parent track so note counts and polyphony stay accurate.
    """
    # Collect every label block first.
    blocks = {}
    current = None
    for raw in lines:
        m = re.match(r"^(mus_\w+):\s*$", raw)
        if m:
            current = m.group(1)
            blocks[current] = []
            continue
        if current:
            blocks[current].append(raw)

    # A real track ends in _N (single digit group); patterns end in _N_NNN.
    track_re = re.compile(r"^mus_\w+?_\d+$")
    pattern_re = re.compile(r"^(mus_\w+?_\d+)_\d+$")

    real_tracks = {name: list(body) for name, body in blocks.items() if track_re.match(name) and not pattern_re.match(name)}

    # Inline PATT calls: replace "PATT" + ".word <label>" with the body of <label>.
    def inline_patts(body):
        out = []
        i = 0
        while i < len(body):
            line = body[i]
            if "PATT" in line and line.strip().endswith("PATT"):
                # next line is .word <label>
                if i + 1 < len(body):
                    m = re.search(r"\.word\s+(\w+)", body[i + 1])
                    if m and m.group(1) in blocks:
                        sub_body = blocks[m.group(1)]
                        # strip terminating PEND from subroutine
                        sub_body = [l for l in sub_body if "PEND" not in l]
                        out.extend(inline_patts(sub_body))
                        i += 2
                        continue
            out.append(line)
            i += 1
        return out

    for name in real_tracks:
        real_tracks[name] = inline_patts(real_tracks[name])
    return real_tracks

# --- main ------------------------------------------------------------------

def audit(song_path):
    lines = load_lines(song_path)
    song_name = os.path.splitext(os.path.basename(song_path))[0]
    print(f"=== Audit: {song_name} ===\n")

    vg_name = find_voicegroup(lines)
    print(f"Voicegroup: {vg_name}")
    slots, err = parse_voicegroup(vg_name) if vg_name else (None, "no voicegroup declared")
    if err:
        print(f"  ! {err}")
    else:
        populated = sum(1 for v in slots.values() if v[1])
        print(f"  {populated}/{len(slots)} slots populated\n")

    tracks = split_tracks(lines)
    print(f"Tracks in song: {len(tracks)} (BGM pool capacity: {NUM_TRACKS_BGM})")
    if len(tracks) > NUM_TRACKS_BGM:
        print(f"  !! EXCESS: {len(tracks) - NUM_TRACKS_BGM} track(s) will be truncated\n")
    else:
        print()

    all_events = []  # (start, end)
    problems = []
    for name, tlines in tracks.items():
        voices, events = parse_track(tlines)
        voice_report = []
        for v in sorted(voices):
            if slots and v in slots:
                macro, ok = slots[v]
                voice_report.append(f"{v}={'OK' if ok else 'EMPTY'}({macro})")
                if not ok:
                    problems.append(f"{name} uses empty voice slot {v}")
            else:
                voice_report.append(f"{v}=UNKNOWN")
        stats = classify_track(events)
        role_line = f"→ {stats.get('role', '?')}"
        if "avg_pitch" in stats:
            role_line += f" (avg {stats['avg_pitch']}, range {stats['min_pitch']}-{stats['max_pitch']}, self-poly {stats['max_self_polyphony']})"
        print(f"  {name}: voices [{', '.join(voice_report) or 'none'}], {len(events)} notes")
        print(f"    {role_line}")
        for (start, dur, _pitch) in events:
            all_events.append((start, start + dur))

    # Polyphony sweep
    ticks = {}  # tick -> active count
    events_sorted = sorted(all_events)
    max_tick = max((e for _, e in all_events), default=0)
    active = []
    peak = 0
    peak_tick = 0
    i = 0
    for t in range(max_tick + 1):
        active = [end for end in active if end > t]
        while i < len(events_sorted) and events_sorted[i][0] == t:
            active.append(events_sorted[i][1])
            i += 1
        if len(active) > peak:
            peak = len(active)
            peak_tick = t

    print(f"\nPeak simultaneous notes: {peak} at tick {peak_tick} (bar ~{peak_tick // 96 + 1})")
    if peak > MAX_DIRECTSOUND_CHANNELS:
        print(f"  !! Voice stealing likely: {peak - MAX_DIRECTSOUND_CHANNELS} note(s) beyond mixer pool of {MAX_DIRECTSOUND_CHANNELS}")
    else:
        print(f"  OK — within DirectSound mixer pool of {MAX_DIRECTSOUND_CHANNELS}")
    if len(tracks) > NUM_TRACKS_BGM:
        print(f"  !! Track count {len(tracks)} exceeds BGM sequencer slots {NUM_TRACKS_BGM}")
    print(f"\nNote: lower track index = higher priority during voice-stealing; SFX shares the mixer pool (budget ~2-3 voices).")

    if problems:
        print("\nProblems:")
        for p in problems:
            print(f"  - {p}")

    print()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    audit(sys.argv[1])
