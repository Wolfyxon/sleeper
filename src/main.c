#include "main.h"

int main(int argc, char **argv) {
    #ifdef RUN_TESTS
    run_tests();
    #endif

    AppContext ctx = {0};
    ctx.sleep_stages = malloc(0);
    ctx.last_interaction = get_time_ms();

    if(ctx.sleep_stages == NULL) {
        fprintf(stderr, "error: Failed to allocate memory for sleep stages\n");
        return 1;
    }

    parse_args(&ctx, argc, argv);

    if(ctx.sleep_stages_len == 0) {
        init_default_sleep_stages(&ctx);
    }

    qsort(ctx.sleep_stages, ctx.sleep_stages_len, sizeof(SleepStage), _sort_compare_stages);
    ctx.last_stage = &ctx.wakeup_stage;

    x11_init(&ctx.x11);

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
    puts("  --help:     Shows this help");
    puts("  --version:  Shows the program version");
    puts("  --no-exec:  Do not execute any commands, only say what would be done.");
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

        if(streq(arg, "--verbose")) {
            ctx->verbose = true;
            continue;
        }
        
        fprintf(stderr, "error: Unrecognized argument '%s'\n", arg);
        exit(1);
    }
}

void main_loop(AppContext *ctx) {
    if(x11_has_interacted(&ctx->x11)) {
        interacted(ctx);
        return;
    }

    millis_t now = get_time_ms();

    if(ctx->current_stage == NULL) {
        ctx->current_stage = get_next_sleep_stage(ctx);
    }

    if(ctx->current_stage == NULL) {
        return;
    }
    
    if(now >= ctx->last_interaction + ctx->current_stage->time && ctx->last_stage != ctx->current_stage) {
        timing_out(ctx);
    }
}

void timing_out(AppContext *ctx) {
    if(has_blocking_media_player()) {
        interacted(ctx);
        return;
    }

    exec_stage(ctx, ctx->current_stage);
    ctx->current_stage = get_next_sleep_stage(ctx);
}

void interacted(AppContext *ctx) {
    ctx->last_interaction = get_time_ms();

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

    ctx->sleep_stages[ctx->sleep_stages_len - 1] = stage;
}

void init_default_sleep_stages(AppContext *ctx) {
    add_sleep_stage(ctx, (SleepStage){.time = 1000});
    add_sleep_stage(ctx, (SleepStage){.time = 2000});
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
