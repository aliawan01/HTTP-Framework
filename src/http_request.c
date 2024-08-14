#include "util.h"
#include "http_request.h"
#include "string_handling.h"
#include "file_handling.h"
#include <time.h>

void HTTP_Initialize(void) {
    // TODO: Perhaps allow the user to specify the size of the global and scratch arena.
    allocator = malloc(sizeof(Allocator));
    allocator->global_arena = malloc(sizeof(Arena));
    allocator->scratch_arena = malloc(sizeof(Arena));

    ArenaInit(allocator->global_arena, MB(500));
    ArenaInit(allocator->scratch_arena, MB(50));

	global_route_and_json_array = ArenaAlloc(allocator->global_arena, sizeof(HTTPRouteJSON)*INITIAL_GLOBAL_ROUTE_AND_JSON_ARRAY_SIZE);
	global_route_and_json_index = -1;
}


bool HTTP_HandleRoute(char* method, char* route, void (*response_func)(Allocator* allocator, HTTPRequestInfo*, HTTPResponse*)) {
	// Checking if the route is in the correct format.
	if (route[0] != '/') {
		printf("[ERROR] HTTP_HandleRoute() route given: `%s` is not in the correct format.\n", route);
		return false;
	}

	// Checking if the route already exists.
	for (int index = 0; index < global_route_and_json_index+1; index++) {
		if (!strcmp(global_route_and_json_array[index].route, route)) {
			printf("[WARNING] HTTP_HandleRoute() data for route: %s already exists.\n", route);
			return false;
		}
	}

	global_route_and_json_index++;
    Assert(global_route_and_json_index < INITIAL_GLOBAL_ROUTE_AND_JSON_ARRAY_SIZE);

	global_route_and_json_array[global_route_and_json_index] = (HTTPRouteJSON) {
        .method = HTTP_StringDup(allocator->global_arena, method),
		.route = HTTP_StringDup(allocator->global_arena, route),
        .response_func = response_func,
        .data = cJSON_CreateObject() 
	};

    // TODO: @memory_leak
	printf("Added Route: %s, Method: %s, with_resonse_func, Data: %s\n", global_route_and_json_array[global_route_and_json_index].route, global_route_and_json_array[global_route_and_json_index].method, cJSON_Print(global_route_and_json_array[global_route_and_json_index].data));
	return true;
}

void HTTP_SetSearchDirectories(char* dirs[], size_t dirs_size) {
	if (dirs != NULL) {
		search_dirs = ArenaAlloc(allocator->global_arena, sizeof(char*)*dirs_size);
		search_dirs_size = dirs_size;
		for (int index = 0; index < search_dirs_size; index++) {
			search_dirs[index] = ArenaAlloc(allocator->global_arena, strlen(dirs[index]));
			strcpy(search_dirs[index], dirs[index]);
		}
	}
}


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
        put_request_default_dir = ArenaAlloc(allocator->global_arena, strlen(default_dir)+1);
        strcpy(put_request_default_dir, default_dir);
    }
    else {
        put_request_default_dir = ArenaAlloc(allocator->global_arena, 4);
        strcpy(put_request_default_dir, "");
    }
}

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


// TODO: Needs to be able to create a new \r\n entry for each header.
static char* HTTP_CreateResponseHeader(Allocator* allocator, enum HTTPStatusCode status_code, char* headers) {
	char* http_response_header = ArenaAlloc(allocator->global_arena, 1024);
	sprintf(http_response_header, "HTTP/1.1 %s\r\n%s\r\n\r\n", HTTP_StatusCodeStrings[status_code], headers);
    
	return http_response_header;
}

