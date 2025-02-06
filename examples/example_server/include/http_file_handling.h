#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

HTTPEXPORTFUNC bool    HTTP_FileExists(char* file_name);
HTTPEXPORTFUNC bool    HTTP_DeleteFile(char* file_name);
HTTPEXPORTFUNC int64_t HTTP_FindFileSize(char *file_name);
HTTPEXPORTFUNC String  HTTP_GetFileContents(Arena* arena, char* file_name);
HTTPEXPORTFUNC bool    HTTP_OverwriteFileContents(char* file_name, char* new_file_contents);

#ifdef __cplusplus
}
#endif
