#include "http_util.h"
#include "http_arena.h"

Arena permanent_arena;
Arena route_callback_arena;

Allocator allocator = {
    .permanent_arena = &permanent_arena,
    .route_callback_arena = &route_callback_arena,
};

HTTPDatabase http_database = {0};

_Thread_local ThreadContext ctx;

char* ConvertStrArrayToString(Arena* arena, StringArray string_array, char* separator) {
    char* converted_string = PushString(arena, string_array.count*100);

    for (int i = 0; i < string_array.count; i++) {
        strcat(converted_string, string_array.array[i]);
        if (i < string_array.count-1) {
            strcat(converted_string, separator);
        }
    }

    return converted_string;
}

ThreadContext HTTP_Thread_GetContext(void) {
    return ctx;
}

void HTTP_Log(enum HTTPLogType type, char* formatString, ...) {
    va_list args;
    va_start(args, formatString);

    Temp scratch = GetScratch(0, 0);

    char* formatStringWithColor = PushString(scratch.arena, strlen(formatString)+20);

    if (type == HTTP_ERROR) {
        strcat(formatStringWithColor, "\033[0;31m");
        strcat(formatStringWithColor, formatString);
        strcat(formatStringWithColor, "\033[0m");

        vfprintf(stderr, formatStringWithColor, args);
        DeleteScratch(scratch);
    }
    else if (type == HTTP_WARNING) {
        strcat(formatStringWithColor, "\033[0;33m");
    }
    else {
        strcat(formatStringWithColor, "\033[0;32m");
    }

    strcat(formatStringWithColor, formatString);
    strcat(formatStringWithColor, "\033[0m");
    vfprintf(stdout, formatStringWithColor, args);
    DeleteScratch(scratch);

    va_end(args);
}