static char* HTTP_CreateResponseHeaderFromFile(Allocator* allocator, char* path_to_file, bool created_new_entry) {
	// Extracting file type into a separate string.
	char file_type[24] = {0};
	HTTP_ExtractFileTypeFromFilePath(path_to_file, file_type);

	// Generating Header based on file type.
	char* content_type = NULL;
	if (!strcmp(file_type, "html")) {
		content_type = "text/html";
	}
	else if (!strcmp(file_type, "css")) {
		content_type = "text/css";
	}
	else if (!strcmp(file_type, "js")) {
		content_type = "text/javascript";
	}
	else if (!strcmp(file_type, "md")) {
		content_type = "text/markdown";
	}
	else if (!strcmp(file_type, "markdown")) {
		content_type = "text/markdown";
	}
	else if (!strcmp(file_type, "json")) {
		content_type = "application/json";
	}
	else if (!strcmp(file_type, "apng")) {
		content_type = "image/apng";
	}
	else if (!strcmp(file_type, "avif")) {
		content_type = "image/avif";
	}
	else if (!strcmp(file_type, "jpeg")) {
		content_type = "image/jpeg";
	}
	else if (!strcmp(file_type, "jpg")) {
		content_type = "image/jpg";
	}
	else if (!strcmp(file_type, "png")) {
		content_type = "image/png";
	}
	else if (!strcmp(file_type, "svg")) {
		content_type = "image/svg+xml";
	}
	else if (!strcmp(file_type, "webp")) {
		content_type = "image/webp";
	}
	else if (!strcmp(file_type, "ico")) {
		content_type = "image/x-ico";
	}

	if (content_type == NULL) {
		return NULL;
	}


    int status_code;
    if (created_new_entry) {
        status_code = 201;
    }
    else {
        status_code = 200;
    }

	char* http_response_header = ArenaAlloc(allocator->global_arena, 1024);
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

#if 0
static bool HTTP_ParsePUTRequest(Arena* arena, HTTPRequestInfo* request_info, bool is_json_response) {
	printf("[PUT Request Data] %s\n", request_info->request_body);
    bool created_new_entry = false;

    bool route_exists = false;
    int route_index = 0;
    for (; route_index < global_route_and_json_index+1; route_index++) {
        if (!strcmp(global_route_and_json_array[route_index].route, request_info->route_to_use)) {
            route_exists = true;
            break;
        }
    }

    if (is_json_response) {
        if (route_exists) {
            // Overwriting JSON data at route if it exists.
            cJSON_Delete(global_route_and_json_array[route_index].data);
            global_route_and_json_array[route_index].data = cJSON_Parse(request_info->request_body);
            printf("[PUT Request Processing]: %s\n", cJSON_Print(global_route_and_json_array[route_index].data));
        }
        else {
            // Creating a new route for JSON data if it doesn't exist.
            created_new_entry = true;
            global_route_and_json_index++;

            global_route_and_json_array[global_route_and_json_index] = (HTTPRouteJSON) {
                .route = HTTP_StringDup(arena, request_info->route_to_use),
                .data = cJSON_Parse(request_info->request_body)
            };
            printf("[PUT Request Processing]: %s\n", cJSON_Print(global_route_and_json_array[global_route_and_json_index].data));
        }

    }
    else {
        char file_extension[24] = {0};
        HTTP_ConvertContentTypeToFileExtension(request_info->content_type, file_extension);
        char file_name[256] = {0};
        if (!strcmp(request_info->route_to_use, "/")) {
            strcpy(file_name, "root");
        }
        else {
            strcpy(file_name, request_info->route_to_use+1);
        }

        char full_file_path[300] = {0};
        if (put_request_default_dir[0] == 0) {
            sprintf(full_file_path, "%s.%s", file_name, file_extension);
        }
        else {
            sprintf(full_file_path, "%s/%s.%s", put_request_default_dir, file_name, file_extension);
        }

        if (HTTP_FindFileSize(full_file_path) == -1) {
            printf("[PUTS Request Handler] Couldn't find an existing file at file path `%s` creating a new file.\n", full_file_path);
        }

        // TODO: Find a better way to do logging.
        if (HTTP_OverwriteFileContents(full_file_path, request_info->request_body)) {
            printf("[PUTS Request Handler] Successfully overwrote data in file at file path `%s`.\n", full_file_path);
        }

        if (route_exists) {
            cJSON* elem = NULL;
            bool found_existing_match = false;
            cJSON_ArrayForEach(elem, global_route_and_json_array[route_index].data) {
                if (!strcmp(elem->string, "file")) {
					found_existing_match = true;
                    cJSON_SetValuestring(elem, full_file_path);
                    break;
                }
            }

            if (!found_existing_match) {
                cJSON_AddItemToObject(global_route_and_json_array[route_index].data, "file", cJSON_CreateString(full_file_path));
            }
            printf("[PUT Request Processing]: %s\n", cJSON_Print(global_route_and_json_array[route_index].data));
        }
        else {
            printf("[PUT Request] Created a new entry in global_route_and_json_array for file at file path `%s`.\n", full_file_path);

            created_new_entry = true;
            cJSON* item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "file", full_file_path);

            global_route_and_json_index++;
            global_route_and_json_array[global_route_and_json_index] = (HTTPRouteJSON) {
                .route = HTTP_StringDup(arena, request_info->route_to_use),
                .data = item
            };

            printf("[PUT Request Processing]: %s\n", cJSON_Print(global_route_and_json_array[global_route_and_json_index].data));
        }

    }

    return created_new_entry;
}

