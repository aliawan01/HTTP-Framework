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

void ArenaFreeAll(Arena* arena) {
    arena->current_offset = 0;
}

void ArenaDelete(Arena* arena) {
    VirtualFree(arena->buffer, 0, MEM_RELEASE);
    arena->buffer_size = 0;
    arena->current_offset = 0;
    arena->initialized = false;
}

void ScratchBegin(Arena* arena) {
    arena->scratch_begin_offset = arena->current_offset;
}

void ScratchEnd(Arena* arena) {
    arena->current_offset = arena->scratch_begin_offset;
}

