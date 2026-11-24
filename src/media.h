#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <regex.h>

#include <glib-object.h>
#include <playerctl/playerctl.h>

#include "global.h"

bool has_blocking_media_player(AppContext *ctx);
bool is_video_url(const char *url);

PlayerctlPlayer *get_blocking_playerctl(AppContext *ctx);
bool is_blocking_playerctl(AppContext *ctx, PlayerctlPlayer *player);
bool is_playerctl_playing(PlayerctlPlayer *player);
