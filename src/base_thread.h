#pragma once

static int  thread_id_index;

typedef struct {
    int thread_id;
    Arena* recycle_arena;
    Arena* scratch_pool[2];
} ThreadContext;

ThreadContext BaseThread_CreateThreadContext(Arena* arena, uint64_t recycle_arena_size, uint64_t scratch_arena_size);

#define GetScratch(conflicting_arenas, num) ScratchGetFree(ctx.scratch_pool, 2, conflicting_arenas, num)
