#include <string.h>

#include "render.h"
#include "emblem.h"
#include "layout.h"
#include "sgf_config.h"

// Longest date the face can produce is "WED 29 JUL"; the model's buffer is 16.
#define SGF_DATE_MAX 16

// Draws `s` letter-spaced and centred in `row`. The SDK has no tracking, so the
// run is set glyph by glyph.
//
// Every glyph is measured individually rather than once: Gothic is a
// proportional face, so a 'W' and a '1' do not share an advance, and reusing a
// single measurement across the string -- which is all a monospaced face
// needs -- would visibly bunch the narrow characters.
static void prv_draw_tracked(GContext *ctx, const char *s, GFont font, GRect row,
                             int track, int space_w) {
  const int n = (int)strlen(s);
  if ((n == 0) || (n > SGF_DATE_MAX)) {
    return;
  }

  int adv[SGF_DATE_MAX];
  int ink = 0;
  char glyph[2] = {'\0', '\0'};
  for (int i = 0; i < n; i++) {
    glyph[0] = s[i];
    adv[i] = (glyph[0] == ' ')
                 ? space_w
                 : graphics_text_layout_get_content_size(
                       glyph, font, row, GTextOverflowModeFill,
                       GTextAlignmentLeft)
                       .w;
    ink += adv[i];
  }
  if (ink <= 0) {
    return;
  }

  // Tracking is the first thing to give up. Chalk's date sits low enough on the
  // glass that the chord there is barely wider than the untracked string, and a
  // date that fits without its letter-spacing beats one that runs off the edge.
  if (n > 1) {
    const int room = row.size.w - ink;
    if (room < (track * (n - 1))) {
      track = (room > 0) ? (room / (n - 1)) : 0;
    }
  }

  int x = row.origin.x + ((row.size.w - (ink + (track * (n - 1)))) / 2);

  for (int i = 0; i < n; i++) {
    glyph[0] = s[i];
    if (glyph[0] != ' ') {
      // The draw box is given slack beyond the measured advance so a glyph
      // whose ink runs a pixel past its metrics is not clipped by Fill mode.
      // Only the advance moves the pen, so the slack costs nothing.
      graphics_draw_text(
          ctx, glyph, font,
          GRect((int16_t)x, row.origin.y, (int16_t)(adv[i] + 4), row.size.h),
          GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }
    x += adv[i] + track;
  }
}

void sgf_render(GContext *ctx, GRect bounds, const SgfModel *m,
                const SgfFonts *f) {
  const SgfLayout l = sgf_layout(bounds);

  graphics_context_set_fill_color(ctx, SGF_COL_FIELD);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // The band is drawn as a full-width rect. On chalk and gabbro the framebuffer
  // is circular, so the part beyond the glass is simply never shown and what
  // remains is the chord -- the design's central move, and it costs nothing.
  graphics_context_set_fill_color(ctx, SGF_COL_BAND);
  graphics_fill_rect(ctx, l.band, 0, GCornerNone);

  sgf_draw_crescent(ctx, l.cresc, l.cresc_r);
  sgf_draw_stars(ctx, l.stars, l.stars_r, l.star_r);

  graphics_context_set_text_color(ctx, SGF_COL_TIME);
  graphics_draw_text(ctx, m->time, f->time, l.time, GTextOverflowModeFill,
                     GTextAlignmentCenter, NULL);

  // Empty on round, where the layout declines to place it.
  if (l.rule.size.w > 0) {
    graphics_context_set_fill_color(ctx, SGF_COL_RULE);
    graphics_fill_rect(ctx, l.rule, 0, GCornerNone);
  }

  graphics_context_set_text_color(ctx, SGF_COL_DATE);
  prv_draw_tracked(ctx, m->date, f->date, l.date, SGF_DATE_TRACK,
                   SGF_DATE_SPACE);
}
