#pragma once

#include <pebble.h>

// Every tunable for the face lives here. Nothing below this file hardcodes a
// pixel value that depends on the platform.

// --- Palette ---------------------------------------------------------------
// Pebble-safe values only: two bits per channel, so no dithering anywhere.
// Three of the seven platforms are 1-bit, so the band needs a defined B&W
// fallback. It goes solid black there, and the composition survives whole --
// white crescent and stars on a dark chord, dark digits on the field below --
// because what carries the design is the cut and the emblem's position on it,
// never the hue.
#define SGF_COL_FIELD   GColorWhite
#define SGF_COL_BAND    PBL_IF_COLOR_ELSE(GColorRed, GColorBlack)
#define SGF_COL_EMBLEM  GColorWhite
#define SGF_COL_TIME    GColorBlack
#define SGF_COL_DATE    PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack)
#define SGF_COL_RULE    SGF_COL_BAND
// The holiday strip takes the rule's place, so it takes the rule's ink too --
// the red mark under the time has simply found something to say.
#define SGF_COL_HOLI    SGF_COL_RULE

// --- Type ------------------------------------------------------------------
// System fonts only. The face ships no font resources at all, so there is
// nothing to subset, nothing to keep inside aplite's 128 KB budget, and no
// licence to vendor. Both faces are hinted by the firmware for these exact
// pixel sizes rather than scaled down to them, which is why the small date
// stays legible where a scaled custom face went thin.
//
// Roboto Bold Subset 49 is the largest numeral face the platform offers, and
// the one the stock digital watchface uses. It is a subset -- digits, a colon
// and little else -- which is all the time row needs and exactly why it is
// large enough to matter. There is no bigger option, so every platform takes
// it and the time has no size classes.
#define SGF_FONT_TIME_KEY  FONT_KEY_ROBOTO_BOLD_SUBSET_49
#define SGF_TIME_PX  49

// The date steps with the screen. Gothic Bold has real weight at 14px, where a
// regular-weight face has none.
#if PBL_DISPLAY_WIDTH >= 240          // gabbro 260x260
  #define SGF_FONT_DATE_KEY  FONT_KEY_GOTHIC_24_BOLD
  #define SGF_DATE_PX  24
#elif PBL_DISPLAY_WIDTH >= 200        // emery 200x228
  #define SGF_FONT_DATE_KEY  FONT_KEY_GOTHIC_18_BOLD
  #define SGF_DATE_PX  18
#else                                 // 144x168 rect platforms, and chalk
  #define SGF_FONT_DATE_KEY  FONT_KEY_GOTHIC_14_BOLD
  #define SGF_DATE_PX  14
#endif

// The holiday strip is set one size step below the date, and that is not a
// stylistic choice -- it is what makes the row exist at all. Measured on the
// shipped screenshots, the gap between the time's ink and the date's ink is
// only 12 rows on a 144px screen (14 on chalk, 24 on emery, 29 on gabbro). A
// line at the date's own size inks 9 to 14 of those rows and would leave
// nothing either side, forcing the three field fractions apart and moving the
// whole composition. A step down inks 7 to 13 and drops in with clearance to
// spare, so SGF_TIME_CY_PCT, SGF_RULE_CY_PCT and SGF_DATE_CY_PCT are untouched.
//
// Nine is the smallest Gothic the firmware has and the only step below 14, so
// the 144px class has no lighter option than this. It is legible because the
// strip is short, upright and set in the band's red against white.
#if PBL_DISPLAY_WIDTH >= 240          // gabbro 260x260
  #define SGF_FONT_HOLI_KEY  FONT_KEY_GOTHIC_18_BOLD
  #define SGF_HOLI_PX  18
#elif PBL_DISPLAY_WIDTH >= 200        // emery 200x228
  #define SGF_FONT_HOLI_KEY  FONT_KEY_GOTHIC_14_BOLD
  #define SGF_HOLI_PX  14
#else                                 // 144x168 rect platforms, and chalk
  #define SGF_FONT_HOLI_KEY  FONT_KEY_GOTHIC_09
  #define SGF_HOLI_PX  9
#endif

// Line box as a fraction of the point size. TrueType faces rendered by the SDK
// sit in roughly 1.2em, and measurement says the ink then lands on the box's
// own centre line closely enough for both faces -- so a line is placed simply
// by centring its box, with no fudge factor for the descender.
#define SGF_LINE_H(px)  ((px) * 6 / 5)

