#pragma once

#include <pebble.h>

// Everything the face draws, formatted once per tick and then left alone. The
// update proc never formats, allocates, or reads a service.
typedef struct {
  char time[8];   // "11:06"
  char date[16];  // "TUE 28 JUL"
} SgfModel;

typedef struct {
  GFont time;
  GFont date;
} SgfFonts;

void sgf_render(GContext *ctx, GRect bounds, const SgfModel *m,
                const SgfFonts *f);
