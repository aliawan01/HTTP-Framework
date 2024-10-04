#include "util.h"
#include "string_handling.h"
#include "http_request.h"
#include "file_handling.h"
#include "authentication.h"

void HTTP_Initialize(void) {
    // TODO: Perhaps allow the user to specify the size of the global and scratch arena.
    ArenaInit(allocator.permanent_arena, MB(1024));
    ArenaInit(allocator.recycle_arena, MB(100));
    ArenaInit(allocator.route_callback_arena, MB(100));

    ctx = BaseThread_CreateThreadContext(allocator.permanent_arena, 20*1024*1024, 10*1024*1024);

    for (int i = 0; i < ArrayCount(allocator.scratch_pool); i++) {
        ArenaInit(allocator.scratch_pool[i], MB(50));
    }

	global_route_callback_array = PushArray(allocator.route_callback_arena, HTTPRouteCallback, INITIAL_GLOBAL_ROUTE_CALLBACK_ARRAY_SIZE);
	global_route_callback_index = -1;
}


bool HTTP_HandleRoute(StringArray permissions, char* method, char* route, bool is_regex_route, void (*response_func)(Arena* arena, HTTPRequestInfo*, HTTPResponse*)) {
	// Checking if the route is in the correct format.
	if (route[0] != '/') {
		printf("[ERROR] HTTP_HandleRoute() route given: `%s` is not in the correct format.\n", route);
		return false;
	}

	// Checking if the route already exists.
	for (int index = 0; index < global_route_callback_index+1; index++) {
		if (!strcmp(global_route_callback_array[index].route, route)) {
            if (!strcmp(global_route_callback_array[index].method, method)) {
                bool contains_same_permissions = true;
                if (permissions.count == global_route_callback_array[index].permissions.count) {
                    for (int i = 0; i < permissions.count; i++) {
                        if (strcmp(permissions.array[i], global_route_callback_array[index].permissions.array[i])) {
                            contains_same_permissions = false;
                            break;
                        }
                    }
                }
                else {
                    contains_same_permissions = false;
                }

                if (contains_same_permissions) {
                    printf("[INFO] HTTP_HandleRoute() overwrote callback function for method `%s` at route `%s` with regex `%d` and with permissions `%s`.\n", method, route, is_regex_route, ConvertStrArrayToString(allocator.recycle_arena, permissions, ", "));
                    global_route_callback_array[index].is_regex_route = is_regex_route;
                    global_route_callback_array[index].response_func = response_func;
                    return true;
                }
            }
		}
	}

	global_route_callback_index++;
    Assert(global_route_callback_index < INITIAL_GLOBAL_ROUTE_CALLBACK_ARRAY_SIZE);

    Assert(permissions.count < 20);
    StringArray permissions_copy = {
        .array = malloc(sizeof(char*)*20),
        .count = permissions.count
    };

    for (int i = 0; i < permissions_copy.count; i++) {
        permissions_copy.array[i] = strdup(permissions.array[i]);
    }
    
	global_route_callback_array[global_route_callback_index] = (HTTPRouteCallback) {
        .permissions = permissions_copy,
        .method = strdup(method),
		.route = strdup(route),
        .is_regex_route = is_regex_route,
        .response_func = response_func,
	};

	printf("Added Route: `%s`, Method: `%s`, Permissions: `%s` and with response_func\n", global_route_callback_array[global_route_callback_index].route, global_route_callback_array[global_route_callback_index].method, ConvertStrArrayToString(allocator.recycle_arena, global_route_callback_array[global_route_callback_index].permissions, ", "));
	return true;
}


static bool DeleteRouteImpl(char* method, char* route, bool is_regex_route, bool check_method) {
    bool successfully_deleted_route = false;

    for (int i = 0; i < global_route_callback_index+1; i ++) {
        bool is_match = !strcmp(route, global_route_callback_array[i].route) &&
                        global_route_callback_array[i].is_regex_route == is_regex_route;

        if (check_method) {
            is_match &= !strcmp(method, global_route_callback_array[i].method);
        }


        if (is_match) {
            successfully_deleted_route = true;

            if (check_method) {
                printf("[INFO] HTTP_DeletRouteForMethod() Deleted route for: `%s` with method: `%s`, contains regex: %d at index: %d\n", route, method, is_regex_route, i);
            }
            else {
                printf("[INFO] HTTP_DeleteRouteAllMethod() Deleted route for: `%s` with method: `%s`, contains regex: %d at index: %d\n", route, global_route_callback_array[i].method, is_regex_route, i);
            }

            free(global_route_callback_array[i].route);
            free(global_route_callback_array[i].method);

            for (int x = 0; x < global_route_callback_array[i].permissions.count; x++) {
                free(global_route_callback_array[i].permissions.array[x]);
            }

            free(global_route_callback_array[i].permissions.array);

            // Checking if there was only 1 element in global_route_callback_array.
            if (global_route_callback_index == 0) {
                ArenaDealloc(allocator.route_callback_arena);
                global_route_callback_index--;
            }
            else {
                if (i+1 > INITIAL_GLOBAL_ROUTE_CALLBACK_ARRAY_SIZE) {
                    printf("--------------------------\n");
                    printf("Resized route_callback_array.\n");
                    printf("--------------------------\n");
                    ArenaIncrementOffset(allocator.route_callback_arena, sizeof(HTTPRouteCallback)*10);
                }
                memmove(global_route_callback_array+i, global_route_callback_array+i+1, sizeof(HTTPRouteCallback)*global_route_callback_index);
                memset(global_route_callback_array+global_route_callback_index, 0, sizeof(HTTPRouteCallback));
                global_route_callback_index--;
            }

             
            i -= 1;
            if (check_method) {
                break;
            }
        }
    }

    return successfully_deleted_route;

}

