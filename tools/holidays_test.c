// Pins sgf_format_holiday(): the gazetted table, the seven-day window and its
// two edges, the day-zero wording, and the fact that the buffers in SgfModel
// are big enough for the longest label the tables can produce.
//
// None of this can be checked on the glass. The emulator resyncs its clock to
// the host, so the face can only ever be screenshotted at today's date -- and
// a countdown is, by definition, about a date that is not today.
//
// Build and run:  tools/run_tests.sh

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../watch/src/c/holidays.h"

static int s_failures = 0;

static void fill(struct tm *t, int y, int mon, int mday) {
  memset(t, 0, sizeof(*t));
  t->tm_year = y - 1900;
  t->tm_mon = mon - 1;
  t->tm_mday = mday;
  t->tm_hour = 12;
  t->tm_isdst = -1;
  mktime(t);
}

// `want` NULL means: expect no countdown at all, and expect both buffers to be
// left exactly as the caller had them.
static void expect(const char *want, const char *want_short, int y, int mon,
                   int mday) {
  struct tm t;
  fill(&t, y, mon, mday);

  char buf[SGF_HOLI_BUF];
  char sbuf[SGF_HOLI_BUF];
  strcpy(buf, "UNTOUCHED");
  strcpy(sbuf, "UNTOUCHED");

  const bool got = sgf_format_holiday(buf, sizeof(buf), sbuf, sizeof(sbuf), &t);

  if (want == NULL) {
    if (got) {
      printf("  FAIL %04d-%02d-%02d: want silence, got \"%s\"\n", y, mon, mday,
             buf);
      s_failures++;
    } else if (strcmp(buf, "UNTOUCHED") != 0 ||
               strcmp(sbuf, "UNTOUCHED") != 0) {
      printf("  FAIL %04d-%02d-%02d: returned false but wrote \"%s\"/\"%s\"\n",
             y, mon, mday, buf, sbuf);
      s_failures++;
    } else {
      printf("  ok   %04d-%02d-%02d -> (no countdown)\n", y, mon, mday);
    }
    return;
  }

  if (!got) {
    printf("  FAIL %04d-%02d-%02d: want \"%s\", got no countdown\n", y, mon,
           mday, want);
    s_failures++;
  } else if (strcmp(buf, want) != 0 || strcmp(sbuf, want_short) != 0) {
    printf("  FAIL %04d-%02d-%02d: want \"%s\"/\"%s\", got \"%s\"/\"%s\"\n", y,
           mon, mday, want, want_short, buf, sbuf);
    s_failures++;
  } else {
    printf("  ok   %04d-%02d-%02d -> \"%s\" / \"%s\"\n", y, mon, mday, buf,
           sbuf);
  }
}

// The eleven gazetted dates of each year, as published by MOM. Kept separate
// from the table in holidays.c on purpose: a test that reads its expectations
// out of the code under test proves only that the code is self-consistent.
static const struct {
  int y, mon, mday;
  const char *name;
} s_gazetted[] = {
  { 2026,  1,  1, "NEW YEAR'S DAY" },
  { 2026,  2, 17, "CHINESE NEW YEAR" },
  { 2026,  3, 21, "HARI RAYA PUASA" },
  { 2026,  4,  3, "GOOD FRIDAY" },
  { 2026,  5,  1, "LABOUR DAY" },
  { 2026,  5, 27, "HARI RAYA HAJI" },
  { 2026,  5, 31, "VESAK DAY" },
  { 2026,  8,  9, "NATIONAL DAY" },
  { 2026, 11,  8, "DEEPAVALI" },
  { 2026, 12, 25, "CHRISTMAS DAY" },
  { 2027,  1,  1, "NEW YEAR'S DAY" },
  { 2027,  2,  6, "CHINESE NEW YEAR" },
  { 2027,  3, 10, "HARI RAYA PUASA" },
  { 2027,  3, 26, "GOOD FRIDAY" },
  { 2027,  5,  1, "LABOUR DAY" },
  { 2027,  5, 17, "HARI RAYA HAJI" },
  { 2027,  5, 20, "VESAK DAY" },
  { 2027,  8,  9, "NATIONAL DAY" },
  { 2027, 10, 28, "DEEPAVALI" },
  { 2027, 12, 25, "CHRISTMAS DAY" },
};

