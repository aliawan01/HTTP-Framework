#pragma once

bool    HTTP_FileExists(char* file_name);
bool    HTTP_DeleteFile(char* file_name);
int64_t HTTP_FindFileSize(char *file_name);
String  HTTP_GetFileContents(Arena* arena, char* file_name);
bool    HTTP_OverwriteFileContents(char* file_name, char* new_file_contents);
