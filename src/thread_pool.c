#include "util.h"
#include "base_thread.h"
#include "thread_pool.h"

static DWORD WINAPI CreateResponse(LPVOID arguments) {
    ThreadArgs* args = (ThreadArgs*)arguments;
    ThreadContext ctx = args->ctx;
    WorkQueue* work_queue = args->work_queue;

    while (true) {
        Temp scratch = GetScratch(0, 0);

        WaitForSingleObjectEx(work_queue_semaphore, INFINITE, FALSE);
        int current_count = work_queue->count; 
        int next_count = current_count - 1;
        if (InterlockedCompareExchange((volatile long*)&work_queue->count, next_count, current_count) == current_count) {
            printf("\t[BEFORE] work_queue->count: %d\n", current_count);
            printf("\t[AFTER DECREMENT]: work_queue->count: %d\n", next_count);
            Assert(current_count - next_count == 1);

            ArenaDealloc(scratch.arena);
            char* thing = PushString(scratch.arena, 200);
            strcpy(thing, "good morning everyone");
            printf("[BEFORE] strlen(thing): %zd, thing: `%s`\n", strlen(thing), thing);
            ArenaDealloc(scratch.arena);
            PushString(scratch.arena, 100);
            thing[0] = 'g';
            thing[1] = 's';
            printf("[AFTER] strlen(thing): %zd, thing: `%s`\n", strlen(thing), thing);
            Assert(strlen(thing) == 2);

            printf("current_count: %d, next_count: %d, current_count buffer: `%s`\n", current_count, next_count, work_queue->work[next_count].buffer);
        }


    }

}

// TODO: Need to move this to the platform layer?
ThreadPool HTTP_Thread_CreateThreadPool(Arena* arena, int count, WorkQueue* work_queue, uint64_t recycle_arena_size, uint64_t scratch_arena_size) {
    HANDLE* thread_array = PushArray(arena, HANDLE, count);
    ThreadArgs* args_array = PushArray(arena, ThreadArgs, count);
    work_queue_semaphore = CreateSemaphoreA(NULL, 0, count, NULL);

    for (int i = 0; i < count; i++) {
        args_array[i].ctx = BaseThread_CreateThreadContext(arena, MB(20), MB(10));
        args_array[i].work_queue = work_queue;

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
        .max_count = max_count
    };
}

bool HTTP_Thread_AddWorkToWorkQueue(WorkQueue* work_queue, Work work) {
    if (work_queue->count+1 > work_queue->max_count) {
        return false;
    }

    if (work_queue_semaphore != 0) {
        ReleaseSemaphore(work_queue_semaphore, 1, NULL);
    }

    work_queue->work[work_queue->count] = work;
    work_queue->count++;

    return true;
}
