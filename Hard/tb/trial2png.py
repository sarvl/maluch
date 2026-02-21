from PIL import Image

INPUT_FILE = "build/trial"
OUTPUT_FILE = "build/trial.png"
WIDTH = 640
HEIGHT = 480


def rgb332_to_rgb888(v: int) -> tuple[int, int, int]:
    r3 = (v >> 5) & 0x7
    g3 = (v >> 2) & 0x7
    b2 = v & 0x3
    r = (r3 * 255) // 7
    g = (g3 * 255) // 7
    b = (b2 * 255) // 3
    return r, g, b


def read_pixels(path: str) -> list[int]:
    pixels: list[int] = []
    with open(path, "r") as f:
        for line in f:
            s = line.strip()
            if len(s) == 8 and all(c in "01" for c in s):
                pixels.append(int(s, 2) & 0xFF)
    return pixels


def main():
    pixels = read_pixels(INPUT_FILE)
    need = WIDTH * HEIGHT
    if len(pixels) < need:
        pixels += [0x00] * (need - len(pixels))
    elif len(pixels) > need:
        pixels = pixels[:need]

    img = Image.new("RGB", (WIDTH, HEIGHT), color="black")
    px = img.load()
    i = 0
    for y in range(HEIGHT):
        for x in range(WIDTH):
            px[x, y] = rgb332_to_rgb888(pixels[i])
            i += 1
    img.save(OUTPUT_FILE)


if __name__ == "__main__":
    main()