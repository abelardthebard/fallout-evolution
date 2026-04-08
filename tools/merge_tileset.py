#!/usr/bin/env python3
"""
Merge multiple GBA animation frames into one shared tileset + N tilemaps.
Outputs files compatible with pokeemerald's gbagfx/smol pipeline.

Usage: python tools/merge_tileset.py <input_dir> <output_dir>
Example: python tools/merge_tileset.py graphics/title_screen/abelard_anim graphics/title_screen/abelard_anim

Reads: frame00.png through frameNN.png (256x256, indexed, <=16 colors)
Outputs:
  - shared_tileset.png  (vertical strip of unique 8x8 tiles, 4-bit indexed)
  - frame00.bin ... frameNN.bin  (GBA 4bpp text-mode tilemaps, 2 bytes/entry)
  - shared.pal  (JASC palette from first frame)
"""

import sys
import os
import struct
from PIL import Image

TILE_W = 8
TILE_H = 8

def extract_tile(img, tx, ty):
    """Extract an 8x8 tile as a tuple of pixel indices."""
    pixels = []
    for y in range(ty * TILE_H, ty * TILE_H + TILE_H):
        for x in range(tx * TILE_W, tx * TILE_W + TILE_W):
            pixels.append(img.getpixel((x, y)))
    return tuple(pixels)

def flip_tile_h(tile):
    """Horizontally flip an 8x8 tile."""
    flipped = []
    for row in range(TILE_H):
        start = row * TILE_W
        flipped.extend(reversed(tile[start:start + TILE_W]))
    return tuple(flipped)

def flip_tile_v(tile):
    """Vertically flip an 8x8 tile."""
    flipped = []
    for row in range(TILE_H - 1, -1, -1):
        start = row * TILE_W
        flipped.extend(tile[start:start + TILE_W])
    return tuple(flipped)

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <input_dir> <output_dir>")
        sys.exit(1)

    input_dir = sys.argv[1]
    output_dir = sys.argv[2]
    os.makedirs(output_dir, exist_ok=True)

    # Find all frameNN.png files
    frame_files = sorted([f for f in os.listdir(input_dir) if f.startswith("frame") and f.endswith(".png")])
    if not frame_files:
        print(f"No frame*.png files found in {input_dir}")
        sys.exit(1)

    print(f"Found {len(frame_files)} frames")

    # Load first frame to get dimensions and palette
    first = Image.open(os.path.join(input_dir, frame_files[0]))
    img_w, img_h = first.size
    tiles_x = img_w // TILE_W
    tiles_y = img_h // TILE_H
    print(f"Frame size: {img_w}x{img_h} = {tiles_x}x{tiles_y} tiles")

    # Extract palette from first frame
    palette = first.getpalette()

    # Build shared tileset: map tile data -> tile ID
    # Also check hflip/vflip variants
    tile_map = {}  # tile_data -> (tile_id, hflip, vflip)
    unique_tiles = []  # list of tile pixel data

    # Always reserve tile 0 as "empty" (all index 0)
    empty_tile = tuple([0] * TILE_W * TILE_H)
    tile_map[empty_tile] = (0, False, False)
    unique_tiles.append(empty_tile)

    frame_tilemaps = []  # list of lists of (tile_id, hflip, vflip)

    for fname in frame_files:
        img = Image.open(os.path.join(input_dir, fname))
        if img.mode != 'P':
            img = img.convert('P')

        tilemap = []
        for ty in range(tiles_y):
            for tx in range(tiles_x):
                tile = extract_tile(img, tx, ty)

                # Check exact match
                if tile in tile_map:
                    tilemap.append(tile_map[tile])
                    continue

                # Check hflip
                hf = flip_tile_h(tile)
                if hf in tile_map:
                    tid, _, _ = tile_map[hf]
                    tile_map[tile] = (tid, True, False)
                    tilemap.append((tid, True, False))
                    continue

                # Check vflip
                vf = flip_tile_v(tile)
                if vf in tile_map:
                    tid, _, _ = tile_map[vf]
                    tile_map[tile] = (tid, False, True)
                    tilemap.append((tid, False, True))
                    continue

                # Check hflip+vflip
                hvf = flip_tile_h(flip_tile_v(tile))
                if hvf in tile_map:
                    tid, _, _ = tile_map[hvf]
                    tile_map[tile] = (tid, True, True)
                    tilemap.append((tid, True, True))
                    continue

                # New unique tile
                tid = len(unique_tiles)
                if tid >= 1024:
                    print(f"ERROR: Too many unique tiles (>1024) at {fname}")
                    sys.exit(1)
                unique_tiles.append(tile)
                tile_map[tile] = (tid, False, False)
                tilemap.append((tid, False, False))

        frame_tilemaps.append(tilemap)
        print(f"  {fname}: {len(unique_tiles)} unique tiles so far")

    print(f"\nTotal unique tiles: {len(unique_tiles)}")

    # Output shared tileset as indexed PNG (8px wide vertical strip)
    tileset_h = len(unique_tiles) * TILE_H
    tileset_img = Image.new('P', (TILE_W, tileset_h))
    tileset_img.putpalette(palette)

    for i, tile in enumerate(unique_tiles):
        for py in range(TILE_H):
            for px in range(TILE_W):
                tileset_img.putpixel((px, i * TILE_H + py), tile[py * TILE_W + px])

    tileset_path = os.path.join(output_dir, "shared_tileset.png")
    tileset_img.save(tileset_path)
    print(f"Saved tileset: {tileset_path} ({TILE_W}x{tileset_h})")

    # Output JASC palette
    pal_path = os.path.join(output_dir, "shared.pal")
    with open(pal_path, 'w') as f:
        num_colors = min(16, len(palette) // 3)
        f.write("JASC-PAL\n0100\n")
        f.write(f"{num_colors}\n")
        for i in range(num_colors):
            r, g, b = palette[i*3], palette[i*3+1], palette[i*3+2]
            f.write(f"{r} {g} {b}\n")
    print(f"Saved palette: {pal_path} ({num_colors} colors)")

    # Output tilemaps as GBA 4bpp text-mode binary (2 bytes per entry)
    for idx, (fname, tilemap) in enumerate(zip(frame_files, frame_tilemaps)):
        bin_name = fname.replace('.png', '.bin')
        bin_path = os.path.join(output_dir, bin_name)
        with open(bin_path, 'wb') as f:
            for tile_id, hflip, vflip in tilemap:
                # GBA text tilemap: bits 0-9 = tile ID, bit 10 = hflip, bit 11 = vflip, bits 12-15 = palette
                entry = tile_id & 0x3FF
                if hflip:
                    entry |= (1 << 10)
                if vflip:
                    entry |= (1 << 11)
                # palette number 0 (will be set at load time or patched later)
                f.write(struct.pack('<H', entry))
        print(f"  Saved tilemap: {bin_name}")

    print(f"\nDone! {len(unique_tiles)} unique tiles, {len(frame_files)} tilemaps")

if __name__ == '__main__':
    main()
