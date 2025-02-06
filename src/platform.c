#include "http_platform.h"

/* #if SOMETHING */
// TODO(ali): Remove this after making it work on linux.
#ifdef _WIN32

enum HTTPCreateDirStatus HTTP_CreateDir(char* file_path) {
    if (file_path == NULL) { 
        return GIVEN_INVALID_FILE_PATH;
    }

    if (!CreateDirectoryA(file_path, NULL)) {
        switch (GetLastError()) {
            case ERROR_ALREADY_EXISTS:
                return DIR_ALREADY_EXISTS;
                break;
            case ERROR_PATH_NOT_FOUND:
                return PATH_NOT_FOUND;
                break;
            default:
                return OTHER_ERROR;
        }
    }
    else {
        return SUCCESS;
    }
}

bool HTTP_DeleteDirRecursive(char* dir_name) {
    bool success = false;
    char dir_name_double_null_terminated[512] = {0};
    memcpy(dir_name_double_null_terminated, dir_name, strlen(dir_name)+1);

    SHFILEOPSTRUCTA delete_dir = {
        .hwnd = NULL,
        .wFunc = FO_DELETE,
        .pFrom = dir_name_double_null_terminated,
        .pTo = NULL,
        .fFlags = FOF_NOCONFIRMATION|FOF_SILENT|FOF_NOERRORUI,
        .fAnyOperationsAborted = false,
        .hNameMappings = NULL,
        .lpszProgressTitle = NULL
    };

    
    if (!SHFileOperationA(&delete_dir)) {
        printf("[INFO] HTTP_DeleteDirRecursive() Successfully deleted directory with name: `%s`\n", dir_name);
        success = true;
    }
    else {
        printf("[ERROR] HTTP_DeleteDirRecursive() Unable to delete directory with name: `%s`\n", dir_name);
    }

    return success;
}

void HTTP_Gen256ByteRandomNum(char* buffer, int buffer_count) {
    Assert(buffer_count > 257);

    int random_num;
    for (int i = 0; i < 256; i += 4) {
        rand_s(&random_num);
        snprintf(buffer+i, 5, "%d", abs(random_num));
    }

}

void ThreadReadWriteLock_Initialize(ThreadReadWriteLock* lock) {
    InitializeSRWLock(lock);
}

void ThreadReadWriteLock_AcquireExclusiveLock(ThreadReadWriteLock* lock) {
    AcquireSRWLockExclusive(lock);
}

void ThreadReadWriteLock_ReleaseExclusiveLock(ThreadReadWriteLock* lock) {
    ReleaseSRWLockExclusive(lock);
}

void ThreadReadWriteLock_AcquireSharedLock(ThreadReadWriteLock* lock) {
    AcquireSRWLockShared(lock);
}

void ThreadReadWriteLock_ReleaseSharedLock(ThreadReadWriteLock* lock) {
    ReleaseSRWLockShared(lock);
}

void ThreadSemaphore_Init(ThreadSemaphore* semaphore, int maxCount) {
    *semaphore = CreateSemaphoreA(NULL, 0, maxCount, NULL);
}

void ThreadSemaphore_Wait(ThreadSemaphore* semaphore) {
    WaitForSingleObjectEx(*semaphore, INFINITE, FALSE);
}

bool ThreadSemaphore_Increment(ThreadSemaphore* semaphore) {
    if (ReleaseSemaphore(*semaphore, 1, NULL) != 0) {
        return true;
    }
    else {
        return false;
    }
}

void Thread_Create(Thread* thread, ThreadFunction function, void* args) {
    *thread = CreateThread(NULL, 0, function, args, 0, NULL);
}

void* MemoryAlloc(int size) {
    return VirtualAlloc(0, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
}

void  MemoryFree(void* buffer, int size) {
    VirtualFree(buffer, 0, MEM_RELEASE);
}

bool AtomicCompareExchange(void* destination, void* compare, int replace) {
    return (InterlockedCompareExchange((volatile void*)destination, replace, *(long*)compare) == *(long*)compare);
}
#else 

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

enum HTTPCreateDirStatus HTTP_CreateDir(char* file_path) {
    if (file_path != NULL) {
        if (mkdir(file_path , S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
            switch (errno) {
                case EEXIST:
                    return DIR_ALREADY_EXISTS;
                    break;
                case ENOENT: 
                    return PATH_NOT_FOUND;
                default:
                    return OTHER_ERROR;
            }

        }
        else {
            return SUCCESS;
        }

    }
    else {
        return GIVEN_INVALID_FILE_PATH;
    }
}

bool HTTP_DeleteDirRecursive(char* dir_name) {
    DIR* directory = opendir(dir_name);
    if (directory == NULL) {
        return false;
    }

    closedir(directory);

    char delete_directory_command[240] = {0};
    sprintf(delete_directory_command, "rm -rf %s", dir_name);
    system(delete_directory_command);

    if ((directory = opendir(dir_name)) == NULL) {
        return true;
    }
    else {
        closedir(directory);
        return false;
    }
}

void HTTP_Gen256ByteRandomNum(char* buffer, int buffer_count) {
    Assert(buffer_count > 255);

    FILE* random_num_generator = fopen("/dev/urandom", "r");

    int random_num;
    for (int i = 0; i < 256; i += 4) {
        fread(&random_num, sizeof(int), 1, random_num_generator);
        snprintf(buffer+i, 5, "%d", abs(random_num));
    }

    fclose(random_num_generator);
}


void ThreadReadWriteLock_Initialize(ThreadReadWriteLock* lock) {
    pthread_rwlock_init(lock, NULL);
}

void ThreadReadWriteLock_AcquireExclusiveLock(ThreadReadWriteLock* lock) {
    pthread_rwlock_wrlock(lock);
}

void ThreadReadWriteLock_ReleaseExclusiveLock(ThreadReadWriteLock* lock) {
    pthread_rwlock_unlock(lock);
}

void ThreadReadWriteLock_AcquireSharedLock(ThreadReadWriteLock* lock) {
    pthread_rwlock_rdlock(lock);
}

void ThreadReadWriteLock_ReleaseSharedLock(ThreadReadWriteLock* lock) {
    pthread_rwlock_unlock(lock);
}

void ThreadSemaphore_Init(ThreadSemaphore* semaphore, int maxCount) {
    sem_init(semaphore, 0, 0);
}

void ThreadSemaphore_Wait(ThreadSemaphore* semaphore) {
    sem_wait(semaphore);
}

bool ThreadSemaphore_Increment(ThreadSemaphore* semaphore) {
    return sem_post(semaphore) == 0 ? true : false;
}

void Thread_Create(Thread* thread, ThreadFunction function, void* args) {
    pthread_create(thread, NULL, function, args);
}

void* MemoryAlloc(int size) {
    return mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

void MemoryFree(void* buffer, int size) {
    munmap(buffer, size);
}

bool AtomicCompareExchange(void* destination, void* compare, int replace) {
    return atomic_compare_exchange_strong((volatile int*)destination, (int*)compare, replace);
}
#endif
