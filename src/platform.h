#ifndef PLATFORM_H
#define PLATFORM_H

enum CreateDirStatus {
    SUCCESS,
    DIR_ALREADY_EXISTS,
    PATH_NOT_FOUND,
    GIVEN_INVALID_FILE_PATH,
    OTHER_ERROR
};

// Platform Specific Functions
static enum CreateDirStatus CreateDir(char* file_name);

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

static enum CreateDirStatus CreateDir(char* file_path) {
    if (file_path != NULL) { 
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
    else {
        return GIVEN_INVALID_FILE_PATH;
    }
}


#else 

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static enum CreateDirStatus CreateDir(char* file_path) {
    if (default_dir != NULL) {
        if (mkdir(default_dir, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
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


#endif


#endif
