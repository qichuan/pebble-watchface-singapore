#!/usr/bin/env python3
"""Generates the app icons and the watch's menu icon.

Everything is built from the same construction the watchface uses -- the
crescent is a disc with a second disc taken out of it, and each star is a
ten-vertex polygon on a ring -- so the icons and the glass cannot drift apart.

The emblem is drawn deliberately larger here than on the face. A watchface is
read at arm's length on a 144px screen; an icon is read at 25px in a list, and
the crescent and stars are the only part of the flag that identifies it.

Run:  python3 tools/make_icons.py
"""
import math
import os

from PIL import Image, ImageDraw

# Supersampling factor. PIL has no anti-aliasing of its own, so everything is
# drawn large and resampled down; at these sizes the difference between that
# and a raw draw is the difference between a star and a smudge.
SS = 16

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

# Pantone 032, the flag's own red. Not Pebble's GColorRed -- the watch is
# limited to two bits per channel, a PNG is not, and the icon should show the
# colour the flag actually is.
FLAG_RED = (237, 41, 57, 255)
WHITE = (255, 255, 255, 255)
BLACK = (0, 0, 0, 255)
CLEAR = (0, 0, 0, 0)


def star_points(cx, cy, r):
    """Ten vertices of a five-pointed star, point up."""
    pts = []
    for i in range(10):
        # Outer vertices every 72 degrees; inner ones offset by 36 at the
        # 0.382 radius that gives a star its classic flag proportions.
        ang = math.radians(i * 36)
        rad = r if i % 2 == 0 else r * 0.382
        pts.append((cx + rad * math.sin(ang), cy - rad * math.cos(ang)))
    return pts


def draw_emblem(d, cresc_c, cresc_r, stars_c, ring_r, star_r, ink, behind):
    """Crescent and five stars in `ink`, cut against a `behind` background."""
    cx, cy = cresc_c
    d.ellipse([cx - cresc_r, cy - cresc_r, cx + cresc_r, cy + cresc_r],
              fill=ink)
    # The opening: a second disc in the background colour, smaller and offset
    # right, leaving the crescent open to the right as it is on the flag.
    bx, br = cx + cresc_r * 0.45, cresc_r * 0.85
    d.ellipse([bx - br, cy - br, bx + br, cy + br], fill=behind)

    sx, sy = stars_c
    for i in range(5):
        ang = math.radians(i * 72)
        d.polygon(star_points(sx + ring_r * math.sin(ang),
                              sy - ring_r * math.cos(ang), star_r), fill=ink)


def app_icon(n):
    """The flag as a square: red over white, emblem on the red."""
    im = Image.new("RGBA", (n * SS, n * SS), WHITE)
    d = ImageDraw.Draw(im)
    s = n * SS

    d.rectangle([0, 0, s, s // 2], fill=FLAG_RED)
    draw_emblem(d,
                cresc_c=(0.245 * s, 0.25 * s), cresc_r=0.145 * s,
                stars_c=(0.60 * s, 0.25 * s), ring_r=0.135 * s,
                star_r=0.055 * s,
                ink=WHITE, behind=FLAG_RED)

    return im.resize((n, n), Image.LANCZOS).convert("RGB")


def menu_icon(n=25):
    """The flag in two tones, for the watch's launcher.

    A dark ground carrying the emblem in white, over a light band -- so the
    icon reads as the flag's two bands rather than as an emblem floating in a
    list, and matches how the face itself renders on the 1-bit watches, where
    the red band goes solid black and the emblem is knocked out of it white.

    Three constraints fight here, and all were settled by thresholding the
    result and looking at it:

    - The SDK converts a menuIcon to 1-bit, so the anti-aliasing that makes a
      small star look star-shaped in this PNG is thrown away on the watch.
      Below about a 3px outer radius a five-pointed star hard-thresholds into a
      cross, and below that into a plain square.
    - Twenty-five pixels is not enough width for a crescent and a five-star
      ring at their natural relative sizes. The crescent is therefore drawn
      smaller than the flag's proportion, because it survives being shrunk and
      the stars do not.
    - The flag's own split is even, but an even split leaves the emblem too
      little height to survive the threshold. The black band is a third.

    Check the thresholded version, not this one, before changing any number.
    """
    im = Image.new("RGBA", (n * SS, n * SS), BLACK)
    d = ImageDraw.Draw(im)
    s = n * SS

    draw_emblem(d,
                cresc_c=(0.21 * s, 0.38 * s), cresc_r=0.175 * s,
                stars_c=(0.68 * s, 0.38 * s), ring_r=0.200 * s,
                star_r=0.130 * s,
                ink=WHITE, behind=BLACK)

    d.rectangle([0, 0.70 * s, s, s], fill=WHITE)

    return im.resize((n, n), Image.LANCZOS)


def main():
    icon_dir = os.path.join(ROOT, "developer-portal", "app_icon")
    img_dir = os.path.join(ROOT, "watch", "resources", "images")
    os.makedirs(icon_dir, exist_ok=True)
    os.makedirs(img_dir, exist_ok=True)

    for n in (80, 144):
        p = os.path.join(icon_dir, f"icon-{n}.png")
        app_icon(n).save(p)
        print(f"  {p}  {n}x{n}")

    p = os.path.join(img_dir, "menu_icon.png")
    menu_icon().save(p)
    print(f"  {p}  25x25 black on white")


if __name__ == "__main__":
    main()
