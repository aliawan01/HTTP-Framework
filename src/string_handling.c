#include "util.h"
#include "string_handling.h"

bool ContainsWhitespace(char* string) {
    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == ' ') {
            return true;
        }
    }

    return false;
}

bool IsInteger(char* string) {
	bool is_int = true;
    int decimal_point_occurances = 0;
	for (int i = 0; i < strlen(string); i++) {
        if (string[i] == '.') {
            decimal_point_occurances++;
        }
        else if (string[i] < 48 || string[i] > 57) {
			is_int = false;
			break;
		}
	}

    if (decimal_point_occurances > 1) {
        is_int = false;
    }

	return is_int;
}

char* HTTP_StringDup(Arena* arena, char* source) {
    Assert(source != NULL);
    char* duplicate = PushString(arena, strlen(source)+1);
    Assert(duplicate != NULL);
    memcpy(duplicate, source, strlen(source)+1);
    return duplicate;
}

char* DecodeURL(Arena* arena, char* url) {
    char* decoded_url = PushString(arena, strlen(url)+1);

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

Dict ParseHeaderIntoDict(Arena* arena, char* header_string) {
	char** header_key_array = PushArray(arena, char*, 50);
	char** header_value_array = PushArray(arena, char*, 50);
	int header_dict_index = 0;

	int i = 0;
	int original_i = i;

	bool on_key = false;
	bool on_value = false;
	while (true) {
		if (header_string[i] == ':' && !on_key) {
			on_key = true;
			on_value = false;
			header_string[i] = 0;
			PushNewStringToDict(arena, header_key_array, header_dict_index, header_string+original_i);
			/* header_dict_index++; */
			header_string[i] = ':';
			original_i = i+1;
		}
		else if (header_string[i] == '\r' && header_string[i+1] == '\n' && !on_value) {
			on_value = true;
			on_key = false;
			header_string[i] = 0;
			// +1 because after there is a space after : and before the value.
			PushNewStringToDict(arena, header_value_array, header_dict_index, header_string+original_i+1);
			header_dict_index++;
			header_string[i] = '\r';
			original_i = i+2;
			i++;
		}

		i++;
		if (header_string[i] == 0) {
			break;
		}
	}

	return (Dict) {
		.keys = header_key_array,
        .values = header_value_array,
		.count = header_dict_index 
	};
}

Dict ParseURIKeyValuePairString(Arena* arena, char* uri_string, char separator) {
	char** key_array = PushArray(arena, char*, 200);
	char** value_array = PushArray(arena, char*, 200);
	int dict_index = 0;

	int i = 0;
	int original_i = i;

    bool on_key = true;
	while (true) {
		if (uri_string[i] == '=' || uri_string[i] == separator || !uri_string[i]) {
			char original_char = uri_string[i];
			uri_string[i] = 0;

            if (on_key) {
                on_key = false;
                PushNewStringToDict(arena, key_array, dict_index, uri_string+original_i);
            }
            else {
                on_key = true;
                PushNewStringToDict(arena, value_array, dict_index, uri_string+original_i);
                dict_index++;
            }

			uri_string[i] = original_char;
			original_i = i+1;
			/* key_value_array_index += 1; */
			// Hit null terminator
			if (uri_string[i] == 0) {
				break;
			}

		}
		i++;
	}

	return (Dict) {
		.keys = key_array,
		.values = value_array,
		.count = dict_index 
	};
}

StringArray StrRegexGetMatches(Arena* arena, char* source, char* pattern) {
	int match_length = 0;
	int match_id = 0;
	int match_id_offset = 0;
	char* source_ptr = source;

	char** matches = PushArray(arena, char*, 100);
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

char* RemoveWhitespaceFrontAndBack(Arena* arena, char* string, int front_offset, int back_offset) {
	// TODO: Could possible be optimized? Instead calculate the parts with and without spaces 
	// 	     and then use a single memmove across multiple bytes rather than a memmove on each
	//		 iteration of the while loop.
    Temp scratch = GetScratch(&arena, 1);
	char* string_copy = HTTP_StringDup(scratch.arena, string);

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

    char* return_string_copy = HTTP_StringDup(arena, string_copy);
    DeleteScratch(scratch);

	return return_string_copy;
}

// TODO: Find a way to make a copy of the source string the scratch_arena, do work to replace
//       duplicates there and then copy the string to the global_arena and then free the 
//       scratch_arena.
char* StrReplaceSubstringAllOccurance(Arena* arena, char* source, char* substring, char* replace) {
    Temp scratch = GetScratch(&arena, 1); 
    char* source_copy = HTTP_StringDup(scratch.arena, source);

    while (true) {
        char* substring_occurance = strstr(source_copy, substring);

        if (substring_occurance == NULL) {
            break;
        }

        if (strlen(replace) > strlen(substring)) {
            size_t new_size = strlen(source_copy) + (strlen(replace)-strlen(substring))+1;
            ArenaIncrementOffset(scratch.arena, new_size);
        }

        substring_occurance = strstr(source_copy, substring);
        memmove(substring_occurance + strlen(replace),
                substring_occurance + strlen(substring),
                strlen(substring_occurance) - strlen(substring)+1);
            
        memcpy(substring_occurance, replace, strlen(replace));
    }

    char* return_source_copy = HTTP_StringDup(arena, source_copy);
    DeleteScratch(scratch);
    return return_source_copy;
}
