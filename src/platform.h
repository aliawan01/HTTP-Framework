#pragma once

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

#define bool int
#define true 1
#define false 0

#ifdef DEBUG_BUILD
#define Assert(condition)\
    if (!(condition)) { \
        fprintf(stderr, "Assertion in file: %s at line %d\n", __FILE__, __LINE__); \
        __debugbreak(); \
    }           
#else
#define Assert(condition)

#endif

#ifdef _WIN32 

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

#endif

enum HTTPCreateDirStatus {
    SUCCESS,
    DIR_ALREADY_EXISTS,
    PATH_NOT_FOUND,
    GIVEN_INVALID_FILE_PATH,
    OTHER_ERROR
};

// Platform Specific Functions
enum HTTPCreateDirStatus HTTP_CreateDir(char* file_path);
bool HTTP_DeleteDirRecursive(char* dir_name);
void HTTP_Gen256ByteRandomNum(char* buffer, int buffer_count);
