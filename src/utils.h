#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

typedef unsigned long millis_t;

bool streq(const char* a, const char *b);
bool strends(const char* str, const char *suffix);

millis_t get_time_ms();
