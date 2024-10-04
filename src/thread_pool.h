#pragma once

typedef struct {
    char* buffer;
    /* int max_buffer_size; // TODO: Do we really need this? */
    /* SOCKET* client_socket; */
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

// TODO: Where do we need arenas?
typedef struct {
    HANDLE* threads;
    int thread_count;
    WorkQueue* work_queue;
} ThreadPool;

static HANDLE work_queue_semaphore;

ThreadPool HTTP_Thread_CreateThreadPool(Arena* arena, int count, WorkQueue* work_queue, uint64_t recycle_arena_size, uint64_t scratch_arena_size);
WorkQueue  HTTP_Thread_CreateWorkQueue(Arena* arena, int max_count);
bool       HTTP_Thread_AddWorkToWorkQueue(WorkQueue* work_queue, Work work);