#define GAZETTED_COUNT ((int)(sizeof(s_gazetted) / sizeof(s_gazetted[0])))

static time_t stamp(int y, int mon, int mday) {
  struct tm t;
  fill(&t, y, mon, mday);
  return mktime(&t);
}

// Midday to midday, so a DST step on the host cannot round a day to 0 or 2.
// The rounding has to be away from zero on both sides: a cast truncates toward
// it, which turns yesterday into today and hides an off-by-one in exactly the
// direction this sweep exists to catch.
static int days_between(time_t from, time_t to) {
  const double days = difftime(to, from) / 86400.0;
  return (int)((days >= 0.0) ? (days + 0.5) : (days - 0.5));
}

// Every single day from before the table to well after it, with the expected
// answer derived independently -- from the gazetted list above, using the
// host's own mktime rather than the arithmetic under test. This is what
// actually pins days_from_civil: the sweep crosses every month end, two year
// ends and a leap-year February, and it catches the overlaps the spot checks
// have to know about in advance. Hari Raya Haji and Vesak Day fall four days
// apart in 2026 and three in 2027, and Christmas sits exactly seven days before
// New Year, so on a good many days two windows are open at once and only the
// nearer holiday may be named.
static void check_every_day(void) {
  const int span = days_between(stamp(2025, 11, 1), stamp(2028, 3, 1));
  int checked = 0;
  int bad = 0;

  for (int n = 0; n <= span; n++) {
    // Walk by incrementing tm_mday and letting mktime renormalise, which is how
    // the calendar rolls a month or a year over without any arithmetic here.
    struct tm t;
    fill(&t, 2025, 11, 1 + n);
    const time_t d = mktime(&t);

    // The nearest gazetted date not yet past, and how far off it is.
    int best = -1;
    int best_days = 0;
    for (int i = 0; i < GAZETTED_COUNT; i++) {
      const int days = days_between(
          d, stamp(s_gazetted[i].y, s_gazetted[i].mon, s_gazetted[i].mday));
      if (days >= 0 && (best < 0 || days < best_days)) {
        best = i;
        best_days = days;
      }
    }

    char want[SGF_HOLI_BUF];
    const bool want_any = (best >= 0) && (best_days <= SGF_HOLI_LEAD_DAYS);
    if (want_any) {
      if (best_days == 0) {
        snprintf(want, sizeof(want), "%s TODAY", s_gazetted[best].name);
      } else {
        snprintf(want, sizeof(want), "%s - %dD", s_gazetted[best].name,
                 best_days);
      }
    }

    char buf[SGF_HOLI_BUF];
    char sbuf[SGF_HOLI_BUF];
    const bool got =
        sgf_format_holiday(buf, sizeof(buf), sbuf, sizeof(sbuf), &t);

    checked++;
    if (got != want_any) {
      printf("  FAIL %04d-%02d-%02d: want %s, got %s\n", t.tm_year + 1900,
             t.tm_mon + 1, t.tm_mday, want_any ? want : "(no countdown)",
             got ? buf : "(no countdown)");
      s_failures++;
      bad++;
    } else if (got && strcmp(buf, want) != 0) {
      printf("  FAIL %04d-%02d-%02d: want \"%s\", got \"%s\"\n",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, want, buf);
      s_failures++;
      bad++;
    }
  }

  if (bad == 0) {
    printf("  ok   %d consecutive days agree with the gazetted list\n", checked);
  }
}

