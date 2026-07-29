#include "emblem.h"
#include "sgf_config.h"

#define SGF_STAR_COUNT 5
#define SGF_STAR_VERTS 10

// The five ring positions, as offsets per 1000 units of ring radius: a regular
// pentagon with a vertex straight up, so the stars sit the way they do on the
// flag rather than as a rosette.
static const int16_t s_ring_x[SGF_STAR_COUNT] = {0, 951, 588, -588, -951};
static const int16_t s_ring_y[SGF_STAR_COUNT] = {-1000, -309, 809, 809, -309};

// One five-pointed star, point up: ten vertices alternating between the outer
// radius and the inner radius of 0.382R that gives the star its classic flag
// proportions. Offsets are per 1000 units of the outer radius, and the inner
// ones already have the 0.382 folded in -- scaling once, from a table computed
// at full precision, keeps the small sizes from being rounded twice.
static const int16_t s_star_x[SGF_STAR_VERTS] = {
  0, 225, 951, 363, 588, 0, -588, -363, -951, -225
};
static const int16_t s_star_y[SGF_STAR_VERTS] = {
  -1000, -309, -309, 118, 809, 382, 809, 118, -309, -309
};

// Scales a per-1000 offset to `r` pixels, rounding to nearest rather than
// truncating. At the sizes involved -- a four-pixel star on a 144px screen --
// truncation alone visibly bends the points out of shape.
static int prv_scale(int per_mille, int r) {
  const int v = per_mille * r;
  return (v >= 0) ? ((v + 500) / 1000) : -((-v + 500) / 1000);
}

void sgf_draw_crescent(GContext *ctx, GPoint c, int r) {
  if (r < 2) {
    return;
  }
  graphics_context_set_fill_color(ctx, SGF_COL_EMBLEM);
  graphics_fill_circle(ctx, c, r);

  // The opening is a second disc in the band colour, smaller and offset right.
  // Nothing else has been drawn on the band yet, so painting over it is safe.
  graphics_context_set_fill_color(ctx, SGF_COL_BAND);
  graphics_fill_circle(
      ctx, GPoint((int16_t)(c.x + SGF_CRESC_BITE_DX(r)), c.y),
      SGF_CRESC_BITE_R(r));
}

// Fills a closed polygon by scanline, even-odd rule.
//
// This is deliberately not gpath_draw_filled(). That primitive cannot cope with
// a concave polygon at the size the flag's stars need -- a nine-pixel star came
// out of it as a three-pixel corner, on every platform. Filling the spans here
// costs about thirty lines and gives exact, predictable pixels at any radius,
// which is the only way the five points survive on a 144px screen.
static void prv_fill_poly(GContext *ctx, const GPoint *pts, int n) {
  int top = pts[0].y;
  int bot = pts[0].y;
  for (int i = 1; i < n; i++) {
    if (pts[i].y < top) {
      top = pts[i].y;
    }
    if (pts[i].y > bot) {
      bot = pts[i].y;
    }
  }

  for (int y = top; y <= bot; y++) {
    // Where this scanline crosses each edge. The half-open test counts a shared
    // vertex once, so the spans never double back on themselves.
    int xs[SGF_STAR_VERTS];
    int cnt = 0;
    for (int i = 0, j = n - 1; i < n; j = i++) {
      const int y0 = pts[j].y;
      const int y1 = pts[i].y;
      if (((y0 <= y) && (y1 > y)) || ((y1 <= y) && (y0 > y))) {
        const int x0 = pts[j].x;
        const int x1 = pts[i].x;
        xs[cnt++] = x0 + (((y - y0) * (x1 - x0)) / (y1 - y0));
      }
    }

    for (int a = 1; a < cnt; a++) {
      const int v = xs[a];
      int b = a - 1;
      while ((b >= 0) && (xs[b] > v)) {
        xs[b + 1] = xs[b];
        b--;
      }
      xs[b + 1] = v;
    }

    for (int a = 0; (a + 1) < cnt; a += 2) {
      graphics_fill_rect(ctx,
                         GRect((int16_t)xs[a], (int16_t)y,
                               (int16_t)(xs[a + 1] - xs[a] + 1), 1),
                         0, GCornerNone);
    }
  }
}

static void prv_draw_star(GContext *ctx, GPoint c, int r) {
  GPoint pts[SGF_STAR_VERTS];
  for (int i = 0; i < SGF_STAR_VERTS; i++) {
    pts[i].x = (int16_t)(c.x + prv_scale(s_star_x[i], r));
    pts[i].y = (int16_t)(c.y + prv_scale(s_star_y[i], r));
  }
  prv_fill_poly(ctx, pts, SGF_STAR_VERTS);
}

void sgf_draw_stars(GContext *ctx, GPoint c, int ring_r, int star_r) {
  graphics_context_set_fill_color(ctx, SGF_COL_EMBLEM);
  for (int i = 0; i < SGF_STAR_COUNT; i++) {
    const GPoint p = {
      .x = (int16_t)(c.x + ((s_ring_x[i] * ring_r) / 1000)),
      .y = (int16_t)(c.y + ((s_ring_y[i] * ring_r) / 1000)),
    };
    prv_draw_star(ctx, p, star_r);
  }
}
