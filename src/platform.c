#include "platform.h"

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



#else 

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

enum CreateDirStatus CreateDir(char* file_path) {
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
