#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

typedef struct {
    unsigned char* buffer;
    uint64_t buffer_size;
    uint64_t current_offset;
} Arena;

typedef struct {
    Arena* arena;
    uint64_t original_offset;
} Temp;

HTTPEXPORTFUNC void  ArenaInit(Arena* arena, uint64_t arena_size);
HTTPEXPORTFUNC void  ArenaDelete(Arena* arena);
HTTPEXPORTFUNC void* ArenaAllocAligned(Arena* arena, uintptr_t num_of_elem, uintptr_t elem_size, uintptr_t align_size);
HTTPEXPORTFUNC void  ArenaDealloc(Arena* arena);
// TODO: Need to replace this since it only works in the case that the allocation 
//       is at the top of the arena.
HTTPEXPORTFUNC bool  ArenaIncrementOffset(Arena* arena, uint64_t increment_value);
HTTPEXPORTFUNC Temp  TempBegin(Arena* arena);
HTTPEXPORTFUNC void  TempEnd(Temp temp);
HTTPEXPORTFUNC Temp  ScratchGetFree(Arena** arena_pool, int arena_pool_size, Arena** conflicting_arenas, int conflicting_num);
HTTPEXPORTFUNC void  DeleteScratch(Temp temp_scratch);

#define PushArrayAlign(arena, type, num, align) (type*)ArenaAllocAligned(arena, (num), sizeof(type), align)
#define PushArray(arena, type, num) (type*)ArenaAllocAligned(arena, (num), sizeof(type), _Alignof(type))
#define PushString(arena, num) (char*)ArenaAllocAligned(arena, (num), sizeof(char), _Alignof(char))
#define PushStruct(arena, type) (type*)ArenaAllocAligned(arena, 1, sizeof(type), _Alignof(type))

// Resize Arena?

#ifdef __cplusplus
}
#endif
