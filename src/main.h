#pragma once

#include <stdio.h>
#include <unistd.h>

#include "global.h"
#include "media.h"
#include "x11.h"

#ifdef RUN_TESTS
#include "tests.h"
#endif

#define FRAME_TIME (1000 * 100)

int main(int argc, char **argv);
void parse_args(AppContext *ctx, int argc, char **argv);
void process_stage_args(SleepStage *stage, size_t *i, int argc, char **argv);
void print_help();
void main_loop(AppContext *ctx);

void timing_out(AppContext *ctx);
void dry_interacted(AppContext *ctx);
void interacted(AppContext *ctx);

void exec_stage(AppContext *ctx, SleepStage *stage);
void add_sleep_stage(AppContext *ctx, SleepStage stage);
void init_default_sleep_stages(AppContext *ctx);
SleepStage *get_next_sleep_stage(AppContext *ctx);
int _sort_compare_stages(const void* a, const void* b);