#pragma once

#include "http_platform.h"
#include "http_arena.h"

typedef struct {
    char* string;
    uint64_t count;
} String;

typedef struct {
    char** array;
    int count;
} StringArray;

static bool IsPowerOfTwo(uint64_t num) {
    return (num != 0) && (num & (num-1)) == 0;
}

#include "http_cjson_helper.h"

// Macros
#define internal static
#define global_variable static
#define local_persist static

// Byte Conversions
#define GB(bytes) (uint64_t)bytes*1024*1024*1024
#define MB(bytes) (uint64_t)bytes*1024*1024
#define KB(bytes) (uint64_t)bytes*1024

#include "base_thread.h"

#define ArrayCount(array) sizeof(array)/sizeof(array[0])
#define Maximum(a, b) (a > b) ? a : b
#define Minimum(a, b) (a < b) ? a : b

char* ConvertStrArrayToString(Arena* arena, StringArray string_array, char* separator);
#define StrLit(string) (String) {(char*)string, sizeof(string)-1}
#define HTTP_StrLit(string) StrLit(string)
// NOTE: Pass in an list of strings like "foo", "bar", "baz" 
#define StrArrayLit(...) (StringArray) {(char*[]){__VA_ARGS__}, sizeof((char*[]){__VA_ARGS__})/sizeof((char*[]){__VA_ARGS__}[0])}

typedef struct {
    Arena* permanent_arena;
    Arena* route_callback_arena;
} Allocator;

typedef struct {
    sqlite3* database;
    bool     initialized;
} HTTPDatabase;

// TODO: Maybe? make sure this a global_variable after testing.
extern HTTPEXPORTFUNC Allocator allocator;
extern HTTPEXPORTFUNC HTTPDatabase http_database;
extern HTTPEXPORTFUNC _Thread_local ThreadContext ctx;
