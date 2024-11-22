#pragma once

typedef struct {
    SSL* ssl;
} Work;

typedef struct {
    Work* work;
    int   count;
    int   max_count;
} WorkQueue;

typedef struct {
    WorkQueue* work_queue; 
    ThreadContext ctx;
} ThreadArgs;

typedef struct {
    HANDLE* threads;
    int thread_count;
    WorkQueue* work_queue;
} ThreadPool;

static HANDLE work_queue_semaphore;
// TODO: Remove this after debugging.
static ThreadContext* context_array;

ThreadPool HTTP_Thread_CreateThreadPool(Arena* arena, int count, WorkQueue* work_queue, uint64_t recycle_arena_size, uint64_t scratch_arena_size);
WorkQueue  HTTP_Thread_CreateWorkQueue(Arena* arena, int max_count);
bool       HTTP_Thread_AddWorkToWorkQueue(WorkQueue* work_queue, Work work);