static bool HTTP_ParseDELETERequest(HTTPRequestInfo* request_info) {
    bool successfully_deleted_entry = false;
	printf("[DELETE Request] %s\n", request_info->request_body);
    for (int index = 0; index < global_route_and_json_index+1; index++) {
        if (!strcmp(request_info->route_to_use, global_route_and_json_array[index].route)) {
            printf("[DELETE Request] Found a matching route at index: %d\n", index);
            // TODO: @memory_leak
            printf("[DELETE Request] Data is route: `%s`, JSON Data: %s\n", global_route_and_json_array[index].route, cJSON_Print(global_route_and_json_array[index].data));

            successfully_deleted_entry = true;
            free(global_route_and_json_array[index].route);
            cJSON_Delete(global_route_and_json_array[index].data);

            // Checking if there was only 1 element in global_route_and_json_array initially.
            if (global_route_and_json_index-1 == -1) {
                global_route_and_json_index--;
            }
            else {
                memmove(global_route_and_json_array, global_route_and_json_array+1, sizeof(HTTPRouteJSON)*global_route_and_json_index);
                memset(global_route_and_json_array+global_route_and_json_index, 0, sizeof(HTTPRouteJSON));
                global_route_and_json_index--;
            }
        }
    }

    return successfully_deleted_entry;
}

static void HTTP_ParsePOSTRequest(Arena* arena, HTTPRequestInfo* request_info) {
	printf("[POST Request Data] %s\n", request_info->request_body);

	StringArray request_key_value_pairs_array = ParseURIKeyValuePairString(request_info->request_body);
	char** key_value_pairs_array = request_key_value_pairs_array.array;
	int max_key_value_pairs_array_matches = request_key_value_pairs_array.count;

	bool route_exists = false;
	int route_index = 0;
	for (; route_index < global_route_and_json_index+1; route_index++) {
		if (!strcmp(global_route_and_json_array[route_index].route, request_info->route_to_use)) {
			route_exists = true;
			break;
		}
	}

	bool found_existing_match = false;
	if (route_exists && request_info->request_body[0] != 0) {
		for (int index = 0; index < max_key_value_pairs_array_matches; index += 2) {
			found_existing_match = false;
            
            cJSON* item = NULL;
            if (IsInteger(key_value_pairs_array[index+1])) {
                char* endptr;
                item = cJSON_CreateNumber(strtod(key_value_pairs_array[index+1], &endptr));
            }
            else if (!strcmp(key_value_pairs_array[index+1], "true")) {
                item = cJSON_CreateBool(1);
            }
            else if (!strcmp(key_value_pairs_array[index+1], "false")) {
                item = cJSON_CreateBool(0);
            }
            else if (!strcmp(key_value_pairs_array[index+1], "null")) {
                item = cJSON_CreateNull();
            }
            else {
                char* decoded_string = DecodeURL(key_value_pairs_array[index+1]);
                item = cJSON_CreateString(decoded_string);
                free(decoded_string);
            }
            // TODO: Support arrays?

			cJSON* elem = NULL;
			cJSON_ArrayForEach(elem, global_route_and_json_array[route_index].data) {
				if (!strcmp(elem->string, key_value_pairs_array[index])) {
					found_existing_match = true;

                    item->string = ArenaAlloc(arena, strlen(elem->string)+1);
                    strcpy(item->string, elem->string);
                    cJSON_ReplaceItemViaPointer(global_route_and_json_array[route_index].data, elem, item);
                    elem = item;
                    break;
				}
			}

			if (!found_existing_match) {
                cJSON_AddItemToObject(global_route_and_json_array[route_index].data, key_value_pairs_array[index], item);
			}
		}
	}

	printf("[POST Request Processing]: %s\n", cJSON_Print(global_route_and_json_array[route_index].data));

	FreeStringArray(key_value_pairs_array, max_key_value_pairs_array_matches);
}

