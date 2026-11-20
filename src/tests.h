#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils.h"
#include "media.h"

typedef bool test_result_t;
typedef test_result_t (*test_func_t)(void);

typedef struct {
    test_func_t func;
    const char *name;
} UnitTest;

extern UnitTest _tests[64];
extern size_t _tests_len;

// very readable
#define TEST(testname) \
    test_result_t _test_##testname(); \
    \
    __attribute__((constructor)) \
    void _register_test_##testname() { \
        _tests[_tests_len].func = _test_##testname; \
        _tests[_tests_len].name = #testname; \
        _tests_len++; \
    } \
    \
    test_result_t _test_##testname()

void run_tests();
