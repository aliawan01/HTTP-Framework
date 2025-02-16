#include "http_util.h"
#include "http_arena.h"

void ArenaInit(Arena* arena, uint64_t arena_size) {
    // TODO: Probably best to reserve the virtual address space first then
    //       commit pages as necessary.
    arena->buffer = MemoryAlloc(arena_size);
    Assert(arena->buffer != NULL);
    arena->buffer_size = arena_size;
    arena->current_offset = 0;
}

void* ArenaAllocAligned(Arena* arena, uintptr_t num_of_elem, uintptr_t elem_size, uintptr_t align_size) {
    Assert(IsPowerOfTwo(align_size));
    
    uintptr_t allocation_size = num_of_elem * elem_size;

    if (allocation_size < elem_size) {
        return NULL;
    }
    
    uintptr_t cur_offset = (uintptr_t)arena->current_offset + (uintptr_t)arena->buffer;
    uintptr_t padding = (~cur_offset+1) & (align_size-1);
    
    cur_offset += padding;
    
    if (cur_offset + allocation_size > (uintptr_t)arena->buffer_size + (uintptr_t)arena->buffer) {
        return NULL;
    }
    
    arena->current_offset += (padding + allocation_size);
    memset((void*)cur_offset, 0, allocation_size);
    
    return (void*)cur_offset;
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
    MemoryFree(arena->buffer, arena->buffer_size);
    arena->buffer_size = 0;
    arena->current_offset = 0;
}

Temp TempBegin(Arena* arena) {
    Temp temp_arena = { arena, arena->current_offset };
    return temp_arena;
}

void TempEnd(Temp temp) {
    temp.arena->current_offset = temp.original_offset;
}

Temp ScratchGetFree(Arena** arena_pool, int arena_pool_num, Arena** conflicting_arenas, int conflicting_num) {
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
