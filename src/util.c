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
