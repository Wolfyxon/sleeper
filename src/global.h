#pragma once

#include <stdbool.h>
#include <X11/Xlib.h>

#include "utils.h"

typedef struct {
    Display *disp;
    struct timeval event_timeout_timer;
    fd_set event_fd;
    int socket;
} X11Context;

typedef struct {
    millis_t time;
    char *command;
    bool valid;
} SleepStage;

typedef struct {
    bool verbose;
    bool no_exec;
    bool no_video_check;
    bool no_lockscreen_check;
    bool no_screensaver_check;
    bool check_fullscreen;

    X11Context x11;
    millis_t last_interaction;
    SleepStage wakeup_stage;
    SleepStage *sleep_stages;
    size_t sleep_stages_len;
    SleepStage *current_stage;
    SleepStage *last_stage;
    
} AppContext;

