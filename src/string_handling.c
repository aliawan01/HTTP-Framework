#include "server.h"
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


static StringArray ParseHeaderIntoKeyValuePairString(char* header_string) {
	char** header_key_value_pairs_array = malloc(sizeof(char*)*50);
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
			PushNewStringToStringArray(header_key_value_pairs_array, header_key_value_pairs_index, header_string+original_i);
			header_key_value_pairs_index++;
			header_string[i] = ':';
			original_i = i+1;
		}
		else if (header_string[i] == '\r' && header_string[i+1] == '\n' && header_string[i+2] != '\r' && !on_value) {
			on_value = true;
			on_key = false;
			header_string[i] = 0;
			// +1 because after there is a space after : and before the value.
			PushNewStringToStringArray(header_key_value_pairs_array, header_key_value_pairs_index, header_string+original_i+1);
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

static StringArray ParseURIKeyValuePairString(char* uri_string) {
	char** key_value_pairs_array = malloc(sizeof(char*)*200);
	int key_value_array_index = 0;
	int i = 0;
	int original_i = i;

	while (true) {
		if (uri_string[i] == '=' || uri_string[i] == '&' || !uri_string[i]) {
			char original_char = uri_string[i];
			uri_string[i] = 0;

			PushNewStringToStringArray(key_value_pairs_array, key_value_array_index, uri_string+original_i);

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

static StringArray StrRegexGetMatches(char* source, char* pattern) {
	int match_length = 0;
	int match_id = 0;
	int match_id_offset = 0;
	char* source_ptr = source;

	char** matches = malloc(sizeof(char*)*100);
	int matches_index = 0;
	while (match_id != -1) {
		match_id = re_match(pattern, source_ptr, &match_length);
		if (match_id != -1) {
			// TODO: Can be optimized.
			match_id_offset += match_id;
			source_ptr += (match_id + match_length);

			char original_value = source[match_id_offset+match_length];
			source[match_id_offset+match_length] = 0;

			PushNewStringToStringArray(matches, matches_index, source+match_id_offset);
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

static char* RemoveWhitespaceFrontAndBack(char* string, int front_offset, int back_offset) {
	// TODO: Could possible be optimized? Instead calculate the parts with and without spaces 
	// 	     and then use a single memmove across multiple bytes rather than a memmove on each
	//		 iteration of the while loop.
	char* string_copy = malloc(strlen(string)+1);
	memset(string_copy, 0, strlen(string)+1);
	strcpy(string_copy, string);

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

	return string_copy;
}

static bool StrReplaceSubstringFirstOccurance(char** source, char* substring, char* replace) {
    char* substring_occurance = strstr(*source, substring);
    if (substring_occurance == NULL) {
        return false;
    }

    if (strlen(replace) > strlen(substring)) {
        size_t new_size = strlen(*source) + (strlen(replace)-strlen(substring))+1;
        *source = realloc(*source, new_size);
    }

    substring_occurance = strstr(*source, substring);
    memmove(substring_occurance + strlen(replace),
            substring_occurance + strlen(substring),
            strlen(substring_occurance) - strlen(substring)+1);
        
    memcpy(substring_occurance, replace, strlen(replace));
    return true;
}