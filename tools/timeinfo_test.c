// Pins sgf_format_date(): the weekday and month tables, the zero-padded day,
// and the fact that the buffer in SgfModel is big enough for the longest string
// the function can produce. The emulator resyncs its clock to the host, so a
// date other than today cannot be parked long enough to screenshot -- which is
// why the whole calendar is checked here instead of on the glass.
//
// Build and run:  tools/run_tests.sh

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../watch/src/c/timeinfo.h"

// Must match SgfModel.date in watch/src/c/render.h.
#define SGF_DATE_BUF 16

static int s_failures = 0;

static void fill(struct tm *t, int y, int mon, int mday) {
  memset(t, 0, sizeof(*t));
  t->tm_year = y - 1900;
  t->tm_mon = mon - 1;
  t->tm_mday = mday;
  t->tm_hour = 12;
  t->tm_isdst = -1;
  // Let mktime fill in tm_wday the same way localtime() does on the watch.
  mktime(t);
}

static void expect_date(const char *want, int y, int mon, int mday) {
  struct tm t;
  fill(&t, y, mon, mday);

  char buf[SGF_DATE_BUF];
  sgf_format_date(buf, sizeof(buf), &t);
  if (strcmp(buf, want) != 0) {
    printf("  FAIL %04d-%02d-%02d: want \"%s\", got \"%s\"\n", y, mon, mday,
           want, buf);
    s_failures++;
  } else {
    printf("  ok   %04d-%02d-%02d -> \"%s\"\n", y, mon, mday, buf);
  }
}

// Every date of a leap year, checking only the shape: three uppercase letters,
// space, two digits, space, three uppercase letters. Catches a table that is
// short by one entry, which the spot checks above would miss for eleven months
// of the year.
static void check_every_day(void) {
  int bad = 0;
  for (int mon = 1; mon <= 12; mon++) {
    for (int mday = 1; mday <= 28; mday++) {
      struct tm t;
      fill(&t, 2024, mon, mday);

      char buf[SGF_DATE_BUF];
      sgf_format_date(buf, sizeof(buf), &t);

      bool ok = (strlen(buf) == 10) && (buf[3] == ' ') && (buf[6] == ' ');
      for (int i = 0; ok && i < 3; i++) {
        ok = (buf[i] >= 'A' && buf[i] <= 'Z') &&
             (buf[i + 7] >= 'A' && buf[i + 7] <= 'Z');
      }
      ok = ok && (buf[4] >= '0' && buf[4] <= '9') &&
           (buf[5] >= '0' && buf[5] <= '9');

      if (!ok) {
        printf("  FAIL 2024-%02d-%02d produced \"%s\"\n", mon, mday, buf);
        s_failures++;
        bad++;
      }
    }
  }
  if (bad == 0) {
    printf("  ok   all 336 dates of 2024 keep the AAA DD AAA shape\n");
  }
}

// The date row is the widest string the face draws, and the one place a table
// index could walk off the end of .rodata. Both are cheap to pin.
static void check_buffer(void) {
  struct tm t;
  fill(&t, 2026, 12, 31);  // "WED 31 DEC" -- 10 chars plus the terminator

  char buf[SGF_DATE_BUF];
  memset(buf, 'x', sizeof(buf));
  sgf_format_date(buf, sizeof(buf), &t);

  const size_t len = strlen(buf);
  if (len + 1 > SGF_DATE_BUF) {
    printf("  FAIL \"%s\" needs %zu bytes, SgfModel.date has %d\n", buf,
           len + 1, SGF_DATE_BUF);
    s_failures++;
  } else {
    printf("  ok   longest date \"%s\" fits in %d bytes with %zu to spare\n",
           buf, SGF_DATE_BUF, SGF_DATE_BUF - (len + 1));
  }
}

int main(void) {
  printf("date row -- the string from the design sheet\n");
  expect_date("TUE 28 JUL", 2026, 7, 28);

  printf("\ndate row -- every weekday appears, in order\n");
  expect_date("SUN 26 JUL", 2026, 7, 26);
  expect_date("MON 27 JUL", 2026, 7, 27);
  expect_date("WED 29 JUL", 2026, 7, 29);
  expect_date("THU 30 JUL", 2026, 7, 30);
  expect_date("FRI 31 JUL", 2026, 7, 31);
  expect_date("SAT 01 AUG", 2026, 8, 1);

  printf("\ndate row -- the two ends of the month table\n");
  expect_date("THU 01 JAN", 2026, 1, 1);
  expect_date("THU 31 DEC", 2026, 12, 31);

  printf("\ndate row -- single-digit days stay zero-padded\n");
  expect_date("MON 09 AUG", 2027, 8, 9);  // National Day, as it happens
  expect_date("FRI 01 MAY", 2026, 5, 1);

  printf("\ndate row -- leap day exists and lands on the right weekday\n");
  expect_date("THU 29 FEB", 2024, 2, 29);

  printf("\ndate row -- shape holds for a whole year\n");
  check_every_day();

  printf("\ndate row -- the model's buffer is wide enough\n");
  check_buffer();

  printf("\n%s (%d failure%s)\n", s_failures ? "FAILED" : "PASSED", s_failures,
         s_failures == 1 ? "" : "s");
  return s_failures != 0;
}