static HTTPGETRequest HTTP_ParseGETRequest(Arena* arena, HTTPRequestInfo* request_info, bool is_json_response, bool created_new_entry, bool deleted_entry) {
	char* data_to_send = {0};
	int data_to_send_length;
	char* http_response_header = {0};
	char* path_to_file_at_route = {0};

    int file_path_index = 0;
    bool route_exists = false;
	for (; file_path_index < global_route_and_json_index+1; file_path_index++) {
		if (!strcmp(global_route_and_json_array[file_path_index].route, request_info->route_to_use)) {
            route_exists = true;
            cJSON* elem = NULL;
            cJSON_ArrayForEach(elem, global_route_and_json_array[file_path_index].data) {
                if (!strcmp(elem->string, "file")) {
                    path_to_file_at_route = elem->valuestring;
                }
            }
            break;
		}
	}


    if (deleted_entry) {
        http_response_header = HTTP_StringDup(arena, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        data_to_send = HTTP_StringDup(arena, "<html><h1>Successfully deleted the resource you told us to delete.</h1></html>");
        data_to_send_length = strlen(data_to_send);
    }
    else if (is_json_response) {
        printf("-------------------------------\n");
        printf("Sending a JSON Response at request path: %s\n", request_info->route_to_use);
        printf("-------------------------------\n");

        http_response_header = HTTP_CreateResponseHeaderFromFile(".json", created_new_entry);
        if (http_response_header == NULL) {
            printf("[ERROR] Couldn't generate JSON header for request path: %s\n", request_info->route_to_use);
            return (HTTPGETRequest){NULL, NULL, 0};
        }
        
        char* json_obj_string = cJSON_PrintUnformatted(global_route_and_json_array[file_path_index].data);
        cJSON* json_obj = cJSON_Parse(json_obj_string);

        if (request_info->contains_query_string) {
            // Extracting variables and values into key_value_pairs_array;
            StringArray request_key_value_pairs_array = ParseURIKeyValuePairString(request_info->query_string);
            char** key_value_pairs_array = request_key_value_pairs_array.array;
            int max_key_value_pairs_array_matches = request_key_value_pairs_array.count;

            bool found_match = false;
            for (int key_index = 0; key_index < max_key_value_pairs_array_matches; key_index += 2) {
                found_match = false;

                cJSON* item = NULL;
                if (IsInteger(key_value_pairs_array[key_index+1])) {
                    char* endptr;
                    item = cJSON_CreateNumber(strtod(key_value_pairs_array[key_index+1], &endptr));
                }
                else if (!strcmp(key_value_pairs_array[key_index+1], "true")) {
                    item = cJSON_CreateBool(1);
                }
                else if (!strcmp(key_value_pairs_array[key_index+1], "false")) {
                    item = cJSON_CreateBool(0);
                }
                else if (!strcmp(key_value_pairs_array[key_index+1], "null")) {
                    item = cJSON_CreateNull();
                }
                else {
                    char* decoded_string = DecodeURL(key_value_pairs_array[key_index+1]);
                    item = cJSON_CreateString(decoded_string);
                    free(decoded_string);
                }

                // Changing the values of existing key-value pairs in the json object with the value specified in the route.
                cJSON* elem = NULL;
                cJSON_ArrayForEach(elem, json_obj) {
                    if (!strcmp(elem->string, key_value_pairs_array[key_index])) {
                        found_match = true;

                        item->string = ArenaAlloc(arena, strlen(elem->string)+1);
                        strcpy(item->string, elem->string);
                        cJSON_ReplaceItemViaPointer(json_obj, elem, item);
                        elem = item;
                    }
                }

                // Creating new key-value pairs in the json object with the key-value pair specified in the route.
                if (!found_match) {
                    cJSON_AddItemToObject(json_obj, key_value_pairs_array[key_index], item);
                }
            }

            FreeStringArray(key_value_pairs_array, max_key_value_pairs_array_matches);
            data_to_send = cJSON_Print(json_obj);
            data_to_send_length = strlen(data_to_send);
            free(json_obj_string);
        }
        else {
            data_to_send = json_obj_string;
            data_to_send_length = strlen(json_obj_string);
        }

        cJSON_Delete(json_obj);
    }
    else {
        // Checking if the route can be mapped to a path to an existing file.
        char file_type[10] = {0};
        char temp_buffer[256] = {0};

        if (path_to_file_at_route == NULL && HTTP_ExtractFileTypeFromFilePath(request_info->route_to_use, file_type)) {
            for (int index = 0; index < search_dirs_size; index++) {
                if (search_dirs[index][strlen(search_dirs[index])-1] != '/' &&
                    search_dirs[index][strlen(search_dirs[index])-1] != '\\') {

                    sprintf(temp_buffer, "%s/%s", search_dirs[index], request_info->route_to_use+1);
                }
                else {
                    sprintf(temp_buffer, "%s%s", search_dirs[index], request_info->route_to_use+1);
                }
                int file_size = HTTP_FindFileSize(temp_buffer);
                if (file_size != -1) {
                    path_to_file_at_route = temp_buffer;
                    break;
                }
            }
        }


        if (path_to_file_at_route == NULL) {
            return (HTTPGETRequest){NULL, NULL, 0};
        }

        data_to_send = HTTP_GetFileContents(path_to_file_at_route);
        data_to_send_length = HTTP_FindFileSize(path_to_file_at_route);

        // File was deleted or it's name was changed during runtime.
        if (data_to_send_length == -1) {
            return (HTTPGETRequest){NULL, NULL, 0};
        }

        // Not a recognised file type.
        http_response_header = HTTP_CreateResponseHeaderFromFile(path_to_file_at_route, created_new_entry);
        if (http_response_header == NULL) {
            printf("[ERROR] Couldn't generate header for %s\n", path_to_file_at_route);
            return (HTTPGETRequest){NULL, NULL, 0};
        }

        if (request_info->contains_query_string) {
            // Using regex to extract matching variables from the file contents buffer.
            StringArray variable_matches = StrRegexGetMatches(data_to_send, "{{[^}]*}}");
            char** original_file_vars = variable_matches.array;
            int max_original_file_vars_matches = variable_matches.count;

            char** trim_file_vars = ArenaAlloc(arena, sizeof(char*)*max_original_file_vars_matches);
            for (int index = 0; index < max_original_file_vars_matches; index++) {
                char* temp = RemoveWhitespaceFrontAndBack(original_file_vars[index], 2, 2);
                // Remove `}}` from the string.
                temp[strlen(temp)-2] = 0;
                trim_file_vars[index] = temp;
            }

            // Extracting variables and values into key_value_pairs_array;
            StringArray request_key_value_pairs_array = ParseURIKeyValuePairString(request_info->query_string);
            char** key_value_pairs_array = request_key_value_pairs_array.array;
            int max_key_value_pairs_array_matches = request_key_value_pairs_array.count;


            // Replacing variables in the file contents buffer with their values
            for (int file_var_index = 0; file_var_index < max_original_file_vars_matches; file_var_index++) {
                for (int key_index = 0; key_index < max_key_value_pairs_array_matches; key_index += 2) {
                    // Adding 2 to the trimmed file variable to remove `{{` at the start.
                    if (!strcmp(trim_file_vars[file_var_index]+2, key_value_pairs_array[key_index])) {
                        StrReplaceSubstringAllOccurance(&data_to_send, original_file_vars[file_var_index], key_value_pairs_array[key_index+1]);
                    }
                }
            }

            FreeStringArray(original_file_vars, max_original_file_vars_matches);
            FreeStringArray(trim_file_vars, max_original_file_vars_matches);
            FreeStringArray(key_value_pairs_array, max_key_value_pairs_array_matches);

            data_to_send_length = strlen(data_to_send);
        }

        // Checking if any of the variables are defined in global_variable_key_value_pairs_array.
        // Getting regex again because we do not want to override any of the variables which were
        // changed by values defined in the route itself.
        StringArray variable_matches = StrRegexGetMatches(data_to_send, "{{[^}]*}}");
        char** original_file_vars = variable_matches.array;
        int max_original_file_vars_matches = variable_matches.count;

        char** trim_file_vars = ArenaAlloc(arena, sizeof(char*)*max_original_file_vars_matches);
        for (int index = 0; index < max_original_file_vars_matches; index++) {
            char* temp = RemoveWhitespaceFrontAndBack(original_file_vars[index], 2, 2);
            // Remove `}}` from the string.
            temp[strlen(temp)-2] = 0;
            trim_file_vars[index] = temp;
        }

        if (route_exists) {
            printf("[GET PROCESSSING]: %s\n", cJSON_Print(global_route_and_json_array[file_path_index].data));
            bool found_matches = false;

            for (int i = 0; i < max_original_file_vars_matches; i++) {
                cJSON* elem = NULL;
                cJSON_ArrayForEach(elem, global_route_and_json_array[file_path_index].data) {
                    if (!strcmp(elem->string, trim_file_vars[i]+2)) {
                        found_matches = true;
                        // TODO: VERY DANGEROUS, NEED TO CHECK WHAT DATA IS STORED IN THE OBJECT FIRST!
                        StrReplaceSubstringAllOccurance(&data_to_send, original_file_vars[i], elem->valuestring);
                    }
                }
            }

            if (found_matches) {
                data_to_send_length = strlen(data_to_send);
            }
        }

        FreeStringArray(original_file_vars, max_original_file_vars_matches);
        FreeStringArray(trim_file_vars, max_original_file_vars_matches);
    }

	return (HTTPGETRequest) {
		.http_response_header = http_response_header,
		.data_to_send = data_to_send,
		.data_to_send_length = data_to_send_length
	};

}
#endif

int HTTP_RunServer(char* server_ip_address, char* server_port) {
	// Setting up and creating server socket.
	WSADATA* wsaData;
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
            ScratchBegin(allocator->global_arena);

			printf("[SERVER] Bytes received: %d\n", init_result);
			printf("[SERVER] Data Received: %s\n", receiving_buffer);

			// Parsing HTTP Requset
            HTTPRequestInfo request_info = { .contains_query_string = false };

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

			request_info.decoded_route = DecodeURL(allocator->global_arena, request_info.original_route);

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

			StringArray parsed_header = ParseHeaderIntoKeyValuePairString(allocator->global_arena, receiving_buffer+index);

			while (!(receiving_buffer[index]   == '\r' &&
                     receiving_buffer[index+1] == '\n' &&
                     receiving_buffer[index+2] == '\r' &&
                     receiving_buffer[index+3] == '\n')) {
                 index++;
            }

			index += 4;

            request_info.request_body = HTTP_StringDup(allocator->global_arena, receiving_buffer+index);

			bool require_json_response = false;
			for (int index = 0; index < parsed_header.count; index += 2) {
				if (!strcmp(parsed_header.array[index], "Content-Type")) {
					if (!strcmp(parsed_header.array[index+1], "application/json")) {
						printf("------------------------------\n");
						printf("JUST GOT A JSON FORMAT REQUEST!!!\n");
						printf("------------------------------\n");
						require_json_response = true;
						break;
                    }

                    strcpy(request_info.content_type, parsed_header.array[index+1]);
				}
			}

            /* bool created_new_entry = false; */
            /* bool successfully_deleted_entry = false; */
			/* if (!strcmp(request_info.request_method, "POST")) { */
				/* HTTP_ParsePOSTRequest(&request_info); */
			/* } */

            /* if (!strcmp(request_info.request_method, "PUT")) { */
            /*     created_new_entry = HTTP_ParsePUTRequest(&request_info, require_json_response); */
            /* } */

            /* if (!strcmp(request_info.request_method, "DELETE")) { */
            /*     successfully_deleted_entry = HTTP_ParseDELETERequest(&request_info); */
            /* } */
            

			/* HTTPGETRequest output = HTTP_ParseGETRequest(&request_info, require_json_response, created_new_entry, successfully_deleted_entry); */
            /* HTTPResponse output = { .response_body_length = -1 }; */
            HTTPResponse* output = ArenaAlloc(allocator->global_arena, sizeof(HTTPResponse));
            output->response_body_length = -1;
            for (int i = 0; i < global_route_and_json_index+1; i++) {
                if (!strcmp(request_info.route_to_use, global_route_and_json_array[i].route)) {
                    if (!strcmp(request_info.request_method, global_route_and_json_array[i].method)) {
                        global_route_and_json_array[i].response_func(allocator, &request_info, output);
                    }
                }
            }

            
			if (output->response_body_length == -1 || output->response_body == NULL) {
				HTTP_Send404Page(client_socket, request_info.original_route);
				closesocket(client_socket);

                ScratchEnd(allocator->global_arena);
				continue;
			}

			/* FreeStringArray(parsed_header.array, parsed_header.count); */


            char* http_response_header = HTTP_CreateResponseHeader(allocator, output->status_code, output->headers);
			send(client_socket, http_response_header, (int)strlen(http_response_header), 0);
			send(client_socket, output->response_body, output->response_body_length, 0);

			printf("[SERVER] Bytes sent: %d\n", init_send_result);
			printf("[SERVER] Data sent (HEADER): %s\n", http_response_header);
			printf("[SERVER] Data sent (DATA): %s\n", output->response_body);
			printf("[SERVER] Data sent (Strlen DATA): %zd\n", strlen(output->response_body));
			printf("[SERVER] Data sent (Predicted DATA): %d\n", output->response_body_length);

			/* free(http_response_header); */

			/* free(output.headers); */
			/* free(output.response_body); */

            /* free(request_info.decoded_route); */
            /* free(request_info.request_body); */
            ScratchEnd(allocator->global_arena);
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
