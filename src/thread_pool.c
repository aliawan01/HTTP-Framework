#include "http_util.h"
#include "thread_pool.h"


#include "http_request.h"

// NOTE(ali): Not pretty I know, but it does the job for now.
#if _WIN32
static DWORD WINAPI CreateResponse(LPVOID arguments) {
#else 
static void* CreateResponse(void* arguments) {
#endif
    ThreadArgs* args = (ThreadArgs*)arguments;
    ctx = args->ctx;
    WorkQueue* work_queue = args->work_queue;

    while (true) {
        ThreadSemaphore_Wait(&work_queue_semaphore);

        int current_count = work_queue->count; 
        int next_count = current_count - 1;
        Work work = work_queue->work[current_count];

        // TODO(ali): Remove InterlockedCompareExchange after we have abstracted it into AtomicCompareExchange MACRO!!! -> Must be a macro.
        /* if (InterlockedCompareExchange((volatile long*)&work_queue->count, next_count, current_count) == current_count) { */
        if (AtomicCompareExchange(&work_queue->count, &current_count, next_count)) {
            // NOTE: Making everything fresh for processing the new request.
            ArenaDealloc(ctx.recycle_arena);
            for (int i = 0; i < ArrayCount(ctx.scratch_pool); i++) {
                ArenaDealloc(ctx.scratch_pool[i]);
            }

            CreateHTTPResponseFunc(ctx, work.ssl);
        }
    }
}

ThreadPool HTTP_Thread_CreateThreadPool(Arena* arena, int count, WorkQueue* work_queue, uint64_t recycle_arena_size, uint64_t scratch_arena_size) {
    Thread* thread_array = PushArray(arena, Thread, count);
    ThreadArgs* args_array = PushArray(arena, ThreadArgs, count);

    ThreadSemaphore_Init(&work_queue_semaphore, 100);

    for (int i = 0; i < count; i++) {
        args_array[i].ctx = BaseThread_CreateThreadContext(arena, MB(20), MB(10));
        args_array[i].work_queue = work_queue,

        Thread_Create(&thread_array[i], CreateResponse, (void*)&args_array[i]);
    }

    ThreadPool thread_pool = {
        .threads = thread_array,
        .work_queue = work_queue,
        .thread_count = count
    };

    return thread_pool;
}

WorkQueue HTTP_Thread_CreateWorkQueue(Arena* arena, int max_count) {
    Work* work = PushArray(arena, Work, max_count);

    return (WorkQueue) {
        .work = work,
        .max_count = max_count,
        .count = -1
    };
}

bool HTTP_Thread_AddWorkToWorkQueue(WorkQueue* work_queue, Work work) {
    if (work_queue->count+1 > work_queue->max_count) {
        return false;
    }

    work_queue->count++;
    work_queue->work[work_queue->count] = work;

    return ThreadSemaphore_Increment(&work_queue_semaphore);
}
