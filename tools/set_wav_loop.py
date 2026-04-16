"""
Add or replace the SMPL chunk in a WAV file to set loop points.
Default: loop start = 0, loop end = last sample (whole file loops).

Usage: python tools/set_wav_loop.py <path.wav> [loop_start] [loop_end]
"""
import sys, struct, os

def parse_wav(path):
    with open(path, 'rb') as f: data = f.read()
    if data[:4] != b'RIFF' or data[8:12] != b'WAVE':
        raise ValueError('Not a RIFF/WAVE file')
    chunks = []
    i = 12
    while i < len(data) - 8:
        cid = data[i:i+4]
        csize = struct.unpack('<I', data[i+4:i+8])[0]
        chunks.append((cid, i, csize, data[i+8:i+8+csize]))
        i += 8 + csize + (csize & 1)  # pad byte if odd
    return data, chunks

def build_smpl_chunk(sample_rate, midi_key, loop_start, loop_end):
    sample_period = 1000000000 // sample_rate  # nanoseconds per sample
    smpl_body = struct.pack(
        '<IIIIIIIII',
        0,              # Manufacturer
        0,              # Product
        sample_period,
        midi_key,       # Unity Note (root)
        0,              # Pitch Fraction
        0,              # SMPTE Format
        0,              # SMPTE Offset
        1,              # Num Sample Loops
        0,              # Sampler Data
    )
    loop = struct.pack(
        '<IIIIII',
        0,              # Cue Point ID
        0,              # Type: 0 = forward loop
        loop_start,
        loop_end,
        0,              # Fraction
        0,              # Play Count: 0 = infinite
    )
    body = smpl_body + loop
    return b'smpl' + struct.pack('<I', len(body)) + body

def get_num_samples(chunks, bits_per_sample, num_channels):
    for cid, off, size, body in chunks:
        if cid == b'data':
            bytes_per_sample = (bits_per_sample // 8) * num_channels
            return size // bytes_per_sample
    return 0

def main():
    if len(sys.argv) < 2:
        print(__doc__); sys.exit(1)
    path = sys.argv[1]
    data, chunks = parse_wav(path)

    # Read fmt chunk
    fmt = next((c for c in chunks if c[0] == b'fmt '), None)
    if not fmt: raise ValueError('No fmt chunk')
    _, _, _, fmt_body = fmt
    audio_fmt, num_ch, sample_rate, byte_rate, block_align, bits = struct.unpack('<HHIIHH', fmt_body[:16])
    num_samples = get_num_samples(chunks, bits, num_ch)
    print(f'Sample rate={sample_rate}, bits={bits}, channels={num_ch}, samples={num_samples}')

    loop_start = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    loop_end = int(sys.argv[3]) if len(sys.argv) > 3 else num_samples - 1
    print(f'Setting loop: start={loop_start}, end={loop_end}')

    smpl_chunk = build_smpl_chunk(sample_rate, 60, loop_start, loop_end)

    # Rebuild: keep all chunks except old smpl, insert new smpl before data
    new_body = b''
    for cid, off, size, body in chunks:
        if cid == b'smpl':
            continue  # drop old
        if cid == b'data':
            new_body += smpl_chunk
        chunk_bytes = cid + struct.pack('<I', size) + body
        if size & 1:
            chunk_bytes += b'\x00'  # pad byte
        new_body += chunk_bytes

    new_riff = b'RIFF' + struct.pack('<I', len(new_body) + 4) + b'WAVE' + new_body
    backup = path + '.pre-loop-bak'
    if not os.path.exists(backup):
        with open(backup, 'wb') as f: f.write(data)
        print(f'Backup: {backup}')
    with open(path, 'wb') as f: f.write(new_riff)
    print(f'Wrote {path} with SMPL loop chunk ({len(new_body)+12} bytes total)')

if __name__ == '__main__':
    main()
