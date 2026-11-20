#include "tests.h"

UnitTest _tests[64] = {0};
size_t _tests_len = 0;

TEST(strends) {
    if(!strends("hello", "llo")) {
        puts("Should end");
        return false;
    }

    if(strends("hello", "hi")) {
        puts("Should not end");
        return false;
    }

    return true;
}

TEST(video_url) {
    const char *should[] = {
        "file://whatever/hi.mp4",
        "hello.webm",
        "https://www.youtube.com/watch?v=dQw4w9WgXcQ"
    };

    const char *shouldnt[] = {
        "file://aaaa.mp3",
        "https://wolfyxon.github.io/wee"
    };

    for(size_t i = 0; i < sizeof(should) / sizeof(char*); i++) {
        if(!is_video_url(should[i])) {
            printf("Should be a video URL: %s\n", should[i]);
            return false;
        }
    }

    for(size_t i = 0; i < sizeof(shouldnt) / sizeof(char*); i++) {
        if(is_video_url(shouldnt[i])) {
            printf("Shouldn't be a video URL: %s\n", should[i]);
            return false;
        }
    }

    return true;
}

void run_tests() {
    for(size_t i = 0; i < _tests_len; i++) {
        UnitTest *test = &_tests[i];

        printf("Running test '%s':\n", test->name);

        test_result_t res = test->func();

        if(res) {
            puts(": OK");
        } else {
            puts(": FAIL");
        }
    }

    exit(0);
}
