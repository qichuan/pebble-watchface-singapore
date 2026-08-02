#include "holidays.h"

// The eleven public holidays of Singapore, as gazetted by the Ministry of
// Manpower. Four of them -- Chinese New Year, the two Hari Rayas and Deepavali
// -- follow lunar or lunisolar calendars and Vesak Day follows the Buddhist
// one, so none of them can be computed from the Gregorian date. A table is not
// a shortcut here; it is the only correct way to know these dates offline.

enum {
  H_NEW_YEAR,
  H_CNY,
  H_RAYA_PUASA,
  H_GOOD_FRIDAY,
  H_LABOUR,
  H_RAYA_HAJI,
  H_VESAK,
  H_NATIONAL,
  H_DEEPAVALI,
  H_CHRISTMAS,
  H_COUNT
};

// The long form is what the face shows wherever the row is wide enough for it.
static const char *const s_name[H_COUNT] = {
  "NEW YEAR'S DAY", "CHINESE NEW YEAR", "HARI RAYA PUASA", "GOOD FRIDAY",
  "LABOUR DAY",     "HARI RAYA HAJI",   "VESAK DAY",       "NATIONAL DAY",
  "DEEPAVALI",      "CHRISTMAS DAY",
};

// The fallback, for the narrowest rows. Only the four names that actually run
// long are shortened; the rest are already as short as they can honestly be,
// and a name is not improved by being clipped for symmetry's sake.
static const char *const s_abbr[H_COUNT] = {
  "NEW YEAR",   "CNY",        "RAYA PUASA", "GOOD FRIDAY",
  "LABOUR DAY", "RAYA HAJI",  "VESAK",      "NATIONAL DAY",
  "DEEPAVALI",  "CHRISTMAS",
};

typedef struct {
  int16_t year;
  uint8_t mon;  // 1-12
  uint8_t day;
  uint8_t id;
} SgfHol;

// Sorted by date. MOM gazettes two years ahead, and only gazetted dates are
// here -- a countdown to a date that later moves is worse than no countdown, so
// the face simply stops offering one after 25 December 2027 rather than guess.
//
// Chinese New Year runs two days; only the first is the countdown's target.
// The Monday that follows a Sunday holiday is deliberately absent: MOM makes it
// conditional on the reader's own rest day, which the watch cannot know.
static const SgfHol s_hol[] = {
  { 2026,  1,  1, H_NEW_YEAR },
  { 2026,  2, 17, H_CNY },
  { 2026,  3, 21, H_RAYA_PUASA },
  { 2026,  4,  3, H_GOOD_FRIDAY },
  { 2026,  5,  1, H_LABOUR },
  { 2026,  5, 27, H_RAYA_HAJI },
  { 2026,  5, 31, H_VESAK },
  { 2026,  8,  9, H_NATIONAL },
  { 2026, 11,  8, H_DEEPAVALI },
  { 2026, 12, 25, H_CHRISTMAS },
  { 2027,  1,  1, H_NEW_YEAR },
  { 2027,  2,  6, H_CNY },
  { 2027,  3, 10, H_RAYA_PUASA },
  { 2027,  3, 26, H_GOOD_FRIDAY },
  { 2027,  5,  1, H_LABOUR },
  { 2027,  5, 17, H_RAYA_HAJI },
  { 2027,  5, 20, H_VESAK },
  { 2027,  8,  9, H_NATIONAL },
  { 2027, 10, 28, H_DEEPAVALI },
  { 2027, 12, 25, H_CHRISTMAS },
};

#define SGF_HOL_COUNT ((int)(sizeof(s_hol) / sizeof(s_hol[0])))

// Days since 1970-01-01 for a proleptic Gregorian date, by Howard Hinnant's
// days_from_civil. Integer throughout and no mktime: mktime would drag in the
// host's timezone handling, and the difference of two of its results is not
// reliably a whole number of days across a DST boundary. Singapore has no DST,
// but the watch is not required to be there.
static int prv_days_from_civil(int y, int m, int d) {
  y -= (m <= 2);
  const int era = ((y >= 0) ? y : (y - 399)) / 400;
  const int yoe = y - (era * 400);                                // [0, 399]
  const int doy = (((153 * (m + ((m > 2) ? -3 : 9))) + 2) / 5) + d - 1;
  const int doe = (yoe * 365) + (yoe / 4) - (yoe / 100) + doy;    // [0, 146096]
  return (era * 146097) + doe - 719468;
}

// "NATIONAL DAY - 6D", or "NATIONAL DAY TODAY" on the day itself. A bare "0D"
// would read as a countdown that had not finished; the date row beside it
// already carries the date, so the word is the clearer mark.
static void prv_label(char *buf, size_t len, const char *name, int days) {
  if (days == 0) {
    snprintf(buf, len, "%s TODAY", name);
  } else {
    snprintf(buf, len, "%s - %dD", name, days);
  }
}

bool sgf_format_holiday(char *buf, size_t len, char *sbuf, size_t slen,
                        const struct tm *t) {
  const int today = prv_days_from_civil(t->tm_year + 1900, (t->tm_mon % 12) + 1,
                                        t->tm_mday);

  for (int i = 0; i < SGF_HOL_COUNT; i++) {
    const int days =
        prv_days_from_civil(s_hol[i].year, s_hol[i].mon, s_hol[i].day) - today;
    if (days < 0) {
      continue;
    }
    // The table is sorted, so the first entry not yet past is the next one.
    // Anything further out than the lead means there is nothing to say -- and
    // no later entry can be nearer, so the scan stops here either way.
    if (days > SGF_HOLI_LEAD_DAYS) {
      return false;
    }
    prv_label(buf, len, s_name[s_hol[i].id], days);
    prv_label(sbuf, slen, s_abbr[s_hol[i].id], days);
    return true;
  }

  // Past the end of the gazetted table.
  return false;
}
