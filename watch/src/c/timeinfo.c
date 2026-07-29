#include "timeinfo.h"

static const char *const s_wday[7] = {
  "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

static const char *const s_mon[12] = {
  "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

void sgf_format_date(char *buf, size_t len, const struct tm *t) {
  // Both indices are taken modulo their table size. tm_wday and tm_mon are
  // already in range on every sane libc, but the tables are the only thing
  // standing between a bad value and a read off the end of .rodata.
  snprintf(buf, len, "%s %02d %s", s_wday[t->tm_wday % 7], t->tm_mday,
           s_mon[t->tm_mon % 12]);
}
