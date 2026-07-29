// Host-side stand-in for the SDK header, so the pure-C helpers in
// src/c/timeinfo.c can be compiled and tested off-watch. It provides only what
// those helpers actually use -- if a test ever needs more than this, that is a
// sign the code under test has stopped being pure logic.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
