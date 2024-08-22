#pragma once

typedef struct {
    unsigned char* buffer;
    uint64_t buffer_size;
    uint64_t current_offset;
    bool initialized;
} Arena;

typedef struct {
    Arena* arena;
    uint64_t original_offset;
} Temp;

void  ArenaInit(Arena* arena, uint64_t arena_size);
void  ArenaDelete(Arena* arena);
void* ArenaAlloc(Arena* arena, uint64_t allocation_size);
void  ArenaDealloc(Arena* arena);
// TODO: Need to replace this since it only works in the case that the allocation 
//       is at the top of the arena.
bool  ArenaIncrementOffset(Arena* arena, uint64_t increment_value);
Temp  TempBegin(Arena* arena);
void  TempEnd(Temp temp);
Temp  ScratchGetFree(Arena** arena_pool, int arena_pool_size, Arena** conflicting_arenas, int conflicting_num);
void  DeleteScratch(Temp temp_scratch);

// Resize Arena?

