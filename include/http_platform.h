#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdarg.h>
#include <string.h>
#include <stdatomic.h>
#include <time.h>
#include <re.h>
#include <sqlite3.h>
#include <cJSON.h>
#include <cJSON_Utils.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <semaphore.h>
#include <sys/mman.h>

#define bool int
#define true 1
#define false 0

#ifdef DEBUG_BUILD
#define Assert(condition)\
    if (!(condition)) { \
        fprintf(stderr, "Assertion in file: %s at line %d\n", __FILE__, __LINE__); \
        /* TODO(ali): Find a way to do this on linux */ \
        /* __debugbreak();*/ \
    }           
#else
#define Assert(condition)

#endif

#ifdef _WIN32 
#ifdef BUILD_LIB
    #define HTTPEXPORTFUNC __declspec(dllexport)
#else
    #define HTTPEXPORTFUNC __declspec(dllimport)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <intrin.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <shellapi.h>
#include <wincrypt.h>
#include <processthreadsapi.h>
#include <synchapi.h>

typedef SRWLOCK ThreadReadWriteLock;
typedef HANDLE  ThreadSemaphore;
typedef HANDLE  Thread;

typedef DWORD WINAPI (*ThreadFunction)(LPVOID);
#else
// NOTE(ali): Linux master race.
#ifdef BUILD_LIB
    #define HTTPEXPORTFUNC __attribute__((visibility("default")))
#else
    #define HTTPEXPORTFUNC
#endif

typedef pthread_rwlock_t ThreadReadWriteLock;
typedef sem_t            ThreadSemaphore;
typedef pthread_t        Thread;

typedef void* (*ThreadFunction)(void*);
#endif

enum HTTPCreateDirStatus {
    SUCCESS,
    DIR_ALREADY_EXISTS,
    PATH_NOT_FOUND,
    GIVEN_INVALID_FILE_PATH,
    OTHER_ERROR
};

// Platform Specific Functions
HTTPEXPORTFUNC enum HTTPCreateDirStatus HTTP_CreateDir(char* file_path);
HTTPEXPORTFUNC bool HTTP_DeleteDirRecursive(char* dir_name);
HTTPEXPORTFUNC void HTTP_Gen256ByteRandomNum(char* buffer, int buffer_count);

void Thread_Create(Thread* thread, ThreadFunction function, void* args);

void ThreadReadWriteLock_Initialize(ThreadReadWriteLock* lock);
void ThreadReadWriteLock_AcquireExclusiveLock(ThreadReadWriteLock* lock);
void ThreadReadWriteLock_ReleaseExclusiveLock(ThreadReadWriteLock* lock);
void ThreadReadWriteLock_AcquireSharedLock(ThreadReadWriteLock* lock);
void ThreadReadWriteLock_ReleaseSharedLock(ThreadReadWriteLock* lock);

void ThreadSemaphore_Init(ThreadSemaphore* semaphore, int count);
void ThreadSemaphore_Wait(ThreadSemaphore* semaphore);
bool ThreadSemaphore_Increment(ThreadSemaphore* semaphore);

void* MemoryAlloc(int size);
void  MemoryFree(void* buffer, int size);

bool AtomicCompareExchange(void* destination, void* compare, int replace);