// Tracking between the date's glyphs, as a fraction of its point size. The
// sheet sets the date as T U E - 2 8 - J U L, wide and quiet, so it reads as a
// caption rather than as a second number competing with the time. Gothic Bold
// is a wider face than the mono it replaced and needs less help to breathe.
#define SGF_DATE_TRACK  ((SGF_DATE_PX) / 5)
// Word gap. Gothic is proportional, and measuring a lone space through the text
// layout returns nothing useful, so the gap between words is set here instead.
// It stays clear of the letter tracking so the three words still read as three.
#define SGF_DATE_SPACE  ((SGF_DATE_PX) * 2 / 5)

// --- The band --------------------------------------------------------------
// The flag's red field, cut by the display edge, at exactly half the height on
// every platform. On a round screen that puts the cut on the diameter, the
// longest chord the glass has to offer, which is the whole idea of the design.
// On a square one it is simply the flag's own split, and it gives the emblem
// the room it needs to be seen.
#define SGF_BAND_H(h)  ((h) / 2)

// --- The emblem ------------------------------------------------------------
// Both panels of the design sheet put the crescent and the star ring on the
// band's own centre line, so the emblem row is band_h/2 everywhere and only the
// two horizontal positions need a platform split. Round pulls both inward:
// climbing away from the diameter, the circle takes width away fastest exactly
// where the emblem sits.
// Round's star ring sits 2% further out than the design sheet's own panel put
// it. With the emblem now scaled off the band, the sheet's 42% left only three
// pixels between the crescent's horn and the nearest star on chalk -- close
// enough to read as a smudge rather than as two marks.
#define SGF_CRESC_CX(w)  PBL_IF_ROUND_ELSE((w) * 21 / 100, (w) * 19 / 100)
#define SGF_STARS_CX(w)  PBL_IF_ROUND_ELSE((w) * 44 / 100, (w) * 48 / 100)

// The crescent scales off the band it sits on, and everything else about the
// emblem scales off the crescent, so the assembly keeps the flag's internal
// proportions at every size and grows with the red field.
//
// This was briefly tied to the display width instead, back when round took half
// the height and square took 38% -- an emblem keyed to the band came out 40%
// oversized on chalk, enough that the star ring ran into the crescent. Now that
// the band is a half everywhere, the band is once again the honest thing to
// measure against, and square gets the larger emblem its deeper band asks for.
#define SGF_CRESC_R(bh)       ((bh) * 19 / 100)
// The bite: a second disc in the band colour, slightly smaller and offset
// right, leaving the crescent open to the right as it is on the flag.
#define SGF_CRESC_BITE_R(r)   ((r) * 17 / 20)
#define SGF_CRESC_BITE_DX(r)  ((r) * 9 / 20)
// The ring the five stars stand on.
#define SGF_STARS_R(r)        ((r) * 13 / 10)
// Outer radius of one star. The flag's stars are five-pointed and this face
// draws them as such -- a dot would be a different emblem, not a simplified
// one. Sized to keep the five points legible while leaving the neighbouring
// stars clear of each other on the ring.
#define SGF_STAR_R(r)         ((r) * 5 / 12 < 4 ? 4 : (r) * 5 / 12)

// --- The field below -------------------------------------------------------
// All three rows are placed as fractions of the white field rather than of the
// screen, so they follow the band wherever it puts them. Now that the band is
// half the height on every platform the field is too, and one set of fractions
// serves round and square alike.
//
// The rule used to be derived from the gap between the time and date line
// boxes. That worked when the field was 62% of the screen; at 50% those boxes
// very nearly touch, and the midpoint between them lands on top of the date.
// A fraction of the field is both simpler and stable under type changes.
#define SGF_TIME_CY_PCT  38
// One fraction, two marks that are never both drawn: the rule, and -- in the
// week before a public holiday -- the countdown strip that displaces it.
#define SGF_RULE_CY_PCT  67
#define SGF_DATE_CY_PCT  79

// The short rule under the time, on rect platforms only. Round already carries
// far more red -- its band runs the full diameter -- and has no vertical slack
// left between the digits and the date to spend on a second mark.
#define SGF_RULE_W(w)  ((w) / 7)
#define SGF_RULE_H(h)  ((h) / 84 < 1 ? 1 : (h) / 84)

// --- Gutters ---------------------------------------------------------------
// Only the text rows need one. The band is meant to run edge to edge, and the
// emblem is positioned absolutely against the flag, not against a margin.
#define SGF_ROUND_INSET_H(w)  ((w) / 9)
#define SGF_RECT_INSET_H(w)   ((w) * 7 / 100)
