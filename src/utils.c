#include "utils.h"

bool streq(const char* a, const char *b) {
    return strcmp(a, b) == 0;
}

bool strends(const char* str, const char *suffix) {
    if(str == NULL) {
        fprintf(stderr, "error: stdrends(): str is NULL\n");
        return false;
    }

    if(suffix == NULL) {
        fprintf(stderr, "error: stdrends(): suffix is NULL\n");
        return false;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if(suffix_len > str_len) {
        return false;
    }

    return streq(str + (str_len - suffix_len), suffix);
}

bool strstarts(const char *str, const char *prefix) {
    if(str == NULL) {
        fprintf(stderr, "error: strstarts(): str is NULL\n");
        return false;
    }

    if(prefix == NULL) {
        fprintf(stderr, "error: strstarts(): prefix is NULL\n");
        return false;
    }

    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool strisint(const char *str) {
    if(str == NULL) {
        fprintf(stderr, "error: strisint(): str is NULL");
        return false;
    }

    size_t len = strlen(str);

    if(len == 0) {
        return false;
    }

    for(size_t i = 0; i < len; i++) {
        if(!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

millis_t get_time_ms() {
    struct timeval time;
    gettimeofday(&time, NULL);
    
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}
