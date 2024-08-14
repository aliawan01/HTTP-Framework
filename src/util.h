#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <re.h>
#include <cJSON.h>

#include "platform.h"
#include "arena.h"

// Macros
#define internal static
#define global_variable static
#define local_persist static

// Byte Conversions
#define GB(bytes) (uint64_t)bytes*1024*1024*1024
#define MB(bytes) (uint64_t)bytes*1024*1024
#define KB(bytes) (uint64_t)bytes*1024

#define ArrayCount(array) sizeof(array)/sizeof(array[0])
#define Maximum(a, b) (a > b) ? a : b
#define Minimum(a, b) (a < b) ? a : b
#ifdef DEBUG_BUILD
#define Assert(condition)\
    if (!(condition)) { \
        fprintf(stderr, "Assertion in file: %s at line %d\n", __FILE__, __LINE__); \
        *(int*)0 = 0; \
    }           
#else
#define Assert(condition)

#endif


