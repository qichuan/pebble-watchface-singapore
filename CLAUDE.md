# CLAUDE.md

## What this is

**Majulah** — a Pebble watchface. The Singapore flag's red band is cut by the
display edge, so the crescent and its five stars sit exactly where they do on
the flag — top-left, small, unfussy. Everything below the cut is white space for
the hour.

Named for the first word of *Majulah Singapura*. The design sheet it was built
from calls the layout "Crescent Field"; that phrase survives only as a
description of the composition, never as the product name.

| Directory | Language | Role |
| --- | --- | --- |
| `watch/` | C | the app; all `pebble` commands run from here |
| `tools/` | C + sh + py | host-side tests, and the icon generator |
| `developer-portal/` | assets | store copy, screenshots and app icons |

## Commands

```sh
cd watch
pebble build
pebble clean                                  # needed after package.json changes
pebble install --emulator basalt
pebble screenshot --emulator basalt --no-open shot.png

tools/run_tests.sh                            # from the repo root; no SDK needed
python3 tools/make_icons.py                   # regenerates all three icons
```

## Architecture

| File | Owns |
| --- | --- |
| `src/c/main.c` | window lifecycle, the tick subscription, formatting into `SgfModel` |
| `src/c/sgf_config.h` | palette, type sizes per platform class, band and emblem ratios |
| `src/c/layout.c` | `sgf_layout()` — band, emblem geometry, time/rule/date rects |
| `src/c/render.c` | `sgf_render()` — the one update proc, plus the tracked-text setter |
| `src/c/emblem.c` | the crescent and the five-star ring |
| `src/c/timeinfo.c` | the date string. Pure C, no SDK calls |

Data flows one way: the tick handler formats into `s_model`, then calls
`layer_mark_dirty()`. The update proc only reads. It never formats, allocates,
or touches a service.

## Key constraints & gotchas

- **A wedged emulator will accept an install and keep showing the old face.**
  This has now happened twice, on gabbro and on emery: `pebble install` printed
  `App install succeeded` and the screenshot was the *previous* project's
  watchface. The install output is not evidence — only the screenshot is. If it
  is not this face, `pkill -9 -f qemu-pebble`, delete that platform's state
  under `~/Library/Application Support/Pebble SDK/4.17/<platform>/`, and install
  again. Relatedly, `pebble kill --force` can leave the tool itself spinning at
  100% CPU — check `ps` for a stray `bin/pebble` afterwards.
- **Run `pebble` from `watch/`.** From the repo root it fails without printing
  anything a grep for "error" will catch, and a following `pebble screenshot`
  still succeeds — capturing whatever was already on the emulator.
- **The emulator's clock follows the host**, so the face can only ever be
  screenshotted at the current time and date. Everything calendar-shaped is
  therefore pinned in `tools/timeinfo_test.c` instead — the whole weekday and
  month table, the zero-padded day, the leap day — rather than eyeballed.
- **The band is half the height on every platform, and the emblem scales off
  it.** That pairing is the whole geometry: change one and the other follows.
  The emblem was briefly keyed to the display width instead, back when square
  took 38% and round took half — with the two bands at different fractions, a
  band-scaled emblem came out ~40% oversized on chalk and the star ring ran into
  the crescent. Equal bands make the band the honest thing to measure against
  again. If anyone reintroduces a per-platform band fraction, this breaks.
- **Watch the crescent-to-star clearance when the emblem grows.** It is the
  first thing to fail and the last thing the eye notices — 3px still looks like
  two marks in a thumbnail and like a smudge on the wrist. The measure script
  reports it; chalk is always the tightest, which is why its star ring sits 2%
  further out than the design sheet's own panel put it.
- **`prv_fit_round()` probes the row's ink depth, not its box height.** The line
  box carries descender room that this face never spends — no glyph on it
  descends. Probing the box hands chalk's date a chord about a dozen pixels
  narrower than it really has, which is exactly the margin its letter-spacing
  needs.
- **Do not fudge text placement vertically.** An early version lifted every line
  by `px/6` to compensate for the SDK's leading. Measuring the rendered ink
  showed the lift was the *only* thing putting the rows off target: centring the
  line box lands the ink within about 2px of its intended centre for both faces,
  on every platform.
- **System fonts only — the face ships no font resources.** The only thing in
  `media` is the menu icon. Both faces come from `fonts_get_system_font()`,
  belong to the firmware, and must **never** be passed to
  `fonts_unload_custom_font()`. `prv_window_unload()` therefore unloads no fonts
  at all, which is correct and not an oversight.
- **Icons are generated, not hand-drawn.** `tools/make_icons.py` builds all
  three from the same crescent-and-star construction the face uses, so they
  cannot drift from the glass. Edit the script, re-run it, never touch the PNGs.
- **A `menuIcon` is hard-converted to 1-bit.** Anti-aliasing that makes a small
  star look star-shaped in the PNG is thrown away on the watch. Below about a
  3px outer radius a five-pointed star thresholds into a cross, and below that
  into a plain square — which is why the menu icon's crescent is drawn smaller
  than the flag's proportion and its stars larger. The crescent survives being
  shrunk; the stars do not.
