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

typedef struct {
    Arena* permanent_arena;
    Arena* recycle_arena;
    Arena* scratch_pool[2];
    int scratch_arena_num;
} Allocator;

// TODO: Maybe? make sure this a global_variable after testing.
extern Allocator allocator;

#define GetScratch(conflicting_arenas, num) ScratchGetFree(allocator.scratch_pool, allocator.scratch_arena_num, conflicting_arenas, num)
