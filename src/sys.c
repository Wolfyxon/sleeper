#include "sys.h"

void init_tty(AppContext *ctx) {
    ctx->tty_id = -1;

    if(ctx->no_tty_check) {
        return;
    }

    char *tty_str = getenv(TTY_ENV);

    if(tty_str == NULL) {
        fprintf(stderr, "error: Env variable '%s' not set. TTY check will not be available.\n", TTY_ENV);
        return;
    }

    if(!strisint(tty_str)) {
        fprintf(stderr, "error: Value of env variable '%s=%s' is not a valid number. TTY check will not be available.\n", TTY_ENV, tty_str);
        return;
    }
    
    ctx->tty_id = atoi(tty_str);
}

bool get_active_tty_name(char *buf, size_t buf_size) {
    FILE *f = fopen(ACTIVE_TTY_FILE, "r");

    if(f == NULL) {
        return false;
    }

    fread(buf, sizeof(char), buf_size - 1, f);
    buf[buf_size - 1] = '\0';

    fclose(f);
    
    return true;
}

bool is_active_tty(AppContext *ctx) {
    if(ctx->tty_id < 0) {
        return true;
    }
    
    char buf[16] = {0};

    if(!get_active_tty_name(buf, sizeof(buf))) {
        fprintf(stderr, "error: Unable to access '%s'. Assuming session's TTY is active.\n", ACTIVE_TTY_FILE);
        return true;
    }

    int current_id = -1;
    sscanf(buf, "tty%d", &current_id);

    if(current_id < 0) {
        fprintf(stderr, "error: Invalid TTY ID: %d. Assuming session's TTY is active.\n", current_id);
        return true;
    }
    
    return current_id == ctx->tty_id;
}
