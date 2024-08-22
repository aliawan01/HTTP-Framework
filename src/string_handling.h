#pragma once

typedef struct {
    char** array;
    int count;
} StringArray;

#define PushNewStringToStringArray(arena, array, index, string) \
	array[index] = ArenaAlloc(arena, strlen(string)+1);\
	strcpy(array[index], string);

#define ResizeStringInStringArray(array, index, string) \
	array[index] = realloc(array[index], strlen(string)+1);\
	memset(array[index], 0, strlen(string)+1);\
	strcpy(array[index], string);

bool IsInteger(char* string);
char* DecodeURL(Arena* arena, char* url);
StringArray ParseHeaderIntoKeyValuePairString(Arena* arena, char* header_string);
StringArray ParseURIKeyValuePairString(Arena* arena, char* uri_string);
StringArray StrRegexGetMatches(Arena* arena, char* source, char* pattern);
char* RemoveWhitespaceFrontAndBack(Arena* arena, char* string, int front_offset, int back_offset);
char* StrReplaceSubstringAllOccurance(Arena* arena, char* source, char* substring, char* replace);
char* HTTP_StringDup(Arena* arena, char* source);
