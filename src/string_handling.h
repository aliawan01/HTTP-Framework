#pragma once

typedef struct Dict {
    char** keys;
    char** values;
    int count;
} Dict;

#define PushNewStringToDict(arena, array, index, string) \
	array[index] = PushString(arena, strlen(string)+1);\
	strcpy(array[index], string);

#define PushNewStringToStringArray(arena, array, index, string) PushNewStringToDict(arena, array, index, string)


bool  ContainsWhitespace(char* string);
bool  IsInteger(char* string);
char* DecodeURL(Arena* arena, char* url);
Dict  ParseHeaderIntoDict(Arena* arena, char* header_string);
Dict  ParseURIKeyValuePairString(Arena* arena, char* uri_string, char separator, bool remove_extra_whitespace);
StringArray StrRegexGetMatches(Arena* arena, char* source, char* pattern);
StringArray StrSplitStringOnSeparator(Arena* arena, char* string, char* separator);
char* RemoveWhitespaceFrontAndBack(Arena* arena, char* string, int front_offset, int back_offset);
char* StrReplaceSubstringAllOccurance(Arena* arena, char* source, char* substring, char* replace);
char* HTTP_StringDup(Arena* arena, char* source);