bool HTTP_DeleteRouteForMethod(char* method, char* route, bool is_regex_route) {
    return DeleteRouteImpl(method, route, is_regex_route, true);
}

bool HTTP_DeleteRouteForAllMethod(char* route, bool is_regex_route) {
    return DeleteRouteImpl(NULL, route, is_regex_route, false);
}

void HTTP_SetSearchDirectories(char* dirs[], size_t dirs_size) {
	if (dirs != NULL) {
		search_dirs = PushArray(allocator.permanent_arena, char*, dirs_size);
		search_dirs_size = dirs_size;
		for (int i = 0; i < search_dirs_size; i++) {
			search_dirs[i] = HTTP_StringDup(allocator.permanent_arena, dirs[i]);
		}
	}
}


#if 0
void HTTP_SetDefaultPUTDirectory(char* default_dir) {
    bool valid_path = false;
    switch (CreateDir(default_dir)) {
        case DIR_ALREADY_EXISTS: 
            printf("[WARNING] HTTP_SetDefaultPUTDirectory() the directory at the path specified: `%s` already exists.\n", default_dir);
            valid_path = true;
            break;
        case PATH_NOT_FOUND: 
            printf("[ERROR] HTTP_SetDefaultPUTDirectory() a component of the path specified: `%s` doesn't exist.\n", default_dir);
            valid_path = false;
            break;
        case GIVEN_INVALID_FILE_PATH:
            printf("[ERROR] HTTP_SetDefaultPUTDirectory() given an invalid path `%s`.\n", default_dir);
            valid_path = false;
            break;
        case SUCCESS: 
            printf("Successfully created the PUT directory at the path specified: `%s`\n", default_dir);
            valid_path = true;
            break;
        case OTHER_ERROR:
            printf("[ERROR] HTTP_SetDefaultPUTDirectory() an expected error occured when creating the directory at the path specified: `%s`\n", default_dir);
            valid_path = false;
            break;
    }

    if (valid_path) {
        put_request_default_dir = PushString(allocator.permanent_arena, strlen(default_dir)+1);
        strcpy(put_request_default_dir, default_dir);
    }
    else {
        put_request_default_dir = PushString(allocator.permanent_arena, 4);
        strcpy(put_request_default_dir, "");
    }
}
#endif

/* bool HTTP_HandleRedirectRoute(char* method, char* origin_route, char* redirect_route) { */
/* 	// Checking if the route is in the correct format. */
/* 	if (origin_route[0] != '/') { */
/* 		printf("[ERROR] HTTP_HandleRedirectRoute() origin route given: `%s` is not in the correct format.\n", origin_route); */
/* 		return false; */
/* 	} */

/* 	bool redirect_route_exists = false; */
/* 	int redirect_route_index; */

/* 	for (int index = 0; index < route_info_array_index+1; index++) { */
/* 		if (!strcmp(route_info_array[index].route, origin_route)) { */
/* 			printf("[ERROR] HTTP_HandleRedirectRoute() origin route given already exists: `%s`\n", origin_route); */
/* 			return false; */
/* 		} */
/* 		else if (!strcmp(route_info_array[index].route, redirect_route)) { */
/* 			redirect_route_exists = true; */
/* 			redirect_route_index = index; */
/* 		} */
/* 	} */

/* 	if (!redirect_route_exists) { */
/* 		printf("[ERROR] HTTP_HandleRedirectRoute() redirection route given does not exist: %s\n", redirect_route); */
/* 		return false; */
/* 	} */

/* 	route_info_array_index++; */
	
/* 	route_info_array[route_info_array_index] = (HTTPRouteAndFilePath) { */
/* 		.route = origin_route, */
/* 		.path_to_file = route_info_array[redirect_route_index].path_to_file */
/* 	}; */

/* 	printf("Added Redirect Route: origin route: %s, redirect route: %s\n", origin_route, redirect_route); */
/* 	return true; */
/* } */

