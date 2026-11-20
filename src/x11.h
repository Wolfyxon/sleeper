#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

#include "global.h"

void x11_init(X11Context *ctx);
bool x11_has_interacted(X11Context *ctx);
