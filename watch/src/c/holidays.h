#pragma once

#include <pebble.h>

// How far ahead the face starts counting down. Seven days: near enough that the
// holiday is a plan rather than a rumour, and short enough that the strip is
// absent from the face for most of the year.
//
// This lives here rather than in sgf_config.h because holidays.c is pure logic
// and is compiled on the host, where the PBL_* macros sgf_config.h is built out
// of do not exist.
#define SGF_HOLI_LEAD_DAYS 7

// Longest label the tables can produce is "CHINESE NEW YEAR TODAY", 22 chars.
#define SGF_HOLI_BUF 28

// Writes the countdown label for the next Singapore public holiday within
// SGF_HOLI_LEAD_DAYS of `t` -- the long form into `buf`, a shorter one into
// `sbuf` for platforms whose row cannot hold the long one. Returns false and
// leaves both buffers untouched when there is no such holiday, or when `t` has
// run past the end of the gazetted table.
//
// Pure C, no SDK calls, so it compiles and runs on the host.
bool sgf_format_holiday(char *buf, size_t len, char *sbuf, size_t slen,
                        const struct tm *t);
