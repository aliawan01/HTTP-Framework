#pragma once

typedef struct Arena {
    unsigned char* buffer;
    uint64_t buffer_size;
    uint64_t current_offset;
    uint64_t scratch_begin_offset;
    bool initialized;
} Arena;

void ArenaInit(Arena* arena, uint64_t arena_size);
void* ArenaAlloc(Arena* arena, uint64_t allocation_size);
void ArenaFreeAll(Arena* arena);
void ArenaDelete(Arena* arena);
void ScratchBegin(Arena* arena);
void ScratchEnd(Arena* arena);

// Resize Arena?

