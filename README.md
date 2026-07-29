# Crescent Field

A Pebble watchface: the Singapore flag, reduced to a chord.

<p align="center">
  <img src="developer-portal/screenshots/basalt/basalt.png" width="144" alt="Basalt">
  <img src="developer-portal/screenshots/chalk/chalk.png" width="180" alt="Chalk">
  <img src="developer-portal/screenshots/aplite/aplite.png" width="144" alt="Aplite">
</p>

The red band is cut by the display edge, so the crescent and its five stars sit
exactly where they do on the flag — top-left, small, unfussy. Everything below
the cut is white space for the hour.

On a round watch the cut lands on the diameter, which is the longest chord the
glass has to offer. That is the whole idea: the shape the flag makes on a circle
is not a compromise, it is the better version.

The five stars are five-pointed, as they are on the flag, down to the 144px
watches — where each one is nine pixels across. They are filled by a scanline
routine in `emblem.c` rather than by the SDK's `gpath_draw_filled()`, which
cannot render a concave polygon at that size.

## Requirements

Runs on all seven Pebble platforms:

| Platform | Screen | Notes |
| --- | --- | --- |
| aplite | 144×168, 1-bit | band goes black |
| basalt | 144×168, colour | |
| chalk | 180×180, colour, round | the cut is the chord |
| diorite | 144×168, 1-bit | band goes black |
| emery | 200×228, colour | |
| flint | 144×168, 1-bit | band goes black |
| gabbro | 260×260, colour, round | the cut is the chord |

The band is half the height on every one of them.

Three of the seven are 1-bit and have no red to spend. There the band goes solid
black, and the composition survives whole — white crescent and stars above, dark
digits below — because what carries the design is the cut and the emblem's
position on it, never the hue.

## Building

```sh
cd watch
pebble build
pebble install --emulator basalt
```

The date string compiles and runs on the host, with no SDK or emulator involved:

```sh
tools/run_tests.sh
```

The emulator's clock follows the host, so the face can only ever be
screenshotted at the current time and date. Everything calendar-shaped — the
weekday and month tables, the zero-padded day, the leap day — is pinned in those
host tests instead of eyeballed on the glass.

## Layout

```
+--------------------------+  <- 0
|            *             |
|  (      *     *          |     crescent and five five-pointed stars,
|  (       *   *           |     both on the band's centre line
+--------------------------+  <- half the height, every platform
|                          |
|         10:43            |     38% of the field below
|           --             |     67% — the short rule; rect only
|       WED 29 JUL         |     79%
+--------------------------+
```

The band takes half the height everywhere. On a round screen that puts the cut
on the diameter; on a square one it is the flag's own split, and it is what
gives the crescent and stars room to be seen at this size. The three rows below
are placed as fractions of the white field rather than of the screen, so they
follow the band.

Every one of those numbers is a ratio of the root layer's bounds, not a pixel
value, so 144×168, 180×180, 200×228 and 260×260 all fall out of one set of
rules. They were set by measuring the rendered screenshots against the design
sheet rather than by eye — see `CLAUDE.md`, where two of the decisions came out
the opposite of what the screenshot suggested.

## Fonts

**System fonts only.** The face ships no font resources at all — nothing to
subset, no licence to vendor, and nothing counting against aplite's 128 KB
budget. Both faces are hinted by the firmware for the exact pixel sizes used
rather than scaled down to them, which is why the small date stays legible.

| Row | Font |
| --- | --- |
| Time | Roboto Bold Subset 49 — the largest numeral face the platform has, and the one the stock digital watchface uses |
| Date | Gothic Bold, stepping 14 / 18 / 24 px so gabbro does not get a caption sized for a 144px screen |

The time has no size classes because 49px is the ceiling; there is no larger
system font to give the 200px and 260px screens. The trade is deliberate — on
gabbro the digits are smaller relative to the glass than a custom face could
have made them.

The date is letter-spaced, which the SDK does not offer, so it is set glyph by
glyph — each one measured on its own, since Gothic is proportional and a `W` and
a `1` do not share an advance. Where the chord is too narrow for the tracking —
the bottom of chalk's glass — the tracking is what gives way, not the date.
