#pragma once

#include <pebble.h>

// Writes the date row, e.g. "TUE 28 JUL".
//
// Built from fixed uppercase tables rather than strftime("%a %b"): the design
// wants uppercase, and this sidesteps both the locale and the need to
// case-convert in place. Pure C, so it compiles and runs on the host.
void sgf_format_date(char *buf, size_t len, const struct tm *t);
