#include "main.h"

int main(int argc, char **argv) {
    #ifdef RUN_TESTS
    run_tests();
    #endif

    AppContext ctx = {0};
    
    ctx.sleep_stages = malloc(0);
    ctx.last_interaction = get_time_ms();

    ctx.wakeup_stage = (SleepStage){
        .command = "brightnessctl --restore",
        .valid = false
    };

    if(ctx.sleep_stages == NULL) {
        fprintf(stderr, "error: Failed to allocate memory for sleep stages\n");
        return 1;
    }

    parse_args(&ctx, argc, argv);
    init_tty(&ctx);

    if(ctx.sleep_stages_len == 0) {
        printf("info: No sleep stages configured. Default will be used:\n");
        init_default_sleep_stages(&ctx);

        for(size_t i = 0; i < ctx.sleep_stages_len; i++) {
            SleepStage stage = ctx.sleep_stages[i];

            printf("  %lds: %s\n", stage.time / 1000, stage.command);
        }

        printf("  wakeup: %s\n\n", ctx.wakeup_stage.command);
    }

    check_duplicate_stages(&ctx);

    if(!ctx.wakeup_stage.valid) {
        fprintf(stderr, "warning: No wakeup stage configured (via --wakeup)! Default will be used:\n");
        fprintf(stderr, "%s\n", ctx.wakeup_stage.command);
        ctx.wakeup_stage.valid = true;
    }

    qsort(ctx.sleep_stages, ctx.sleep_stages_len, sizeof(SleepStage), _sort_compare_stages);
    ctx.last_stage = &ctx.wakeup_stage;

    x11_init(&ctx.x11);

    if(ctx.verbose) {
        puts("info: Running");
    }

    while(1) {
        main_loop(&ctx);
        usleep(FRAME_TIME);
    }

    return 0;
}

void print_help() {
    puts("Usage: sleeper [OPTION]...");
    puts("");
    puts("Available options:");
    puts("  --help:                                 Shows this help.");
    puts("  --version:                              Shows the program version.");
    puts("  --no-exec:                              Do not execute any commands, only say what would be done.");
    puts("  --no-video-check:                       Do not check for active video players (via playerctl)");
    puts("  --no-lockscreen-check:                  Do not check for an active lockscreen app");
    puts("  --no-screensaver-check:                 Do not check for an active screensaver app");
    puts("  --no-tty-check:                         Do not check if switched to console mode or other graphical session.");
    puts("  --check-fullscreen:                     Check for fullscreen apps.");
    puts("  --check-command <command>:              Check a custom command. Use exit code other than 0 to prevent sleeping.");
    puts("  --media-url-regex: <regex>              Custom regex pattern for checking playerctl URLs.");
    puts("  --stage: <seconds> <STAGE OPTION>...  : Adds a sleep stage.");
    puts("  --wakeup: <seconds> <STAGE OPTION>... : Adds a wakeup stage.");
    puts("");
    puts("Stage options (for --stage and --wakeup)");
    puts("  --command: <command>: Command to execute");
    puts("");
    puts("Example usage:");
    puts("  sleeper \\");
    puts("  --stage 60 --command 'backlightctl s 1%'");
    puts("  --stage 120 --command 'your-lockscreen-command'");
    puts("  --media-url-regex 'https://example.com/.*|https://example.org/.*'");
    puts("");
    puts("Source code and bugs:");
    puts("https://github.com/Wolfyxon/sleeper");
}

