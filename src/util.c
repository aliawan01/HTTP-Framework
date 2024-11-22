#include "util.h"
#include "arena.h"

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
