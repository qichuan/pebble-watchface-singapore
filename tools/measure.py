#!/usr/bin/env python3
"""Reports the face's real proportions off a screenshot.

The design is a table of ratios -- band depth, and the position of each row in
the white field -- and the eye cannot judge a three-pixel drift in any of them.
Worse, the rows are placed by centring line boxes that carry descender room this
face never spends, so where a row's *box* sits says little about where its ink
lands. This measures the ink.

    python3 tools/measure.py shot-basalt.png [shot-chalk.png ...]

For each screenshot it prints the band depth as a fraction of the display, then
one line per band of ink in the field below: its top and bottom row, its height,
its horizontal extent, and its dominant colour. The gap between consecutive
bands is what matters when a row is added -- the holiday strip has to clear the
time above it and the date below it, and three rows is about the least that
still reads as a gap rather than as a collision.

Round screenshots are handled by ignoring everything outside the glass; the
corners of the framebuffer are not part of the display.
"""

import sys

from PIL import Image

# The emulator's simulated LCD washes colour out: GColorRed is genuinely
# #FF0000 but screenshots as #E35462. Greys come through exact. So "white" is
# tested generously and everything else is simply ink.
WHITE_MIN = 200


def glass_mask(w, h):
    """Rows of (x0, x1) spans that are actually on the display."""
    if w == h and w in (180, 260):  # chalk, gabbro
        r = w / 2.0
        cx = cy = r - 0.5
        rows = []
        for y in range(h):
            dy = abs(y - cy)
            if dy > r:
                rows.append(None)
                continue
            half = (r * r - dy * dy) ** 0.5
            rows.append((int(cx - half + 0.5), int(cx + half - 0.5)))
        return rows
    return [(0, w - 1)] * h


def name_colour(rgb):
    r, g, b = rgb
    if r > WHITE_MIN and g > WHITE_MIN and b > WHITE_MIN:
        return "white"
    if r > 120 and g < 120 and b < 120:
        return "red"
    if abs(r - g) < 24 and abs(g - b) < 24:
        return "black" if r < 60 else "grey"
    return "#%02x%02x%02x" % rgb


def measure(path):
    im = Image.open(path).convert("RGB")
    w, h = im.size
    px = im.load()
    spans = glass_mask(w, h)

    print("%s  %dx%d" % (path, w, h))

    # The band: leading rows that are mostly not white.
    band_h = 0
    for y in range(h):
        span = spans[y]
        if span is None:
            band_h = y + 1
            continue
        x0, x1 = span
        total = x1 - x0 + 1
        ink = sum(
            1 for x in range(x0, x1 + 1)
            if not (px[x, y][0] > WHITE_MIN and px[x, y][1] > WHITE_MIN
                    and px[x, y][2] > WHITE_MIN)
        )
        if ink * 2 <= total:
            break
        band_h = y + 1
    print("  band      rows 0-%-4d h=%-4d %5.1f%% of display"
          % (band_h - 1, band_h, 100.0 * band_h / h))

    # Ink in the field below, grouped into contiguous runs of rows.
    field_h = h - band_h
    runs = []
    cur = None
    for y in range(band_h, h):
        span = spans[y]
        found = []
        if span is not None:
            x0, x1 = span
            for x in range(x0, x1 + 1):
                c = px[x, y]
                if not (c[0] > WHITE_MIN and c[1] > WHITE_MIN
                        and c[2] > WHITE_MIN):
                    found.append((x, c))
        if found:
            if cur is None:
                cur = {"top": y, "bot": y, "x0": found[0][0],
                       "x1": found[-1][0], "cols": {}}
            cur["bot"] = y
            cur["x0"] = min(cur["x0"], found[0][0])
            cur["x1"] = max(cur["x1"], found[-1][0])
            for _, c in found:
                n = name_colour(c)
                cur["cols"][n] = cur["cols"].get(n, 0) + 1
        elif cur is not None:
            runs.append(cur)
            cur = None
    if cur is not None:
        runs.append(cur)

    prev_bot = band_h - 1
    for r in runs:
        cy = (r["top"] + r["bot"]) / 2.0
        dom = max(r["cols"].items(), key=lambda kv: kv[1])[0]
        print("  gap       %d rows" % (r["top"] - prev_bot - 1))
        print("  ink       rows %d-%-4d h=%-4d x %d-%-4d w=%-4d "
              "cy=%4.1f%% of field  %s"
              % (r["top"], r["bot"], r["bot"] - r["top"] + 1, r["x0"],
                 r["x1"], r["x1"] - r["x0"] + 1,
                 100.0 * (cy - band_h) / field_h, dom))
        prev_bot = r["bot"]
    print("  gap       %d rows to the bottom edge" % (h - 1 - prev_bot))
    print()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for p in sys.argv[1:]:
        measure(p)
