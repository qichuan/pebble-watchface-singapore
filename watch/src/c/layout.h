#pragma once

#include <pebble.h>

// The face, resolved against the root layer's bounds. Nothing here is hardcoded
// per platform, so one set of rules serves 144x168, 180x180, 200x228 and
// 260x260, square and round alike.
typedef struct {
  GRect band;      // the flag's red field, cut by the display edge
  GPoint cresc;    // crescent centre, on the band's centre line
  int cresc_r;
  GPoint stars;    // centre of the five-star ring, on the same line
  int stars_r;     // radius of the ring the stars stand on
  int star_r;      // outer radius of one five-pointed star
  GRect time;      // line box for the digits
  GRect rule;      // the short rule under them; empty where it is not drawn
  GRect holiday;   // the countdown strip, in the rule's slot; empty when idle
  GRect date;      // line box for the tracked date
} SgfLayout;

// `countdown` is true in the week before a public holiday. It swaps one mark
// for another in a single slot -- the strip for the rule -- and changes nothing
// else about the composition.
SgfLayout sgf_layout(GRect bounds, bool countdown);
