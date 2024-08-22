#include "util.h"
#include "arena.h"

void ArenaInit(Arena* arena, uint64_t arena_size) {
    // TODO: Probably best to reserve the virtual address space first then
    //       commit pages as necessary.
    arena->buffer = VirtualAlloc(0, arena_size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    Assert(arena->buffer != NULL);
    arena->buffer_size = arena_size;
    arena->current_offset = 0;
    arena->initialized = true;
}

void* ArenaAlloc(Arena* arena, uint64_t allocation_size) {
    if (!arena->initialized) {
        printf("Unitialized arena!");
        return NULL;
    }

    // TODO: Probably a better way to do this.
    if (arena->current_offset == 0 && allocation_size < arena->buffer_size) {
        arena->current_offset += allocation_size;
        memset(arena->buffer, 0, allocation_size);
        return (void*)(arena->buffer);
    }

    uint64_t word_size = sizeof(void*);
    uint64_t cur_offset = arena->current_offset;

    uint64_t gap_from_cur_allocation = arena->current_offset % word_size;
    if (gap_from_cur_allocation != 0) {
        gap_from_cur_allocation = word_size - gap_from_cur_allocation;
    }

    cur_offset += gap_from_cur_allocation;

    if (cur_offset + allocation_size > arena->buffer_size) {
        return NULL;
    }

    arena->current_offset += (gap_from_cur_allocation + allocation_size);
    memset(arena->buffer+cur_offset, 0, allocation_size);
    
    return (void*)(arena->buffer+cur_offset);
}

bool ArenaIncrementOffset(Arena* arena, uint64_t increment_value) {
    if (arena->current_offset + increment_value > arena->buffer_size) {
        return false;
    }

    memset(arena->buffer+arena->current_offset, 0, increment_value);
    arena->current_offset += increment_value;
    return true;
}

void ArenaDealloc(Arena* arena) {
    arena->current_offset = 0;
}

void ArenaDelete(Arena* arena) {
    VirtualFree(arena->buffer, 0, MEM_RELEASE);
    arena->buffer_size = 0;
    arena->current_offset = 0;
    arena->initialized = false;
}

Temp TempBegin(Arena* arena) {
    Temp temp_arena = { arena, arena->current_offset };
    return temp_arena;
}

void TempEnd(Temp temp) {
    temp.arena->current_offset = temp.original_offset;
}

Temp  ScratchGetFree(Arena** arena_pool, int arena_pool_num, Arena** conflicting_arenas, int conflicting_num) {
    bool is_conflicting_arena;
    for (int i = 0; i < arena_pool_num; i++) {
        is_conflicting_arena = false;
        for (int x = 0; x < conflicting_num; x++) {
            if (arena_pool[i] == conflicting_arenas[x]) {
                is_conflicting_arena = true;
                break;
            }
        }

        if (!is_conflicting_arena) {
            return TempBegin(arena_pool[i]);
        }
    }

    Assert(is_conflicting_arena != false);
    return (Temp){NULL, 20};
}

void DeleteScratch(Temp temp_scratch) {
    TempEnd(temp_scratch);
}
