#include "util.h"
#include "arena.h"

Arena permanent_arena;
Arena recycle_arena;
Arena route_callback_arena;
Arena scratch_pool[2];

Allocator allocator = {
    .permanent_arena = &permanent_arena,
    .recycle_arena = &recycle_arena,
    .route_callback_arena = &route_callback_arena,
    .scratch_pool = { &scratch_pool[0], &scratch_pool[1] },
    .scratch_arena_num = 2
};

HTTPDatabase http_database = {0};

ThreadContext ctx;

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
