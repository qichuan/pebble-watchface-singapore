#!/bin/sh
# Compiles the watch's pure-C helpers against a host stub of pebble.h and runs
# them. Nothing here touches the SDK or an emulator.
set -e

cd "$(dirname "$0")"
OUT=$(mktemp -d)
trap 'rm -rf "$OUT"' EXIT

cc -std=c11 -Wall -Wextra -Werror -I test \
   timeinfo_test.c ../watch/src/c/timeinfo.c \
   -o "$OUT/timeinfo_test"

cc -std=c11 -Wall -Wextra -Werror -I test \
   holidays_test.c ../watch/src/c/holidays.c \
   -o "$OUT/holidays_test"

"$OUT/timeinfo_test"
echo
"$OUT/holidays_test"
