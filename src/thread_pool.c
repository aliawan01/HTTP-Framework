#include "util.h"
#include "thread_pool.h"
#include "http_request.h"


static DWORD WINAPI CreateResponse(LPVOID arguments) {
    ThreadArgs* args = (ThreadArgs*)arguments;
    ctx = args->ctx;
    WorkQueue* work_queue = args->work_queue;

    while (true) {
        WaitForSingleObjectEx(work_queue_semaphore, INFINITE, FALSE);

        int current_count = work_queue->count; 
        int next_count = current_count - 1;
        Work work = work_queue->work[current_count];

        if (InterlockedCompareExchange((volatile long*)&work_queue->count, next_count, current_count) == current_count) {
            // NOTE: Making everything fresh for processing the new request.
            ArenaDealloc(ctx.recycle_arena);
            for (int i = 0; i < ArrayCount(ctx.scratch_pool); i++) {
                ArenaDealloc(ctx.scratch_pool[i]);
            }

            CreateHTTPResponseFunc(ctx, work.ssl);
        }

    }

}

// TODO: Need to move this to the platform layer?
ThreadPool HTTP_Thread_CreateThreadPool(Arena* arena, int count, WorkQueue* work_queue, uint64_t recycle_arena_size, uint64_t scratch_arena_size) {
    HANDLE* thread_array = PushArray(arena, HANDLE, count);
    ThreadArgs* args_array = PushArray(arena, ThreadArgs, count);
    context_array = PushArray(arena, ThreadContext, count);

    work_queue_semaphore = CreateSemaphoreA(NULL, 0, 100000, NULL);

    for (int i = 0; i < count; i++) {
        context_array[i] =  BaseThread_CreateThreadContext(arena, MB(20), MB(10));
        /* TODO: Should just be:
           
            args_array[i].ctx = BaseThread_CreateThreadContext(arena, MB(20), MB(10));

            And remove context_array from thread_pool.h
        */

        args_array[i].ctx = context_array[i];
        args_array[i].work_queue = work_queue,

        thread_array[i] = CreateThread(NULL, 0, CreateResponse, (void*)&args_array[i], 0, NULL);
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

    if (work_queue_semaphore != 0) {
        ReleaseSemaphore(work_queue_semaphore, 1, NULL);
        return true;
    }
    else {
        return false;
    }
}
