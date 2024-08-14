#include "util.h"
#include "http_request.h"
#include "string_handling.h"

bool IsInteger(char* string) {
	bool is_int = true;
	for (int i = 0; i < strlen(string); i++) {
		if (string[i] < 48 || string[i] > 57) {
			is_int = false;
			break;
		}
	}

	return is_int;
}

char* HTTP_StringDup(Arena* arena, char* source) {
    char* duplicate = ArenaAlloc(arena, strlen(source)+1);
    Assert(duplicate != NULL);
    memcpy(duplicate, source, strlen(source)+1);
    printf("HTTP_StringDup: %s\n", duplicate);
    return duplicate;
}

char* DecodeURL(Arena* arena, char* url) {
    char* decoded_url = ArenaAlloc(arena, strlen(url)+1);

    int url_index = 0;
    int decoded_index = 0;
    for (;url_index < strlen(url); url_index++, decoded_index++) {
        if (url[url_index] == '+') {
            decoded_url[decoded_index] = ' '; 
        }
        else if (url[url_index] == '%') {
            char escape_character;
            char escape_code[3] = {0};
            escape_code[0] = url[url_index+1];
            escape_code[1] = url[url_index+2];
            url_index += 2;

            // Matching escape character
            if (!strcmp(escape_code, "20")) {
                escape_character = ' ';
            }
            else if (!strcmp(escape_code, "3C")) {
                escape_character = '<';
            }
            else if (!strcmp(escape_code, "3E")) {
                escape_character = '>';
            }
            else if (!strcmp(escape_code, "23")) {
                escape_character = '#';
            }
            else if (!strcmp(escape_code, "25")) {
                escape_character = '%';
            }
            else if (!strcmp(escape_code, "2B")) {
                escape_character = '+';
            }
            else if (!strcmp(escape_code, "7B")) {
                escape_character = '{';
            }
            else if (!strcmp(escape_code, "7D")) {
                escape_character = '}';
            }
            else if (!strcmp(escape_code, "7C")) {
                escape_character = '|';
            }
            else if (!strcmp(escape_code, "5C")) {
                escape_character = '\\';
            }
            else if (!strcmp(escape_code, "5E")) {
                escape_character = '^';
            }
            else if (!strcmp(escape_code, "7E")) {
                escape_character = '~';
            }
            else if (!strcmp(escape_code, "5B")) {
                escape_character = '[';
            }
            else if (!strcmp(escape_code, "5D")) {
                escape_character = ']';
            }
            else if (!strcmp(escape_code, "60")) {
                escape_character = '‘';
            }
            else if (!strcmp(escape_code, "3B")) {
                escape_character = ';';
            }
            else if (!strcmp(escape_code, "2F")) {
                escape_character = '/';
            }
            else if (!strcmp(escape_code, "3F")) {
                escape_character = '?';
            }
            else if (!strcmp(escape_code, "3A")) {
                escape_character = ':';
            }
            else if (!strcmp(escape_code, "40")) {
                escape_character = '@';
            }
            else if (!strcmp(escape_code, "3D")) {
                escape_character = '=';
            }
            else if (!strcmp(escape_code, "26")) {
                escape_character = '&';
            }
            else if (!strcmp(escape_code, "24")) {
                escape_character = '$';
            }
            else if (!strcmp(escape_code, "21")) {
                escape_character = '!';
            }
            else if (!strcmp(escape_code, "2A")) {
                escape_character = '*';
            }
            else if (!strcmp(escape_code, "2D")) {
                escape_character = '-';
            }
            else if (!strcmp(escape_code, "2E")) {
                escape_character = '.';
            }

            decoded_url[decoded_index] = escape_character;
        }
        else {
            decoded_url[decoded_index] = url[url_index];
        }
    }


    return decoded_url;
}

StringArray ParseHeaderIntoKeyValuePairString(Arena* arena, char* header_string) {
	char** header_key_value_pairs_array = ArenaAlloc(arena, sizeof(char*)*50);
	int header_key_value_pairs_index = 0;
	int i = 0;
	int original_i = i;

	bool on_key = false;
	bool on_value = false;
	while (true) {
		if (header_string[i] == ':' && !on_key) {
			on_key = true;
			on_value = false;
			header_string[i] = 0;
			PushNewStringToStringArray(arena, header_key_value_pairs_array, header_key_value_pairs_index, header_string+original_i);
			header_key_value_pairs_index++;
			header_string[i] = ':';
			original_i = i+1;
		}
		else if (header_string[i] == '\r' && header_string[i+1] == '\n' && header_string[i+2] != '\r' && !on_value) {
			on_value = true;
			on_key = false;
			header_string[i] = 0;
			// +1 because after there is a space after : and before the value.
			PushNewStringToStringArray(arena, header_key_value_pairs_array, header_key_value_pairs_index, header_string+original_i+1);
			header_key_value_pairs_index++;
			header_string[i] = '\r';
			original_i = i+2;
			i++;
		}

		i++;
		if (header_string[i] == 0) {
			break;
		}
	}

	return (StringArray) {
		.array = header_key_value_pairs_array,
		.count = header_key_value_pairs_index 
	};
}

