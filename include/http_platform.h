#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <re.h>
#include <sqlite3.h>
#include <cJSON.h>
#include <cJSON_Utils.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define bool int
#define true 1
#define false 0

#ifdef DEBUG_BUILD
#define Assert(condition)\
    if (!(condition)) { \
        fprintf(stderr, "Assertion in file: %s at line %d\n", __FILE__, __LINE__); \
        /* TODO(ali): Find a way to do this on linux */ \
         __debugbreak(); \
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

typedef DWORD (WINAPI *ThreadFunction)(LPVOID);
#else
#include <semaphore.h>
#include <sys/mman.h>
#include <stdatomic.h>

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

// NOTE(ali):  Platform Specific Functions

/*
   @desc Creates a directory at the specified file path and 
         returns the status of the operation.
   @param file_path The path where the directory should be created.
   @return Returns a `HTTPCreateDirStatus` enum indicating the status
           of the directory creation operation.
*/
HTTPEXPORTFUNC enum HTTPCreateDirStatus HTTP_CreateDir(char* file_path);

/*
   @desc Recursively deletes a directory and its contents.
         This function deletes the specified directory and
         any files or subdirectories inside it.
   @param dir_name The name of the directory to delete.
   @return Returns true if the directory and its contents were successfully deleted, false otherwise.
*/
HTTPEXPORTFUNC bool HTTP_DeleteDirRecursive(char* dir_name);

/*
   @desc Generates a 256 digit random number and stores it as a string
         in the buffer provided.
   @param buffer The buffer to store the generated random number.
   @param buffer_count The size of the buffer (should be at least 256 bytes).
*/
HTTPEXPORTFUNC void HTTP_Gen256ByteRandomNum(char* buffer, int buffer_count);

void Thread_Create(Thread* thread, ThreadFunction function, void* args);

void ThreadReadWriteLock_Initialize(ThreadReadWriteLock* lock);
void ThreadReadWriteLock_AcquireExclusiveLock(ThreadReadWriteLock* lock);
void ThreadReadWriteLock_ReleaseExclusiveLock(ThreadReadWriteLock* lock);
void ThreadReadWriteLock_AcquireSharedLock(ThreadReadWriteLock* lock);
void ThreadReadWriteLock_ReleaseSharedLock(ThreadReadWriteLock* lock);

void ThreadSemaphore_Init(ThreadSemaphore* semaphore, int maxCount);
void ThreadSemaphore_Wait(ThreadSemaphore* semaphore);
bool ThreadSemaphore_Increment(ThreadSemaphore* semaphore);

void* MemoryAlloc(uint64_t size);
void  MemoryFree(void* buffer, uint64_t size);

bool AtomicCompareExchange(void* destination, void* compare, int replace);

#ifdef __cplusplus
}
#endif
