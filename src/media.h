#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <regex.h>

#include <glib-object.h>
#include <playerctl/playerctl.h>

bool has_blocking_media_player();
bool is_video_url(const char *url);

PlayerctlPlayer *get_blocking_playerctl();
bool is_blocking_playerctl(PlayerctlPlayer *player);
bool is_playerctl_playing(PlayerctlPlayer *player);
