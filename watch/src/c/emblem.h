#pragma once

#include <pebble.h>

// The flag's two marks, drawn onto the band. Both assume the band has already
// been filled -- the crescent's opening is cut by painting back over it in the
// band colour, so there has to be a band to paint.

// A crescent of radius `r` centred on `c`, opening to the right.
void sgf_draw_crescent(GContext *ctx, GPoint c, int r);

// The flag's five stars: five-pointed, point up, each of outer radius `star_r`,
// standing on a point-up ring of radius `ring_r` centred on `c`.
void sgf_draw_stars(GContext *ctx, GPoint c, int ring_r, int star_r);