// Both model buffers have to hold the longest label either table can produce.
static void check_buffer(void) {
  struct tm t;
  fill(&t, 2026, 2, 17);  // Chinese New Year -- the longest name there is

  char buf[SGF_HOLI_BUF];
  char sbuf[SGF_HOLI_BUF];
  memset(buf, 'x', sizeof(buf));
  memset(sbuf, 'x', sizeof(sbuf));
  sgf_format_holiday(buf, sizeof(buf), sbuf, sizeof(sbuf), &t);

  const size_t len = strlen(buf);
  if (len + 1 > SGF_HOLI_BUF) {
    printf("  FAIL \"%s\" needs %zu bytes, SgfModel.holiday has %d\n", buf,
           len + 1, SGF_HOLI_BUF);
    s_failures++;
  } else {
    printf("  ok   longest label \"%s\" fits in %d bytes with %zu to spare\n",
           buf, SGF_HOLI_BUF, SGF_HOLI_BUF - (len + 1));
  }
}

int main(void) {
  printf("countdown -- the two edges of the seven-day window\n");
  expect(NULL, NULL, 2026, 8, 1);                              // 8 days out
  expect("NATIONAL DAY - 7D", "NATIONAL DAY - 7D", 2026, 8, 2);
  expect("NATIONAL DAY - 1D", "NATIONAL DAY - 1D", 2026, 8, 8);

  printf("\ncountdown -- the day itself is named, not counted\n");
  expect("NATIONAL DAY TODAY", "NATIONAL DAY TODAY", 2026, 8, 9);
  expect(NULL, NULL, 2026, 8, 10);  // and it is gone the morning after

  printf("\ncountdown -- the short form is what differs, never the count\n");
  expect("CHINESE NEW YEAR - 3D", "CNY - 3D", 2026, 2, 14);
  expect("HARI RAYA PUASA - 2D", "RAYA PUASA - 2D", 2026, 3, 19);
  expect("HARI RAYA HAJI - 5D", "RAYA HAJI - 5D", 2026, 5, 22);
  expect("VESAK DAY TODAY", "VESAK TODAY", 2026, 5, 31);

  printf("\ncountdown -- across a month end and a year end\n");
  expect("LABOUR DAY - 4D", "LABOUR DAY - 4D", 2026, 4, 27);
  expect("NEW YEAR'S DAY - 6D", "NEW YEAR - 6D", 2026, 12, 26);
  expect("NEW YEAR'S DAY - 1D", "NEW YEAR - 1D", 2026, 12, 31);

  printf("\ncountdown -- the nearer holiday wins when two windows overlap\n");
  // 25 Dec 2026 is Christmas Day and exactly seven days before New Year 2027.
  expect("CHRISTMAS DAY TODAY", "CHRISTMAS TODAY", 2026, 12, 25);

  printf("\ncountdown -- Hari Raya Haji and Vesak share a week in 2026\n");
  expect("HARI RAYA HAJI - 1D", "RAYA HAJI - 1D", 2026, 5, 26);
  // Vesak is four days out here, but Haji is today and today wins.
  expect("HARI RAYA HAJI TODAY", "RAYA HAJI TODAY", 2026, 5, 27);
  expect("VESAK DAY - 3D", "VESAK - 3D", 2026, 5, 28);
  expect("VESAK DAY - 1D", "VESAK - 1D", 2026, 5, 30);

  printf("\ncountdown -- leap-day arithmetic\n");
  expect("HARI RAYA PUASA - 7D", "RAYA PUASA - 7D", 2027, 3, 3);
  expect(NULL, NULL, 2024, 2, 29);  // before the table: silence, not a crash

  printf("\ncountdown -- the table ends where MOM's gazette does\n");
  expect("CHRISTMAS DAY TODAY", "CHRISTMAS TODAY", 2027, 12, 25);
  expect(NULL, NULL, 2027, 12, 26);
  expect(NULL, NULL, 2028, 1, 1);
  expect(NULL, NULL, 2030, 6, 15);

  printf("\ncountdown -- every day from before the table to after it\n");
  check_every_day();

  printf("\ncountdown -- the model's buffers are wide enough\n");
  check_buffer();

  printf("\n%s (%d failure%s)\n", s_failures ? "FAILED" : "PASSED", s_failures,
         s_failures == 1 ? "" : "s");
  return s_failures != 0;
}