StringArray ParseURIKeyValuePairString(Arena* arena, char* uri_string) {
	char** key_value_pairs_array = ArenaAlloc(arena, sizeof(char*)*200);
	int key_value_array_index = 0;
	int i = 0;
	int original_i = i;

	while (true) {
		if (uri_string[i] == '=' || uri_string[i] == '&' || !uri_string[i]) {
			char original_char = uri_string[i];
			uri_string[i] = 0;

			PushNewStringToStringArray(arena, key_value_pairs_array, key_value_array_index, uri_string+original_i);

			uri_string[i] = original_char;
			original_i = i+1;
			key_value_array_index += 1;
			// Hit null terminator
			if (uri_string[i] == 0) {
				break;
			}

		}
		i++;
	}

	return (StringArray) {
		.array = key_value_pairs_array,
		.count = key_value_array_index
	};
}

StringArray StrRegexGetMatches(Arena* arena, char* source, char* pattern) {
	int match_length = 0;
	int match_id = 0;
	int match_id_offset = 0;
	char* source_ptr = source;

	char** matches = ArenaAlloc(arena, sizeof(char*)*100);
	int matches_index = 0;
	while (match_id != -1) {
		match_id = re_match(pattern, source_ptr, &match_length);
		if (match_id != -1) {
			// TODO: Can be optimized.
			match_id_offset += match_id;
			source_ptr += (match_id + match_length);

			char original_value = source[match_id_offset+match_length];
			source[match_id_offset+match_length] = 0;

			PushNewStringToStringArray(arena, matches, matches_index, source+match_id_offset);
			matches_index++;

			source[match_id_offset+match_length] = original_value;
			match_id_offset += match_length;
		}
	}

	return (StringArray) {
		.array = matches,
		.count = matches_index
	};
}

char* RemoveWhitespaceFrontAndBack(Allocator* allocator, char* string, int front_offset, int back_offset) {
	// TODO: Could possible be optimized? Instead calculate the parts with and without spaces 
	// 	     and then use a single memmove across multiple bytes rather than a memmove on each
	//		 iteration of the while loop.
	char* string_copy = HTTP_StringDup(allocator->scratch_arena, string);

	char* front_ptr = string_copy+front_offset;
	if (front_ptr[0] == ' ') {
		for (; front_ptr[0] != ' '; front_ptr++);
	}

	while (front_ptr[0] == ' ') {
		memmove(front_ptr, front_ptr+1, strlen(front_ptr+1)+1);
	}

	char* back_ptr = string_copy+strlen(string_copy)-back_offset-1;
	if (back_ptr[0] == ' ') {
		for (; back_ptr[0] != ' '; back_ptr--);
	}

	while (back_ptr[0] == ' ') {
		memmove(back_ptr, back_ptr+1, strlen(back_ptr+1)+1);
		back_ptr--;
	}

    char* return_string_copy = HTTP_StringDup(allocator->global_arena, string_copy);
    ArenaFreeAll(allocator->scratch_arena);

	return return_string_copy;
}

// TODO: Find a way to make a copy of the source string the scratch_arena, do work to replace
//       duplicates there and then copy the string to the global_arena and then free the 
//       scratch_arena.
char* StrReplaceSubstringAllOccurance(Allocator* allocator, char** source, char* substring, char* replace) {
    while (true) {
        char* substring_occurance = strstr(*source, substring);
        if (substring_occurance == NULL) {
            break;
        }

        int64_t size = strlen(*source);
        if (strlen(replace) > strlen(substring)) {
            size += (strlen(replace)-strlen(substring))+1;
        }

        char* copy_string = ArenaAlloc(allocator->scratch_arena, size);
        strcpy(copy_string, *source);

        substring_occurance = strstr(copy_string, substring);
        memmove(substring_occurance + strlen(replace),
                substring_occurance + strlen(substring),
                strlen(substring_occurance) - strlen(substring)+1);
            
        memcpy(substring_occurance, replace, strlen(replace));
    }

    char* return_string = HTTP_StringDup(allocator->global_arena, copy_string);
    ArenaFreeAll(allocator->scratch_arena);

    return return_string;
}
