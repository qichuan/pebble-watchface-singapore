#pragma once

#include <pebble.h>

#include "holidays.h"

// Everything the face draws, formatted once per tick and then left alone. The
// update proc never formats, allocates, or reads a service.
typedef struct {
  char time[8];   // "11:06"
  char date[16];  // "TUE 28 JUL"
  // The countdown, in two lengths. An empty `holiday` means there is no public
  // holiday within the week and the rule is drawn instead -- no separate flag.
  // The short form exists because the widest label is 22 characters and the
  // narrowest row is a chord on chalk; which one is used is settled by
  // measurement at draw time, not by a per-platform table.
  char holiday[SGF_HOLI_BUF];        // "CHINESE NEW YEAR - 3D"
  char holiday_short[SGF_HOLI_BUF];  // "CNY - 3D"
} SgfModel;

typedef struct {
  GFont time;
  GFont date;
  GFont holiday;
} SgfFonts;

void sgf_render(GContext *ctx, GRect bounds, const SgfModel *m,
                const SgfFonts *f);
