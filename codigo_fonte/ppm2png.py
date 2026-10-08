import struct
import sys
import zlib


def ppm_to_png(ppm_path, png_path):
    with open(ppm_path, "rb") as f:
        data = f.read()

    assert data[:2] == b"P6"
    pos = 2
    vals = []
    while len(vals) < 3:
        while data[pos] in b" \t\r\n":
            pos += 1
        if data[pos:pos + 1] == b"#":
            while data[pos] not in b"\r\n":
                pos += 1
            continue
        start = pos
        while data[pos] not in b" \t\r\n":
            pos += 1
        vals.append(int(data[start:pos]))
    pos += 1
    width, height, maxval = vals
    pixels = data[pos:pos + width * height * 3]

    def chunk(tag, payload):
        return (struct.pack(">I", len(payload)) + tag + payload +
                struct.pack(">I", zlib.crc32(tag + payload) & 0xffffffff))

    raw = bytearray()
    stride = width * 3
    for y in range(height):
        raw.append(0)
        raw.extend(pixels[y * stride:(y + 1) * stride])

    png = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    png += chunk(b"IHDR", ihdr)
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")

    with open(png_path, "wb") as f:
        f.write(png)


if __name__ == "__main__":
    ppm_to_png(sys.argv[1], sys.argv[2])