void parse_args(AppContext *ctx, int argc, char **argv) {
    for(size_t i = 1; i < argc; i++) {
        char *arg = argv[i];

        if(streq(arg, "--help")) {
            print_help();
            exit(0);
        }

        if(streq(arg, "--version")) {
            printf("sleeper v%s\n", PROGRAM_VERSION); // it's not a double, shut up clangd
            exit(0);
        }

        if(streq(arg, "--no-exec")) {
            ctx->no_exec = true;
            continue;
        }

        if(streq(arg, "--no-video-check")) {
            ctx->no_video_check = true;
            continue;
        }

        if(streq(arg, "--no-lockscreen-check")) {
            ctx->no_lockscreen_check = true;
            continue;
        }

        if(streq(arg, "--no-screensaver-check")) {
            ctx->no_screensaver_check = true;
            continue;
        }

        if(streq(arg, "--no-tty-check")) {
            ctx->no_tty_check = true;
            continue;
        }

        if(streq(arg, "--check-fullscreen")) {
            ctx->check_fullscreen = true;
            continue;
        }

        if(streq(arg, "--check-command")) {
            if(argc <= i + 1) {
                fprintf(stderr, "error: --check-command requires an argument\n");
                exit(1);
            }

            if(ctx->check_command != NULL) {
                fprintf(stderr, "error: Custom check command is already set\n");
                exit(1);
            }

            ctx->check_command = argv[i + 1];

            i++;
            continue;
        }

        if(streq(arg, "--media-url-regex")) {
            if(argc <= i + 1) {
                fprintf(stderr, "error: --media-url-regex requires an argument\n");
                exit(1);
            }

            char *pat = argv[i + 1];
            int res = regcomp(&ctx->playerctl_url_regex, pat, 0);
            
            if(res != REG_NOERROR) {
                char err_str[256] = {0};

                regerror(res, &ctx->playerctl_url_regex, err_str, sizeof(err_str));
                err_str[sizeof(err_str) - 1] = '\0';

                fprintf(stderr, "error: --media-url-regex: %s\n", err_str);
                exit(1);
            }

            ctx->has_playerctl_url_regex = true;

            i++;
            continue;
        }

        if(streq(arg, "--verbose")) {
            ctx->verbose = true;
            continue;
        }

        if(streq(arg, "--stage")) {
            SleepStage new_stage = {0};

            if(argc <= i + 3) {
                fprintf(stderr, "error: --stage requires arguments: --stage <seconds> --command <command>\n");
                exit(1);
            }

            char *time_str = argv[i + 1];
            i += 1;

            if(!strisint(time_str)) {
                fprintf(stderr, "error: Not a valid number for --stage: '%s'\n", time_str);
                exit(1);
            }

            int seconds = atoi(time_str);
            
            if(seconds <= 0) {
                fprintf(stderr, "error: Seconds value for --stage must be greater than 0\n");
                exit(1);
            }

            new_stage.time = seconds * 1000;
            process_stage_args(&new_stage, &i, argc, argv);
            add_sleep_stage(ctx, new_stage);
            continue;
        }

        if(streq(arg, "--wakeup")) {
            SleepStage wakeup_stage = {0};
            wakeup_stage.valid = true;

            process_stage_args(&wakeup_stage, &i, argc, argv);
            ctx->wakeup_stage = wakeup_stage;
            continue;
        }
        
        fprintf(stderr, "error: Unrecognized argument '%s'\n", arg);
        exit(1);
    }
}

void process_stage_args(SleepStage *stage, size_t *i_ptr, int argc, char **argv) {
    if(argc <=  *i_ptr + 2) {
        fprintf(stderr, "error: Stage options are required: --command <command>\n");
        exit(1);
    }

    char *flag = argv[*i_ptr + 1];
    char *command = argv[*i_ptr + 2];
    
    if(!streq(flag, "--command")) {
        fprintf(stderr, "error: Unrecognized argument '%s'\n", flag);
        exit(1);
    }

    stage->command = command;
    
    *i_ptr += 2;
}

void main_loop(AppContext *ctx) {
    if(!ctx->no_tty_check && !is_active_tty(ctx)) {
        dry_interacted(ctx);
        return;
    }
    
    if(x11_has_interacted(&ctx->x11)) {
        interacted(ctx);
        return;
    }

    if(ctx->current_stage == NULL) {
        ctx->current_stage = get_next_sleep_stage(ctx);
    }

    if(ctx->current_stage == NULL) {
        return;
    }
    
    millis_t now = get_time_ms();

    if(now >= ctx->last_interaction + ctx->current_stage->time && ctx->last_stage != ctx->current_stage) {
        timing_out(ctx);
    }
}

