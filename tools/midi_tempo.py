#!/usr/bin/env python3
"""Extract tempo (BPM) events from a MIDI file."""
import sys, struct

def read_vlq(data, pos):
    """Read variable-length quantity from MIDI data."""
    value = 0
    while True:
        b = data[pos]
        pos += 1
        value = (value << 7) | (b & 0x7F)
        if (b & 0x80) == 0:
            return value, pos

def parse_midi(path):
    with open(path, 'rb') as f:
        data = f.read()
    if data[:4] != b'MThd':
        print("Not a MIDI file")
        return
    _, fmt, ntrks, division = struct.unpack('>IHHH', data[4:14])
    print(f"Format: {fmt}, Tracks: {ntrks}, Division: {division} ticks/quarter")

    pos = 14
    tempos = []
    for trk in range(ntrks):
        if data[pos:pos+4] != b'MTrk':
            print(f"Track {trk}: missing MTrk marker")
            return
        trk_len = struct.unpack('>I', data[pos+4:pos+8])[0]
        pos += 8
        end = pos + trk_len
        running_status = 0
        while pos < end:
            delta, pos = read_vlq(data, pos)
            status = data[pos]
            if status == 0xFF:  # meta event
                pos += 1
                meta_type = data[pos]
                pos += 1
                length, pos = read_vlq(data, pos)
                if meta_type == 0x51 and length == 3:  # tempo
                    mpq = (data[pos] << 16) | (data[pos+1] << 8) | data[pos+2]
                    bpm = 60_000_000 / mpq
                    tempos.append((trk, bpm))
                    print(f"  Track {trk}: TEMPO event — {bpm:.2f} BPM ({mpq} microsec/quarter)")
                pos += length
            elif status >= 0xF0:  # sysex
                pos += 1
                length, pos = read_vlq(data, pos)
                pos += length
            else:  # channel message
                if status < 0x80:
                    status = running_status
                else:
                    running_status = status
                    pos += 1
                event_type = status & 0xF0
                if event_type in (0xC0, 0xD0):
                    pos += 1
                else:
                    pos += 2
    if not tempos:
        print("\nNO TEMPO EVENTS FOUND — song uses default 120 BPM")
    else:
        print(f"\nFound {len(tempos)} tempo event(s)")

if __name__ == '__main__':
    parse_midi(sys.argv[1] if len(sys.argv) > 1 else 'sound/songs/midi/mus_fallout_main_theme.mid')