static bool HTTP_ExtractFileTypeFromFilePath(char* path_to_file, char* file_type) {
	int index = 0;
	
	// TODO: Can probably optimise this.
	while (true) {
		if (path_to_file[index] == '.') {
			break;
		}

		if (index < strlen(path_to_file)) {
			index++;
		}
		else {
			return false;
		}
	}
	
	index++;
	for (int file_type_index = 0; index != strlen(path_to_file); index++, file_type_index++) {
		file_type[file_type_index] = path_to_file[index];
	}
	return true;
}


char* HTTP_CreateDateString(Arena* arena, String day_name, int day_num, String month, int year, int hour, int minute, int second) {
    char* valid_day_names[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    char* valid_month[]     = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    if (day_name.count != 3) {
        printf("[ERROR] HTTP_CreateDateString() day_name needs to be a 3 character string, currently it is %lld characters.\n", day_name.count);
        return NULL;
    }

    if (month.count != 3) {
        printf("[ERROR] HTTP_CreateDateString() month needs to be a 3 character string, currently it is %lld characters.\n", month.count);
        return NULL;
    }

    bool is_valid_day_name = false;
    for (int i = 0; i < ArrayCount(valid_day_names); i++) {
        if (!strcmp(day_name.string, valid_day_names[i])) {
            is_valid_day_name = true;
            break;
        }
    }

    if (!is_valid_day_name) {
        printf("[ERROR] HTTP_CreateDateString() day_name's current string `%s` is incorrect, it must be one of the following `Mon, Tue, Wed, Thu, Fri, Sat, Sun`.\n", day_name.string);
        return NULL;
    }


    bool is_valid_month = false;
    for (int i = 0; i < ArrayCount(valid_month); i++) {
        if (!strcmp(month.string, valid_month[i])) {
            is_valid_month = true;
            break;
        }
    }

    if (!is_valid_month) {
        printf("[ERROR] HTTP_CreateDateString() month's current string `%s` is incorrect, it must be one of the following `Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec`.\n", month.string);
        return NULL;
    }

    if (year > 9999 || year < 0) {
        printf("[ERROR] HTTP_CreateDateString() year's current value `%d` is incorrect, it must be in the range 0-9999.\n", year);
        return NULL;
    }

    if (day_num < 1 || day_num > 31) {
        printf("[ERROR] HTTP_CreateDateString() day's current value `%d` is incorrect, it must be in the range 1-31.\n", day_num);
        return NULL;
    }

    if (hour < 0 || hour > 23) {
        printf("[ERROR] HTTP_CreateDateString() hour's current value `%d` is incorrect, it must be in the range 0-23.\n", hour);
        return NULL;
    }

    if (minute < 0 || minute > 59) {
        printf("[ERROR] HTTP_CreateDateString() minute's current value `%d` is incorrect, it must be in the range 0-59.\n", minute);
        return NULL;
    }

    if (second < 0 || second > 59) {
        printf("[ERROR] HTTP_CreateDateString() second's current value `%d` is incorrect, it must be in the range 0-59.\n", second);
        return NULL;
    }

    char num_string_array[5][8] = {0};
    char year_string[8] = {0};
    int num_value_array[] = { day_num, hour, minute, second };

    for (int i = 0; i < ArrayCount(num_value_array); i++) {
        if (num_value_array[i] < 10) {
            sprintf(num_string_array[i], "0%d", num_value_array[i]);
        }
        else {
            sprintf(num_string_array[i], "%d", num_value_array[i]);
        }
    }

    sprintf(year_string, "%d", year);
    int num_of_zeroes_to_prefix = 4 - strlen(year_string);
    if (num_of_zeroes_to_prefix) {
        memset(year_string, 0, 8);
        int year_string_offset = 0;

        for (int i = 0; i < num_of_zeroes_to_prefix; i++) {
            sprintf(year_string+year_string_offset, "0");
            year_string_offset++;
        }

        sprintf(year_string+year_string_offset, "%d", year);
    }

    char* date_string = PushString(arena, 124);
    sprintf(date_string, "%s, %s %s %s %s:%s:%s GMT", day_name.string, num_string_array[0], month.string, year_string, num_string_array[1], num_string_array[2], num_string_array[3]);

    return date_string;
}

static char* CreateResponseHeader(Arena* arena, enum HTTPStatusCode status_code, HeaderDict headers, CookieJar cookie_jar) {
    char general_buffer[100] = {0};

	char* http_response_header = PushString(arena, 8192);
	sprintf(http_response_header, "HTTP/1.1 %s\r\n", HTTP_StatusCodeStrings[status_code]);
    for (int x = 0; x < headers.count; x++) {
        strcat(http_response_header, headers.keys[x]);
        strcat(http_response_header, ": ");
        strcat(http_response_header, headers.values[x]);
        strcat(http_response_header, "\r\n");
    }

    for (int i = 0; i < cookie_jar.count; i++) {
        strcat(http_response_header, "Set-Cookie: ");
        strcat(http_response_header, cookie_jar.cookies[i].key);
        strcat(http_response_header, "=");
        strcat(http_response_header, cookie_jar.cookies[i].value);

        if (cookie_jar.cookies[i].max_age > -1) {
            sprintf(general_buffer, ";Max-Age=%lld", cookie_jar.cookies[i].max_age);
            strcat(http_response_header, general_buffer);
        }

        if (cookie_jar.cookies[i].expires) {
            strcat(http_response_header, ";Expires=");
            strcat(http_response_header, cookie_jar.cookies[i].expires);
        }

        if (cookie_jar.cookies[i].path) {
            strcat(http_response_header, ";Path=");
            strcat(http_response_header, cookie_jar.cookies[i].path);
        }

        if (cookie_jar.cookies[i].domain) {
            strcat(http_response_header, ";Domain=");
            strcat(http_response_header, cookie_jar.cookies[i].domain);
        }

        if (cookie_jar.cookies[i].secure) {
            strcat(http_response_header, ";Secure");
        }

        if (cookie_jar.cookies[i].http_only) {
            strcat(http_response_header, ";HttpOnly");
        }

        strcat(http_response_header, "\r\n");
    }

    strcat(http_response_header, "\r\n");

	return http_response_header;
}

static void HTTP_ConvertFileExtensionToContentType(char* content_type, char* file_type) {
	if (!strcmp(file_type, "html")) {
		strcpy(content_type, "text/html");
	}
	else if (!strcmp(file_type, "css")) {
		strcpy(content_type, "text/css");
	}
	else if (!strcmp(file_type, "js")) {
		strcpy(content_type, "text/javascript");
	}
	else if (!strcmp(file_type, "md")) {
		strcpy(content_type, "text/markdown");
	}
	else if (!strcmp(file_type, "markdown")) {
		strcpy(content_type, "text/markdown");
	}
	else if (!strcmp(file_type, "json")) {
		strcpy(content_type, "application/json");
	}
	else if (!strcmp(file_type, "apng")) {
		strcpy(content_type, "image/apng");
	}
	else if (!strcmp(file_type, "avif")) {
		strcpy(content_type, "image/avif");
	}
	else if (!strcmp(file_type, "jpeg")) {
		strcpy(content_type, "image/jpeg");
	}
	else if (!strcmp(file_type, "jpg")) {
		strcpy(content_type, "image/jpg");
	}
	else if (!strcmp(file_type, "png")) {
		strcpy(content_type, "image/png");
	}
	else if (!strcmp(file_type, "svg")) {
		strcpy(content_type, "image/svg+xml");
	}
	else if (!strcmp(file_type, "webp")) {
		strcpy(content_type, "image/webp");
	}
	else if (!strcmp(file_type, "ico")) {
		strcpy(content_type, "image/x-ico");
	}
}

static char* HTTP_CreateResponseHeaderFromFile(Arena* arena, char* path_to_file, bool created_new_entry) {
	// Extracting file type into a separate string.
	char file_type[24] = {0};
	HTTP_ExtractFileTypeFromFilePath(path_to_file, file_type);

	// Generating Header based on file type.
	char content_type[100] = {0};
    HTTP_ConvertFileExtensionToContentType(content_type, file_type);

    int status_code;
    if (created_new_entry) {
        status_code = 201;
    }
    else {
        status_code = 200;
    }

	char* http_response_header = PushString(arena, 1024);
	sprintf(http_response_header, "HTTP/1.1 %d OK\r\nContent-Type: %s\r\n\r\n", status_code, content_type);

	return http_response_header;
}

// TODO: Allow the user to send a custom 404 page.
void HTTP_Send404Page(SOCKET client_socket, char* route) {
	char* http_error_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><h1>404 couldn't find requested resource.</h1></html>";
	send(client_socket, http_error_header, (int)strlen(http_error_header), 0);
	printf("[SERVER] Sent 404 message for route: %s\n", route);
}

static void HTTP_ConvertContentTypeToFileExtension(char* content_type, char* file_extension) {
	if (!strcmp(content_type, "text/html")) {
		strcpy(file_extension, "html");
	}
	else if (!strcmp(content_type, "text/css")) {
		strcpy(file_extension, "css");
	}
	else if (!strcmp(content_type, "text/javascript")) {
		strcpy(file_extension, "js");
	}
	else if (!strcmp(content_type, "text/markdown")) {
		strcpy(file_extension, "md");
	}
	else if (!strcmp(content_type, "application/json")) {
		strcpy(file_extension, "json");
	}
	else if (!strcmp(content_type, "image/apng")) {
		strcpy(file_extension, "apng");
	}
	else if (!strcmp(content_type, "image/avif")) {
		strcpy(file_extension, "avif");
	}
	else if (!strcmp(content_type, "image/jpeg")) {
		strcpy(file_extension, "jpeg");
	}
	else if (!strcmp(content_type, "image/jpg")) {
		strcpy(file_extension, "jpg");
	}
	else if (!strcmp(content_type, "image/png")) {
		strcpy(file_extension, "png");
	}
	else if (!strcmp(content_type, "image/svg+xml")) {
		strcpy(file_extension, "svg");
	}
	else if (!strcmp(content_type, "image/webp")) {
		strcpy(file_extension, "webp");
	}
	else if (!strcmp(content_type, "image/x-ico")) {
		strcpy(file_extension, "ico");
	}
}

static void ConvertURIDataToJSON(HTTPRequestInfo* request_info, char* request_body) {
	printf("[POST Request Data] %s\n", request_body);
    Temp scratch = GetScratch(0, 0);

	Dict request_key_value_dict = ParseURIKeyValuePairString(scratch.arena, request_body, '&', false);

    request_info->json_request_body = cJSON_CreateObject();
	if (request_body[0] != 0) {
		for (int index = 0; index < request_key_value_dict.count; index ++) {

            // TODO: Support arrays?
            cJSON* item = NULL;
            if (IsInteger(request_key_value_dict.values[index])) {
                char* endptr;
                item = cJSON_CreateNumber(strtod(request_key_value_dict.values[index], &endptr));
            }
            else if (!strcmp(request_key_value_dict.values[index], "true")) {
                item = cJSON_CreateBool(1);
            }
            else if (!strcmp(request_key_value_dict.values[index], "false")) {
                item = cJSON_CreateBool(0);
            }
            else if (!strcmp(request_key_value_dict.values[index], "null")) {
                item = cJSON_CreateNull();
            }
            else {
                char* decoded_string = DecodeURL(scratch.arena, request_key_value_dict.values[index]);
                item = cJSON_CreateString(decoded_string);
            }

            cJSON_AddItemToObject(request_info->json_request_body, request_key_value_dict.keys[index], item);
		}
	}

	printf("[POST Request Processing]: %s\n", cJSON_Print(request_info->json_request_body));

    DeleteScratch(scratch);
}

void HTTP_TemplateText(Arena* arena, HTTPRequestInfo* request_info, cJSON* variables, String* source) {
    Temp scratch = GetScratch(0, 0);
    char** source_string = &source->string;

    if (request_info->contains_query_string) {
        // Using regex to extract matching variables from the file contents buffer.
        StringArray original_file_vars_matches = StrRegexGetMatches(scratch.arena, *source_string, "{{[^}]*}}");

        char** trim_file_vars = PushArray(scratch.arena, char*, original_file_vars_matches.count);
        for (int index = 0; index < original_file_vars_matches.count; index++) {
            char* temp = RemoveWhitespaceFrontAndBack(scratch.arena, original_file_vars_matches.array[index], 2, 2);
            // Remove `}}` from the string.
            temp[strlen(temp)-2] = 0;
            // Remove `{{` from the string.
            temp += 2;
            trim_file_vars[index] = temp;
        }

        // Extracting variables and values into key_value_pairs_array;
        Dict request_key_value_dict = ParseURIKeyValuePairString(scratch.arena, request_info->query_string, '&', false);

        // Replacing variables in the file contents buffer with their values
        for (int file_var_index = 0; file_var_index < original_file_vars_matches.count; file_var_index++) {
            for (int key_index = 0; key_index < request_key_value_dict.count; key_index ++) {
                if (!strcmp(trim_file_vars[file_var_index], request_key_value_dict.keys[key_index])) {
                    *source_string = StrReplaceSubstringAllOccurance(arena, *source_string, original_file_vars_matches.array[file_var_index], request_key_value_dict.values[key_index]);
                }
            }
        }

        TempEnd(scratch);
    }

    // Checking if any of the variables are defined in global_variable_key_value_pairs_array.
    // Getting regex again because we do not want to override any of the variables which were
    // changed by values defined in the route itself.
    StringArray original_file_vars_matches = StrRegexGetMatches(scratch.arena, *source_string, "{{[^}]*}}");

    char** trim_file_vars = PushArray(scratch.arena, char*, original_file_vars_matches.count);
    for (int index = 0; index < original_file_vars_matches.count; index++) {
        char* temp = RemoveWhitespaceFrontAndBack(scratch.arena, original_file_vars_matches.array[index], 2, 2);
        // Remove `}}` from the string.
        temp[strlen(temp)-2] = 0;
        // Remove `{{` from the string.
        temp += 2;
        trim_file_vars[index] = temp;
    }

    if (variables != NULL) {
        printf("[GET PROCESSSING]: %s\n", cJSON_Print(variables));

        for (int i = 0; i < original_file_vars_matches.count; i++) {
            cJSON* elem = NULL;
            cJSON_ArrayForEach(elem, variables) {
                if (cJSON_IsObject(elem)) {
                    printf("[ERROR] HTTP_TemplateText() The variables JSON object you passed in contains nested objects which is not supported, please pass in a single JSON object.\n");
                    DeleteScratch(scratch);
                    return;
                }
                else if (cJSON_IsArray(elem)) {
                    printf("[ERROR] HTTP_TemplateText() The variables JSON object you passed in contains an array which is not supported, please pass in a single JSON object.\n");
                    DeleteScratch(scratch);
                    return;
                }
                if (!strcmp(elem->string, trim_file_vars[i])) {
                    char* string_buf = HTTP_cJSON_GetStringValue(scratch.arena, elem);
                    *source_string = StrReplaceSubstringAllOccurance(arena, *source_string, original_file_vars_matches.array[i], string_buf);
                }
            }
        }
    }

    source->count = strlen(*source_string);

    DeleteScratch(scratch);
}

String HTTP_TemplateTextFromFile(Arena* arena, HTTPRequestInfo* request_info, cJSON* variables, char* file_path)  {
    String file_contents = HTTP_GetFileContents(arena, file_path);
    if (file_contents.string) {
        HTTP_TemplateText(arena, request_info, variables, &file_contents);
    }
    return file_contents;
}

void HTTP_AddHeaderToHeaderDict(Arena* arena, HeaderDict* header_dict, char* key, char* value) {
    header_dict->keys[header_dict->count] = HTTP_StringDup(arena, key);
    header_dict->values[header_dict->count] = HTTP_StringDup(arena, value);
    header_dict->count++;
}

void HTTP_AddCookieToCookieJar(Arena* arena,
                               CookieJar* cookie_jar,
                               char* key,
                               char* value,
                               int64_t max_age,
                               char* expires,
                               char* path,
                               char* domain,
                               bool secure,
                               bool http_only) {

    if (key == NULL) {
        printf("[ERROR] HTTP_AddCookieToCookieJar() can't add cookie to cookie jar as the key is NULL.\n");
        return;
    }

    if (value == NULL) {
        printf("[ERROR] HTTP_AddCookieToCookieJar() can't add cookie to cookie jar as the value is NULL.\n");
        return;
    }

    cookie_jar->cookies[cookie_jar->count] = (Cookie) {
        .key       = key ? HTTP_StringDup(arena, key) : NULL,
        .value     = value ? HTTP_StringDup(arena, value) : NULL,
        .max_age   = max_age,
        .expires   = expires ? HTTP_StringDup(arena, expires) : NULL,
        .path      = path ? HTTP_StringDup(arena, path) : NULL,
        .domain    = domain ? HTTP_StringDup(arena, domain) : NULL,
        .secure    = secure,
        .http_only = http_only
    };

    cookie_jar->count++;
}


void* Arena_cJSONMalloc(size_t size) {
    return ArenaAllocAligned(allocator.recycle_arena, size, sizeof(char), sizeof(void*));
}

void Arena_cJSONFree(void* object) {
    return;
}

int HTTP_RunServer(char* server_ip_address, char* server_port) {
	// Setting up and creating server socket.
	WSADATA wsaData;
	int init_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (init_result != 0) {
		printf("[SERVER] WSAStartup() failed: %d\n", init_result);
		return 1;
	}

	struct addrinfo* result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	init_result = getaddrinfo(server_ip_address, server_port, &hints, &result);
	if (init_result != 0) {
		printf("[SERVER] getaddrinfo() failed: %d\n", init_result);
		WSACleanup();
		return 1;
	}

	SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listen_socket == INVALID_SOCKET) {
		printf("[SERVER] Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	init_result = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
	if (init_result == SOCKET_ERROR) {
		printf("[SERVER] bind() failed with the error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	if (listen(listen_socket, 5) == SOCKET_ERROR) {
		printf("[SERVER] listen() failed with error: %ld\n", WSAGetLastError());
	}
	else {
		printf("[SERVER] Listening on IP Address: %s, and Port Number: %s\n", server_ip_address, server_port);
	}

    cJSON_Hooks cjson_arena_hooks = { Arena_cJSONMalloc, Arena_cJSONFree };
    cJSON_InitHooks(&cjson_arena_hooks);

	while (true) {
		SOCKET client_socket = accept(listen_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET) {
			printf("[SERVER] accept() failed: %d\n", WSAGetLastError());
			closesocket(listen_socket);
			WSACleanup();
			return 1;
		}

		char receiving_buffer[MAX_HTTP_REQUEST_SIZE] = {0};
		int receiving_buffer_len = MAX_HTTP_REQUEST_SIZE;
		int init_send_result = 1;

		init_result = recv(client_socket, receiving_buffer, receiving_buffer_len, 0);
		if (init_result > 0) {
            ArenaDealloc(allocator.recycle_arena);

			/* printf("[SERVER] Bytes received: %d\n", init_result); */
			printf("[SERVER] Data Received: %s\n", receiving_buffer);

			// Parsing HTTP Requset
            HTTPRequestInfo request_info = { 
                .contains_query_string = false,
                .is_json_request = false
            };

			// Getting HTTP request method.
			int index = 0;
			for (;receiving_buffer[index] != ' '; index++) {
				request_info.request_method[index] = receiving_buffer[index];
			}

			// Getting the route.
			index++;
			for (int route_index = 0; receiving_buffer[index] != ' '; index++, route_index++) {
				request_info.original_route[route_index] = receiving_buffer[index];
			}

			request_info.decoded_route = DecodeURL(allocator.recycle_arena, request_info.original_route);

            // If the decoded_route contains a `?` then the actual route is the text
            // before it.
            for (int i = 0; i < strlen(request_info.decoded_route); i++) {
                if (request_info.decoded_route[i] == '?') {
                    request_info.contains_query_string = true;
                    request_info.decoded_route[i] = 0;
                    strcpy(request_info.stripped_route, request_info.decoded_route);
                    request_info.decoded_route[i] = '?';
                    strcpy(request_info.query_string, request_info.decoded_route+i+1);
                    break;
                }
            }

            if (request_info.contains_query_string) {
                request_info.route_to_use = request_info.stripped_route;
            }
            else {
                request_info.route_to_use = request_info.decoded_route;
            }

			// Generating array of header keys and values
			while (receiving_buffer[index] != '\r' && receiving_buffer[index+1] != '\n') {
				index++;
			}
			index += 2;

			request_info.headers = ParseHeaderIntoDict(allocator.recycle_arena, receiving_buffer+index);

			while (!(receiving_buffer[index]   == '\r' &&
                     receiving_buffer[index+1] == '\n' &&
                     receiving_buffer[index+2] == '\r' &&
                     receiving_buffer[index+3] == '\n')) {
                 index++;
            }

			index += 4;

            /* request_info.request_body = HTTP_StringDup(allocator.recycle_arena, receiving_buffer+index); */
            request_info.cookies.count = 0;

			for (int index = 0; index < request_info.headers.count; index ++) {
				if (!strcmp(request_info.headers.keys[index], "Content-Type")) {
					if (!strcmp(request_info.headers.values[index], "application/json")) {
						printf("------------------------------\n");
						printf("JUST GOT A JSON FORMAT REQUEST!!!\n");
						printf("------------------------------\n");
						request_info.is_json_request = true;
                    }
                    break;
				}
                else if (!strcmp(request_info.headers.keys[index], "Cookie")) {
                    printf("------------------------------\n");
                    printf("GOT COOKIES!!!\n");
                    printf("------------------------------\n");
                    request_info.cookies = ParseURIKeyValuePairString(allocator.recycle_arena, request_info.headers.values[index], ';', true);
                    for (int i = 0; i < request_info.cookies.count; i++) {
                        if (!strcmp(request_info.cookies.keys[i], "SessionID")) {
                            printf("--------- GOT SESSION ID!!! ---------\n");
                            request_info.session_id = request_info.cookies.values[i];
                        }
                    }
                }
			}

            if (!strcmp(request_info.request_method, "POST") && !request_info.is_json_request) {
                request_info.is_json_request = true;
                ConvertURIDataToJSON(&request_info, receiving_buffer+index);
            }
            else if (request_info.is_json_request) {
                request_info.json_request_body = cJSON_Parse(receiving_buffer+index);
            }
            else {
                request_info.request_body = HTTP_StringDup(allocator.recycle_arena, receiving_buffer+index);
            }

            HTTPResponse output = {0};
            output.headers.keys = PushArray(allocator.recycle_arena, char*, 50);
            output.headers.values = PushArray(allocator.recycle_arena, char*, 50);

            // NOTE: Someone on the internet said that 300 cookies were the maximum per website.
            output.cookie_jar.cookies = PushArray(allocator.recycle_arena, Cookie, 300);

            output.response_body.count = -1;

            int match_id = 0;
            int match_length = 0;
            for (int i = 0; i < global_route_callback_index+1; i++) {
                if (!strcmp(request_info.request_method, global_route_callback_array[i].method)) {
                    bool is_non_regex_match = !strcmp(request_info.route_to_use, global_route_callback_array[i].route) ||
                                              !strcmp(request_info.original_route, global_route_callback_array[i].route) &&
                                              !global_route_callback_array[i].is_regex_route;

                    bool is_regex_match = global_route_callback_array[i].is_regex_route &&
                                          re_match(global_route_callback_array[i].route, request_info.route_to_use, &match_length) != -1;

                    if (is_non_regex_match || is_regex_match) {
                        // NOTE: Checking for a `global` permission first.
                        for (int x = 0; x < global_route_callback_array[i].permissions.count; x++) {
                            if (!strcmp(global_route_callback_array[i].permissions.array[x], "global")) {
                                request_info.user_permission = "global";
                                global_route_callback_array[i].response_func(allocator.recycle_arena, &request_info, &output);
                                break;
                            }
                        }

                        if (HTTP_Auth_SessionCheckIsEnabled()) {
                            if (HTTP_Auth_CheckSessionIDExists(request_info.session_id)) {
                                printf("An existing user was found at session id: `%s`\n", request_info.session_id);
                                StringArray user_specific_permissions = HTTP_Auth_StringArray_GetPermissionsAtSessionID(allocator.recycle_arena, request_info.session_id);
                                bool found_matching_permisison = false;

                                for (int x = 0; x < user_specific_permissions.count && !found_matching_permisison; x++) {
                                    for (int y = 0; y < global_route_callback_array[i].permissions.count; y++) {
                                        if (!strcmp(global_route_callback_array[i].permissions.array[y], user_specific_permissions.array[x])) {
                                            printf("----------------------------------\n");
                                            printf("Found matching permission at: user_specific_permissions: `%s`, global_route_callback_array: `%s`\n", user_specific_permissions.array[x], global_route_callback_array[i].permissions.array[y]);
                                            printf("----------------------------------\n");
                                            request_info.user_permission = user_specific_permissions.array[x];
                                            global_route_callback_array[i].response_func(allocator.recycle_arena, &request_info, &output);
                                            found_matching_permisison = true;
                                            break;
                                        }
                                    }
                                }    

                            }

                        }
                    }
                }
            }

            
            bool invalid_response = false;
			if (output.response_body.count == -1 || output.response_body.string == NULL) {
                invalid_response = true;
			}

            if (!invalid_response) {
                bool contains_content_type_header = false;

                for (int index = 0; index < output.headers.count; index++) {
                    if (!strcmp(output.headers.keys[index], "Content-Type")) {
                        contains_content_type_header = true;
                    }
                }

                if (!contains_content_type_header) {
                    printf("[SERVER] No `Content-Type` header specified for response at route `%s`, sending 404 page.\n", request_info.route_to_use);
                    HTTP_Send404Page(client_socket, request_info.original_route);
                    closesocket(client_socket);
                    continue;
                }
            }


            if (invalid_response) {
                // Checking if the route can be mapped to a path to an existing file.
                char file_type[12] = {0};
                char file_path[512] = {0};
                char content_type[100] = {0};
                bool found_match = false;

                if (HTTP_ExtractFileTypeFromFilePath(request_info.route_to_use, file_type)) {
                    for (int i = 0; i < search_dirs_size; i++) {
                        if (search_dirs[i][strlen(search_dirs[i])-1] != '/' &&
                            search_dirs[i][strlen(search_dirs[i])-1] != '\\') {

                            sprintf(file_path, "%s/%s", search_dirs[i], request_info.route_to_use+1);
                        }
                        else {
                            sprintf(file_path, "%s%s", search_dirs[i], request_info.route_to_use+1);
                        }

                        if (HTTP_FileExists(file_path)) {
                            output.response_body = HTTP_GetFileContents(allocator.recycle_arena, file_path);
                            HTTP_ConvertFileExtensionToContentType(content_type, file_type);
                            HTTP_AddHeaderToHeaderDict(allocator.recycle_arena, &output.headers, "Content-Type", content_type);
                            found_match = true;
                            break;
                        }
                    }

                }

                if (!found_match) {
                    HTTP_Send404Page(client_socket, request_info.original_route);
                    closesocket(client_socket);
                    continue;
                }
            }


            char* http_response_header = CreateResponseHeader(allocator.recycle_arena, output.status_code, output.headers, output.cookie_jar);
			send(client_socket, http_response_header, (int)strlen(http_response_header), 0);
			send(client_socket, output.response_body.string, output.response_body.count, 0);

			printf("[SERVER] Bytes sent: %d\n", init_send_result);
			printf("[SERVER] Data sent (HEADER): %s\n", http_response_header);
			printf("[SERVER] Data sent (DATA): %s\n", output.response_body.string);
			printf("[SERVER] Data sent (Strlen DATA): %zd\n", strlen(output.response_body.string));
			printf("[SERVER] Data sent (Predicted DATA): %lld\n", output.response_body.count);
		}
		else if (init_result == 0) {
			printf("[SERVER] Connection gracefully closing...\n");
		}
		else {
			printf("[SERVER] recv failed: %d\n", WSAGetLastError());
			closesocket(client_socket);
			WSACleanup();
			return 1;
		}

		closesocket(client_socket);

	}

	/*
	init_result = shutdown(client_socket, SD_SEND);
	if (init_result == SOCKET_ERROR) {
		printf("[SERVER] shutdown() failed: %d\n", WSAGetLastError());
		closesocket(client_socket);
		WSACleanup();
		return 1;
	}

	closesocket(client_socket);
	WSACleanup();
	*/
	return 0;
}