bool check_custom_command(AppContext *ctx) {
    if(ctx->check_command == NULL) {
        return false;
    }

    if(ctx->check_command[0] == '\0') {
        return false;
    }

    if(ctx->verbose) {
        printf("info: Executing %s\n", ctx->check_command);
    }

    int code = system(ctx->check_command);

    if(ctx->verbose) {
        printf("info: Exit code: %d. ", code);

        if(code == 0) {
            printf("Blocking sleep.\n");
        } else {
            printf("Not blocking sleep.\n");
        }
    }

    return code == 0;
}

void timing_out(AppContext *ctx) {
    if(x11_is_screen_off(&ctx->x11)) {
        dry_interacted(ctx);
        return;
    }

    if(has_blocking_media_player(ctx)) {
        dry_interacted(ctx);
        return;
    }

    if(!ctx->no_screensaver_check && x11_has_screensaver(&ctx->x11)) {
        dry_interacted(ctx);
        return;
    }

    if(!ctx->no_lockscreen_check && x11_has_lockscreen(&ctx->x11)) {
        dry_interacted(ctx);
        return;
    }

    if(ctx->check_fullscreen && x11_has_fullscreen(&ctx->x11)) {
        dry_interacted(ctx);
        return;
    }

    if(check_custom_command(ctx)) {
        dry_interacted(ctx);
        return;
    }

    exec_stage(ctx, ctx->current_stage);
    ctx->current_stage = get_next_sleep_stage(ctx);
}

void dry_interacted(AppContext *ctx) {
    ctx->last_interaction = get_time_ms();
}

void interacted(AppContext *ctx) {
    dry_interacted(ctx);

    if(ctx->last_stage != &ctx->wakeup_stage) {
        if(ctx->verbose) {
            puts("Wakeup");
        }

        exec_stage(ctx, &ctx->wakeup_stage);
        ctx->current_stage = NULL;
    }
}

void exec_stage(AppContext *ctx, SleepStage *stage) {
    ctx->last_stage = stage;

    if(ctx->no_exec || ctx->verbose) {
        printf("Executing stage of time %ld command: %s\n", stage->time, stage->command);
    }

    if(ctx->no_exec) {
        return;
    }

    system(stage->command);
}

void add_sleep_stage(AppContext *ctx, SleepStage stage) {
    ctx->sleep_stages = realloc(ctx->sleep_stages, sizeof(SleepStage) * ++ctx->sleep_stages_len);

    if(ctx->sleep_stages == NULL) {
        fprintf(stderr, "error: Failed to resize sleep stages buffer\n");
        exit(1);
    }

    stage.valid = true;
    ctx->sleep_stages[ctx->sleep_stages_len - 1] = stage;
}

void init_default_sleep_stages(AppContext *ctx) {
    add_sleep_stage(ctx, (SleepStage){
        .command = "brightnessctl --save set 1%",
        .time = 1000 * 60 * 1
    });

    add_sleep_stage(ctx, (SleepStage){
        .command = "xset s activate",
        .time = 1000 * 60 * 2
    });
    
    ctx->wakeup_stage.valid = true;
}

void check_duplicate_stages(AppContext *ctx) {
    for(size_t i = 0; i < ctx->sleep_stages_len; i++) {
        for(size_t j = 0; j < ctx->sleep_stages_len; j++) {
            if(i == j) {
                continue;
            }

            if(ctx->sleep_stages[i].time == ctx->sleep_stages[j].time) {
                fprintf(
                    stderr, 
                    "error: Multiple stages cannot run at the same time. Use 'command1 && command2' if you want to chain commands.\n"
                );
                exit(1);
            }
        }   
    }
}

SleepStage *get_next_sleep_stage(AppContext *ctx) {
    millis_t now = get_time_ms();

    for(size_t i = 0; i < ctx->sleep_stages_len; i++) {
        SleepStage *stage = &ctx->sleep_stages[i];
        
        if(now < ctx->last_interaction + stage->time) {
            return stage;
        }
    }

    return NULL;
}

int _sort_compare_stages(const void* a, const void* b) {
    SleepStage *stage_a = ( (SleepStage*) a );
    SleepStage *stage_b = ( (SleepStage*) b );

    if(stage_a->time == stage_b->time) {
        return 0;
    }

    if(stage_a->time < stage_b->time) {
        return -1;
    }

    return 1;
}
