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

// TODO: Need to replace this with the scratch_arena.
#define FreeStringArray(array, max_size) \
	for (int x = 0; x < max_size; x++) {\
		free(array[x]);\
	}\
	free(array);


bool IsInteger(char* string);
char* DecodeURL(Arena* arena, char* url);
StringArray ParseHeaderIntoKeyValuePairString(Arena* arena, char* header_string);
StringArray ParseURIKeyValuePairString(Arena* arena, char* uri_string);
StringArray StrRegexGetMatches(Arena* arena, char* source, char* pattern);
char* RemoveWhitespaceFrontAndBack(Allocator* allocator, char* string, int front_offset, int back_offset);
char* StrReplaceSubstringAllOccurance(Allocator* allocator, char** source, char* substring, char* replace);
char* HTTP_StringDup(Arena* arena, char* source);
