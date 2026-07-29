# Changelog

## 1.0.0 — 2026-07-29

First release.

- The Singapore flag's red band, cut by the display edge, with the crescent and
  its five stars in the flag's own top-left position. Everything below the cut
  is white space for the hour.
- The band takes half the height on every platform. On round screens the cut
  lands on the diameter — the longest chord the glass has — so the band's lower
  edge is as wide as the watch can make it. On square screens it is the flag's
  own split, and it gives the emblem the room it needs to be read.
- The crescent and stars scale off the band they sit on, so they grow with it.
- The five stars are five-pointed, as on the flag, on every platform — nine
  pixels across on the 144px watches and nineteen on gabbro. They are filled by
  an even-odd scanline routine rather than by `gpath_draw_filled()`, which
  renders a concave polygon of that size as a three-pixel corner.
- System fonts only: the face ships no font resources, so there is nothing to
  subset, no licence to vendor and nothing counting against aplite's 128 KB
  budget. Time in Roboto Bold Subset 49, the largest numeral face the platform
  has; date in Gothic Bold, stepping 14 / 18 / 24 px by screen size.
- The date is letter-spaced, which the SDK does not offer, so it is set glyph by
  glyph with each one measured individually against the proportional face. Where
  the chord is narrower than the tracked string, the tracking gives way and the
  date stays whole.
- All seven platforms. Every position is a ratio of the root layer's bounds, so
  144×168, 180×180, 200×228 and 260×260 fall out of one set of rules, with round
  screens additionally fitting each text row to the chord at its own height.
- On the three 1-bit platforms the band goes solid black. Nothing on the face
  relies on hue to carry meaning, so the composition survives intact.
- No complications. The empty field below the cut is the design.
- Host-side tests for the date string across the whole calendar — every weekday
  and month, the zero-padded day, the leap day — since the emulator's clock
  follows the host and cannot be parked on another date.
