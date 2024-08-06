#pragma once

typedef struct {
    char** array;
    int count;
} StringArray;

#define StrReplaceSubstringAllOccurance(a, b, c) while(StrReplaceSubstringFirstOccurance(a, b, c))
#define PushNewStringToStringArray(array, index, string) \
	array[index] = malloc(strlen(string)+1);\
	memset(array[index], 0, strlen(string)+1);\
	strcpy(array[index], string);

#define ResizeStringInStringArray(array, index, string) \
	array[index] = realloc(array[index], strlen(string)+1);\
	memset(array[index], 0, strlen(string)+1);\
	strcpy(array[index], string);

#define FreeStringArray(array, max_size) \
	for (int x = 0; x < max_size; x++) {\
		free(array[x]);\
	}\
	free(array);


bool IsInteger(char* string);
char* DecodeURL(char* url);
StringArray ParseHeaderIntoKeyValuePairString(char* header_string);
StringArray ParseURIKeyValuePairString(char* uri_string);
StringArray StrRegexGetMatches(char* source, char* pattern);
char* RemoveWhitespaceFrontAndBack(char* string, int front_offset, int back_offset);
bool StrReplaceSubstringFirstOccurance(char** source, char* substring, char* replace);
