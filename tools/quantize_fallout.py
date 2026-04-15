"""
Quantize all note starts to the 16th-note grid and fix same-pitch overlaps
(note_on before the previous note_off on the same pitch).

Backup: mus_fallout_main_theme.mid.quantize-bak
"""
import mido, shutil, os

SRC = os.path.join(os.path.dirname(__file__), '..', 'sound', 'songs', 'midi', 'mus_fallout_main_theme.mid')
BAK = SRC + '.quantize-bak'

def main():
    if not os.path.exists(BAK):
        shutil.copy(SRC, BAK); print(f'Backup: {BAK}')

    mid = mido.MidiFile(SRC)
    tpb = mid.ticks_per_beat
    grid = tpb // 4  # 16th note

    for tid, tr in enumerate(mid.tracks):
        # Convert to absolute time
        t = 0
        abs_msgs = []
        for m in tr:
            t += m.time
            abs_msgs.append([t, m])

        # Quantize note_on/note_off to nearest 16th
        for entry in abs_msgs:
            t, m = entry
            if m.type in ('note_on', 'note_off'):
                entry[0] = round(t / grid) * grid

        # Fix same-pitch overlaps: for each (channel, note), if a new note_on
        # comes before the prior note_off, shift the offending note_off earlier.
        active = {}  # (ch,note) -> index of note_on entry
        for i, (t, m) in enumerate(abs_msgs):
            if m.type == 'note_on' and m.velocity > 0:
                key = (m.channel, m.note)
                if key in active:
                    # find the matching note_off between active[key] and i
                    prev_on_i = active[key]
                    for j in range(prev_on_i+1, i):
                        mj = abs_msgs[j][1]
                        if (mj.type=='note_off' or (mj.type=='note_on' and mj.velocity==0)) \
                           and mj.channel==m.channel and mj.note==m.note:
                            # Move the note_off to just before current note_on
                            abs_msgs[j][0] = max(abs_msgs[prev_on_i][0], t - 1)
                            break
                active[key] = i
            elif m.type == 'note_off' or (m.type=='note_on' and m.velocity==0):
                key = (m.channel, m.note)
                if key in active and active[key] < i:
                    del active[key]

        # Re-sort (quantization may have reordered things) and rebuild with delta times
        abs_msgs.sort(key=lambda x: (x[0], 0 if x[1].type=='note_off' or (x[1].type=='note_on' and x[1].velocity==0) else 1))
        new_track = mido.MidiTrack()
        prev_t = 0
        for t, m in abs_msgs:
            new_track.append(m.copy(time=max(0, t - prev_t)))
            prev_t = t
        mid.tracks[tid] = new_track

    mid.save(SRC)
    print(f'Wrote: {SRC} (quantized to 16ths, overlaps fixed)')

if __name__ == '__main__':
    main()
