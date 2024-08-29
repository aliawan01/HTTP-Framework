#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>
#include <re.h>
#include <sqlite3.h>
#include <cJSON.h>

#ifdef _WIN32 

#define _CRT_SECURE_NO_WARNINGS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <intrin.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <shellapi.h>

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

