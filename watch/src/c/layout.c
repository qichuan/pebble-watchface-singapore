#include "layout.h"
#include "sgf_config.h"

#if defined(PBL_ROUND)
static int prv_isqrt(int v) {
  int r = 0;
  while (((r + 1) * (r + 1)) <= v) {
    r++;
  }
  return r;
}

// Half-width of the glass at row `y`. A single inset cannot serve a round
// screen: the date sits near the bottom, where the circle has already given
// back most of its width, so a rectangle wide enough for the digits runs
// straight off the glass one row lower.
static int prv_chord_half(GRect bounds, int y) {
  const int r = bounds.size.w / 2;
  const int cy = bounds.origin.y + bounds.size.h / 2;
  const int dy = (y < cy) ? (cy - y) : (y - cy);
  if (dy >= r) {
    return 0;
  }
  return prv_isqrt((r * r) - (dy * dy));
}

// Shrinks `row` to the chord actually available, measured at whichever of its
// edges is further from the centre.
//
// The probe uses the row's *ink* depth, `px`, not its box height. The box
// carries descender room that this face never spends -- the time is digits and
// a colon, the date is capitals and digits, and not one glyph on the face
// descends. Measuring the box instead would hand chalk's date a chord a dozen
// pixels narrower than it actually has, which is the difference between keeping
// its letter-spacing and losing it.
static GRect prv_fit_round(GRect bounds, GRect row, int px) {
  const int cy = bounds.origin.y + bounds.size.h / 2;
  const int probe = (row.origin.y < cy) ? row.origin.y : (row.origin.y + px);
  int half = prv_chord_half(bounds, probe) - 3;
  if (half < 10) {
    half = 10;
  }
  const int cx = bounds.origin.x + bounds.size.w / 2;
  if ((half * 2) < row.size.w) {
    row.origin.x = (int16_t)(cx - half);
    row.size.w = (int16_t)(half * 2);
  }
  return row;
}
#endif

// A line box for `px`-point type, centred on `cy`.
static GRect prv_line(GRect gutter, int cy, int px) {
  const int h = SGF_LINE_H(px);
  return GRect(gutter.origin.x, (int16_t)(cy - (h / 2)), gutter.size.w,
               (int16_t)h);
}

SgfLayout sgf_layout(GRect bounds) {
  SgfLayout l;

  const int w = bounds.size.w;
  const int h = bounds.size.h;

  // --- The band and its emblem ---------------------------------------------
  // Both run to the display edge. On a round screen the rect simply falls off
  // the glass and what is left is the chord -- the framebuffer is circular, so
  // no clipping of our own is needed.
  const int band_h = SGF_BAND_H(h);
  l.band = GRect(bounds.origin.x, bounds.origin.y, (int16_t)w, (int16_t)band_h);

  const int emblem_cy = bounds.origin.y + (band_h / 2);
  l.cresc_r = SGF_CRESC_R(band_h);
  l.cresc = GPoint((int16_t)(bounds.origin.x + SGF_CRESC_CX(w)),
                   (int16_t)emblem_cy);
  l.stars_r = SGF_STARS_R(l.cresc_r);
  l.stars = GPoint((int16_t)(bounds.origin.x + SGF_STARS_CX(w)),
                   (int16_t)emblem_cy);
  l.star_r = SGF_STAR_R(l.cresc_r);

  // --- The field below -----------------------------------------------------
  const GRect gutter = grect_inset(
      bounds, GEdgeInsets(0, PBL_IF_ROUND_ELSE(SGF_ROUND_INSET_H(w),
                                               SGF_RECT_INSET_H(w))));

  const int field_y = bounds.origin.y + band_h;
  const int field_h = h - band_h;

  // The time takes the full width of the glass, not the text gutter. At 49px
  // the digits are wider than a 144px screen's gutter, and GTextOverflowModeFill
  // answers that by ellipsizing the minutes -- "10:..." rather than "10:19".
  // The gutter exists for the date, which is small enough to want one.
  l.time = prv_line(bounds, field_y + ((field_h * SGF_TIME_CY_PCT) / 100),
                    SGF_TIME_PX);
  l.date = prv_line(gutter, field_y + ((field_h * SGF_DATE_CY_PCT) / 100),
                    SGF_DATE_PX);

#if defined(PBL_ROUND)
  l.rule = GRect(0, 0, 0, 0);
#else
  const int rule_h = SGF_RULE_H(h);
  const int rule_w = SGF_RULE_W(w);
  l.rule = GRect(
      (int16_t)(bounds.origin.x + ((w - rule_w) / 2)),
      (int16_t)((field_y + ((field_h * SGF_RULE_CY_PCT) / 100)) - (rule_h / 2)),
      (int16_t)rule_w, (int16_t)rule_h);
#endif

#if defined(PBL_ROUND)
  l.time = prv_fit_round(bounds, l.time, SGF_TIME_PX);
  l.date = prv_fit_round(bounds, l.date, SGF_DATE_PX);
#endif

  return l;
}