- **Judge the menu icon thresholded, at size, and then in the launcher.** A
  four-up contact sheet of candidates is not enough — a variant that looked
  star-shaped there turned out to be five crosses once zoomed. Threshold the
  image at 50%, view it at 14x, and confirm on the watch:
  `pebble emu-button --emulator basalt click select` twice reaches the
  watchface list, where the icon is actually drawn.
- **`ROBOTO_BOLD_SUBSET_49` is the ceiling.** It is the largest numeral face the
  platform offers, so emery and gabbro get the same 49px as a 144px watch and
  the time has no size classes. If a future change wants bigger digits on
  gabbro, that means bringing a custom font back — there is no system font above
  49px.
- **`ROBOTO_BOLD_SUBSET_49` is a subset.** Digits and a colon are in it; assume
  nothing else is. It is fine for `%H:%M` and would be wrong for anything with a
  letter in it.
- **The time row takes the full bounds width, not the text gutter.** At 49px the
  digits are wider than a 144px screen's gutter, and `GTextOverflowModeFill`
  answers that by ellipsizing — the face rendered `10:...` instead of `10:19`,
  on rect platforms only, while chalk looked perfect. The gutter is for the
  date.
- **`prv_draw_tracked()` measures every glyph, not just one.** That is required
  now the date face is proportional. The earlier version measured a single `0`
  and reused the advance for the whole string, which is correct only for a
  monospaced face; against Gothic it bunches the narrow characters. If the date
  face ever changes, check this assumption first.
- **The chord is free.** On chalk and gabbro the framebuffer is circular, so a
  full-width rect simply falls off the glass and what is left is the chord. No
  clipping of our own is involved, and no round-specific band value either —
  the half is what every platform uses.
- **Place the rule by fraction, not by the gap between line boxes.** It used to
  be derived as the midpoint between the time and date boxes. That held while
  the field was 62% of the screen; at 50% those boxes very nearly touch and the
  midpoint lands on top of the date.
- **The stars are five-pointed, and they must stay that way.** This is a
  national flag; a dot is a different emblem, not a simplified one. The original
  design sheet called for dots on the grounds that a small star turns to mush —
  it does not, provided the fill is done properly.
- **Do not use `gpath_draw_filled()` for the stars.** It cannot cope with a
  concave polygon at this size: a 9px star came out of it as a three-pixel
  corner, identically on every platform. `prv_fill_poly()` in `emblem.c` does
  an even-odd scanline fill instead — about thirty lines, exact at any radius,
  and no heap. The `GPath` struct is public and only carries four fields, so if
  a future shape does need it, the path can live on the stack.
- **Round the star's vertices, do not truncate them.** At an outer radius of 5
  the difference is a visibly bent point. `prv_scale()` rounds to nearest, and
  the inner-vertex table has the 0.382 star ratio already folded in so the
  coordinates are scaled once rather than twice.
- **Nothing may rely on hue alone.** Three of the seven platforms are 1-bit. The
  band goes solid black there and the composition survives, because what carries
  the design is the cut and the emblem's position on it.
- **The emulator washes out colour.** `GColorRed` is genuinely `#FF0000`, but
  every emulator screenshots the band as `#E35462`. That is QEMU's simulated
  LCD, not a bug, and not something to "correct" in the source — the flag red it
  lands on is in fact closer to the real thing than `#FF0000` is. Greys come
  through exact (`GColorDarkGray` reads `#545454`), so use the date row to tell
  a filter artefact from a real problem.

## Verifying a change

1. `tools/run_tests.sh` — the date string, across the whole calendar.
2. `pebble build` from `watch/` — must be clean for all seven platforms.
3. Screenshot basalt (the sheet's rect panel), chalk (its round panel, and the
   worst case for the chord fit), gabbro (260×260 round), and aplite (1-bit).
   The 144px platforms are the worst case for the stars — if the five points
   survive there they survive everywhere.
4. **Measure rather than squint.** The design sheet is a table of ratios — band
   depth, emblem centres, the time and date rows — and a script that reports
   those ratios off a screenshot catches a 3px drift the eye will not. Every
   layout decision in this repo was settled that way, and two of them came out
   the opposite of what the screenshot suggested.
   For the stars specifically, check the count *and* the shape: five separate
   connected components, each filling well under 60% of its bounding box. A
   filled disc fills about 79% of its box and a real star about 40%, so that one
   ratio is what tells a star from a blob without having to zoom in.

## Conventions

- Static functions `prv_`, file-scope state `s_`, tunables in `sgf_config.h`.
- No `malloc`. Fixed-size buffers in `SgfModel`.
- Layout is always derived from `layer_get_bounds()` — never a hardcoded pixel
  value keyed to a platform.
- Capability macros (`PBL_ROUND`, `PBL_COLOR`, `PBL_BW`, `PBL_DISPLAY_WIDTH`)
  only; never `PBL_PLATFORM_*`.
- `wscript` is the stock SDK default. Do not edit it.
