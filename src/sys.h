#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "global.h"

#define TTY_ENV "XDG_VTNR"
#define ACTIVE_TTY_FILE "/sys/class/tty/tty0/active"

void init_tty(AppContext *ctx);
bool get_active_tty_name(char *buf, size_t buf_size);
bool is_active_tty(AppContext *ctx);
