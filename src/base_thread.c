#include "platform.h"
#include "arena.h"
#include "base_thread.h"

#define ArrayCount(array) sizeof(array)/sizeof(array[0])
#define MB(bytes) (uint64_t)bytes*1024*1024

ThreadContext BaseThread_CreateThreadContext(Arena* arena, uint64_t recycle_arena_size, uint64_t scratch_arena_size) {
    ThreadContext ctx = { .thread_id = thread_id_index };
    printf("Created thread with thread_id: %d\n", thread_id_index);

    ThreadReadWriteLock_Initialize(&ctx.global_route_callback_array_shared_mutex);
    ThreadReadWriteLock_Initialize(&ctx.error_page_shared_mutex);
    
    thread_id_index++;
    ctx.recycle_arena = PushStruct(arena, Arena);
    ArenaInit(ctx.recycle_arena, recycle_arena_size);

    for (int i = 0; i < ArrayCount(ctx.scratch_pool); i++) {
        ctx.scratch_pool[i] = PushStruct(arena, Arena);
        ArenaInit(ctx.scratch_pool[i], scratch_arena_size);
    }

    return ctx;
}

