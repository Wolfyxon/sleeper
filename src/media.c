#include "media.h"

const char *VIDEO_PATTERNS[] = {
    ".*.mp4", ".*.mov", ".*.webm",
    ".*.mkv", ".*.mpv",".*.ogv",
    ".*.avi", ".*.flv", ".*.vob",
    "https://.*/watch.*",
    "https://.*netflix.com/.*",
    "https://.*hbomax.com/.*",
    "https://.*primevideo.com/.*",
    "https://.*hulu.com/.*",
    "https://.*cda.pl/.*",
    "https://.*twitch.tv/.*",
    "https://.*kick.com/.*",
    "https://.*crunchyroll.com/.*"
};

bool has_blocking_media_player() {
    PlayerctlPlayer *pctl_player = get_blocking_playerctl();

    if(pctl_player != NULL) {
        g_object_unref(pctl_player);
        return true;
    }

    return false;
}

bool is_video_url(const char *url) {
    for(size_t i = 0; i < (sizeof(VIDEO_PATTERNS) / sizeof(char*)); i++) {
        const char *pat = VIDEO_PATTERNS[i];
        
        regex_t regex;
        regcomp(&regex, pat, 0);

        int res = regexec(&regex, url, 0, NULL, 0);
        regfree(&regex);

        if(res == 0) {
            return true;
        }
    }

    return false;
}

PlayerctlPlayer *get_blocking_playerctl() {
    GError *list_err = NULL;
    GList *players = playerctl_list_players(&list_err);

    for(GList *i = players; i != NULL; i = i->next) {
        GError *get_err = NULL;

        PlayerctlPlayerName *name_obj = i->data;
        PlayerctlPlayer *player = playerctl_player_new_from_name(name_obj, &get_err);

        if(get_err != NULL) {
            fprintf(stderr, "error: Unable to get player '%s': %s\n", name_obj->name, get_err->message);
            g_free(get_err);
            continue;
        }

        if(player == NULL) {
            fprintf(stderr, "error: Player instance of '%s' is null\n", name_obj->name);
            continue;
        }

        if(is_blocking_playerctl(player)) {
            return player;
        }

        g_object_unref(player);
    }

    return NULL;
}

bool is_blocking_playerctl(PlayerctlPlayer *player) {
    if(player == NULL) {
        fprintf(stderr, "error: is_blocking_playerctl(): player is NULL\n");
        return false;
    }

    if(!is_playerctl_playing(player)) {
        return false;
    }

    GError *url_meta_err = NULL;
    gchar *url = playerctl_player_print_metadata_prop(player, "xesam:url", &url_meta_err);

    if(url_meta_err != NULL) {
        fprintf(stderr, "error: Unable to get URL from player\n");
        g_free(url_meta_err);
        return false;
    }

    if(url == NULL) {
        return false;
    }

    if(is_video_url(url)) {
        g_free(url);
        return true;
    }
    
    g_free(url);

    return false;
}

bool is_playerctl_playing(PlayerctlPlayer *player) {
    PlayerctlPlaybackStatus status;
    g_object_get(player, "playback-status", &status, NULL); // fix your api dammit
    
    return status == PLAYERCTL_PLAYBACK_STATUS_PLAYING;
}
