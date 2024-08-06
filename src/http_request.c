#include "global.h"
#include "http_request.h"
#include "string_handling.h"
#include "file_handling.h"

#define ResizeArrayIfFull(array, index, num_of_added_elements, MAX_SIZE, size_per_element) \
	if (index > MAX_SIZE-1) {\
		printf("Realloc %s.\n", #array);\
		num_of_added_elements++;\
		array = realloc(array, size_per_element*(MAX_SIZE+num_of_added_elements));\
	}


void HTTP_Initialize(void) {
	route_info_array = malloc(sizeof(HTTPRouteAndFilePath)*INITIAL_ROUTE_INFO_ARRAY_SIZE);
	memset(route_info_array, 0, sizeof(HTTPRouteAndFilePath)*INITIAL_ROUTE_INFO_ARRAY_SIZE);
	route_info_array_index = -1;
	num_of_added_elements = 0;

	global_route_and_json_array = malloc(sizeof(HTTPRouteJSON)*INITIAL_GLOBAL_ROUTE_AND_JSON_ARRAY_SIZE);
	memset(global_route_and_json_array, 0, sizeof(HTTPRouteJSON)*INITIAL_GLOBAL_ROUTE_AND_JSON_ARRAY_SIZE);
	global_route_and_json_index = -1;
	global_route_and_json_added_elements = 0;
}


bool HTTP_HandleRoute(char* method, char* route, char* path_to_data) {
	// Checking if the route is in the correct format.
	if (route[0] != '/') {
		printf("[ERROR] HTTP_HandleRoute() route given: `%s` is not in the correct format.\n", route);
		return false;
	}

	// Checking if the route already exists.
	for (int index = 0; index < route_info_array_index+1; index++) {
		if (!strcmp(route_info_array[index].route, route)) {
			printf("[WARNING] HTTP_HandleRoute() data for route: %s already exists.\n", route);
			return false;
		}
	}

	// Checking if the file exists.
	if (HTTP_FindFileSize(path_to_data) == -1) {
		printf("[ERROR] HTTP_HandleRoute() No file exists at for file path: %s\n", path_to_data);
		return false;
	}

	route_info_array_index++;
	ResizeArrayIfFull(route_info_array, route_info_array_index, num_of_added_elements, INITIAL_ROUTE_INFO_ARRAY_SIZE, sizeof(HTTPRouteAndFilePath));

	global_route_and_json_index++;
	ResizeArrayIfFull(global_route_and_json_array, global_route_and_json_index, global_route_and_json_added_elements, INITIAL_GLOBAL_ROUTE_AND_JSON_ARRAY_SIZE, sizeof(HTTPRouteJSON));

    char* alloc_route = malloc(strlen(route)+1);
    strcpy(alloc_route, route);

	route_info_array[route_info_array_index] = (HTTPRouteAndFilePath) {
		.route = alloc_route,
		.path_to_file = path_to_data
	};

	cJSON* route_json_data_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(route_json_data_obj, "file", path_to_data);
	global_route_and_json_array[global_route_and_json_index] = (HTTPRouteJSON) {
		.route = route,
		.data = route_json_data_obj
	};

	printf("Added Route: %s, Path to File: %s\n", route_info_array[route_info_array_index].route, route_info_array[route_info_array_index].path_to_file);
	return true;
}

void HTTP_SetSearchDirectories(char* dirs[], size_t dirs_size) {
	if (dirs != NULL) {
		search_dirs = malloc(sizeof(char*)*dirs_size);
		search_dirs_size = dirs_size;
		for (int index = 0; index < search_dirs_size; index++) {
			search_dirs[index] = malloc(strlen(dirs[index]));
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
        put_request_default_dir = malloc(strlen(default_dir)+1);
        strcpy(put_request_default_dir, default_dir);
    }
    else {
        put_request_default_dir = malloc(4);
        strcpy(put_request_default_dir, "");
    }
}

bool HTTP_HandleRedirectRoute(char* method, char* origin_route, char* redirect_route) {
	// Checking if the route is in the correct format.
	if (origin_route[0] != '/') {
		printf("[ERROR] HTTP_HandleRedirectRoute() origin route given: `%s` is not in the correct format.\n", origin_route);
		return false;
	}

	bool redirect_route_exists = false;
	int redirect_route_index;

	for (int index = 0; index < route_info_array_index+1; index++) {
		if (!strcmp(route_info_array[index].route, origin_route)) {
			printf("[ERROR] HTTP_HandleRedirectRoute() origin route given already exists: `%s`\n", origin_route);
			return false;
		}
		else if (!strcmp(route_info_array[index].route, redirect_route)) {
			redirect_route_exists = true;
			redirect_route_index = index;
		}
	}

	if (!redirect_route_exists) {
		printf("[ERROR] HTTP_HandleRedirectRoute() redirection route given does not exist: %s\n", redirect_route);
		return false;
	}

	route_info_array_index++;
	ResizeArrayIfFull(route_info_array, route_info_array_index, num_of_added_elements, INITIAL_ROUTE_INFO_ARRAY_SIZE, sizeof(HTTPRouteAndFilePath));
	
	route_info_array[route_info_array_index] = (HTTPRouteAndFilePath) {
		.route = origin_route,
		.path_to_file = route_info_array[redirect_route_index].path_to_file
	};

	printf("Added Redirect Route: origin route: %s, redirect route: %s\n", origin_route, redirect_route);
	return true;
}

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

static char* HTTP_GenHeaderFromFile(char* path_to_file) {
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

	char* http_response_header = malloc(1024);
	memset(http_response_header, 0, 1024);
	sprintf(http_response_header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);
	return http_response_header;
}

// TODO: Allow the user to send a custom 404 page.
void HTTP_Send404Page(SOCKET client_socket, char* route) {
	char* http_error_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><h1>404 couldn't find requested resource.</h1></html>";
	send(client_socket, http_error_header, (int)strlen(http_error_header), 0);
	printf("[SERVER] Sent 404 message for route: %s\n", route);
}

static void HTTP_ConvertContentTypeToFileExtension(char* content_type, char* file_type) {
	if (!strcmp(content_type, "text/html")) {
		strcpy(file_type, "html");
	}
	else if (!strcmp(content_type, "text/css")) {
		strcpy(file_type, "css");
	}
	else if (!strcmp(content_type, "text/javascript")) {
		strcpy(file_type, "js");
	}
	else if (!strcmp(content_type, "text/markdown")) {
		strcpy(file_type, "md");
	}
	else if (!strcmp(content_type, "application/json")) {
		strcpy(file_type, "json");
	}
	else if (!strcmp(content_type, "image/apng")) {
		strcpy(file_type, "apng");
	}
	else if (!strcmp(content_type, "image/avif")) {
		strcpy(file_type, "avif");
	}
	else if (!strcmp(content_type, "image/jpeg")) {
		strcpy(file_type, "jpeg");
	}
	else if (!strcmp(content_type, "image/jpg")) {
		strcpy(file_type, "jpg");
	}
	else if (!strcmp(content_type, "image/png")) {
		strcpy(file_type, "png");
	}
	else if (!strcmp(content_type, "image/svg+xml")) {
		strcpy(file_type, "svg");
	}
	else if (!strcmp(content_type, "image/webp")) {
		strcpy(file_type, "webp");
	}
	else if (!strcmp(content_type, "image/x-ico")) {
		strcpy(file_type, "ico");
	}
}

static void HTTP_ParsePUTRequest(HTTPRequestInfo* request_info, bool is_json_response) {
    char* request_route;
    if (request_info->contains_query_string) {
        request_route = request_info->stripped_route;
    }
    else {
        request_route = request_info->decoded_route;
    }

	printf("[PUT Request Data] %s\n", request_info->request_body);

	// Resizing global_route_and_json_array if it is full
	ResizeArrayIfFull(global_route_and_json_array, global_route_and_json_index, global_route_and_json_added_elements, INITIAL_GLOBAL_ROUTE_AND_JSON_ARRAY_SIZE, sizeof(HTTPRouteJSON));

    bool route_exists = false;
    int route_index = 0;
    for (; route_index < global_route_and_json_index+1; route_index++) {
        if (!strcmp(global_route_and_json_array[route_index].route, request_route)) {
            route_exists = true;
            break;
        }
    }

    if (is_json_response) {
        if (route_exists) {
            // Overwriting JSON data at route if it exists.
            cJSON_Delete(global_route_and_json_array[route_index].data);
            global_route_and_json_array[route_index].data = cJSON_Parse(request_info->request_body);
        }
        else {
            // Creating a new route for JSON data if it doesn't exist.
            global_route_and_json_index++;

            global_route_and_json_array[global_route_and_json_index] = (HTTPRouteJSON) {
                .route = strdup(request_route),
                .data = cJSON_Parse(request_info->request_body)
            };
        }

        printf("[PUT Request Processing]: %s\n", cJSON_Print(global_route_and_json_array[route_index].data));
    }
    else {
        char file_type[48] = {0};
        HTTP_ConvertContentTypeToFileExtension(request_info->content_type, file_type);
    }

}

static void HTTP_ParsePOSTRequest(HTTPRequestInfo* request_info) {
    char* request_route;
    if (request_info->contains_query_string) {
        request_route = request_info->stripped_route;
    }
    else {
        request_route = request_info->decoded_route;
    }
	printf("[POST Request Data] %s\n", request_info->request_body);

	StringArray request_key_value_pairs_array = ParseURIKeyValuePairString(request_info->request_body);
	char** key_value_pairs_array = request_key_value_pairs_array.array;
	int max_key_value_pairs_array_matches = request_key_value_pairs_array.count;

	// Resizing global_route_and_json_array if it is full
	ResizeArrayIfFull(global_route_and_json_array, global_route_and_json_index, global_route_and_json_added_elements, INITIAL_GLOBAL_ROUTE_AND_JSON_ARRAY_SIZE, sizeof(HTTPRouteJSON));

	bool route_exists = false;
	int route_index = 0;
	for (; route_index < global_route_and_json_index+1; route_index++) {
		if (!strcmp(global_route_and_json_array[route_index].route, request_route)) {
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
                    cJSON* current = NULL;
					found_existing_match = true;

                    item->string = malloc(strlen(elem->string)+1);
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

static HTTPGETRequest HTTP_ParseGETRequest(HTTPRequestInfo* request_info, bool is_json_response) {
	// Responding to the request with the data in route_info_array if
	// an entry for the route exists.
	char* data_to_send = {0};
	int data_to_send_length;
	char* http_response_header = {0};
	char* path_to_file_at_route = {0};

    char* request_path;
    if (request_info->contains_query_string) {
        request_path = request_info->stripped_route;
    }
    else {
        request_path = request_info->decoded_route;
    }

    int file_path_index = 0;
    bool route_exists = false;
	for (; file_path_index < global_route_and_json_index+1; file_path_index++) {
		if (!strcmp(global_route_and_json_array[file_path_index].route, request_path)) {
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


    if (is_json_response) {
        printf("-------------------------------\n");
        printf("Sending a JSON Response at request path: %s\n", request_path);
        printf("-------------------------------\n");

        if (path_to_file_at_route == NULL) {
            return (HTTPGETRequest){NULL, NULL, 0};
        }

        http_response_header = HTTP_GenHeaderFromFile(".json");
        if (http_response_header == NULL) {
            printf("[ERROR] Couldn't generate JSON header for request path: %s\n", request_path);
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

                        item->string = malloc(strlen(elem->string)+1);
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

        if (path_to_file_at_route == NULL && HTTP_ExtractFileTypeFromFilePath(request_path, file_type)) {
            for (int index = 0; index < search_dirs_size; index++) {
                if (search_dirs[index][strlen(search_dirs[index])-1] != '/' &&
                    search_dirs[index][strlen(search_dirs[index])-1] != '\\') {

                    sprintf(temp_buffer, "%s/%s", search_dirs[index], request_path+1);
                }
                else {
                    sprintf(temp_buffer, "%s%s", search_dirs[index], request_path+1);
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
        http_response_header = HTTP_GenHeaderFromFile(path_to_file_at_route);
        if (http_response_header == NULL) {
            printf("[ERROR] Couldn't generate header for %s\n", path_to_file_at_route);
            return (HTTPGETRequest){NULL, NULL, 0};
        }

        if (request_info->contains_query_string) {
            // Using regex to extract matching variables from the file contents buffer.
            StringArray variable_matches = StrRegexGetMatches(data_to_send, "{{[^}]*}}");
            char** original_file_vars = variable_matches.array;
            int max_original_file_vars_matches = variable_matches.count;

            char** trim_file_vars = malloc(sizeof(char*)*max_original_file_vars_matches);
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

        char** trim_file_vars = malloc(sizeof(char*)*max_original_file_vars_matches);
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

			request_info.decoded_route = DecodeURL(request_info.original_route);

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

			// Generating array of header keys and values
			while (receiving_buffer[index] != '\r' && receiving_buffer[index+1] != '\n') {
				index++;
			}
			index += 2;

			StringArray parsed_header = ParseHeaderIntoKeyValuePairString(receiving_buffer+index);

			while (!(receiving_buffer[index]   == '\r' &&
                     receiving_buffer[index+1] == '\n' &&
                     receiving_buffer[index+2] == '\r' &&
                     receiving_buffer[index+3] == '\n')) {
                 index++;
            }

			index += 4;

            request_info.request_body = strdup(receiving_buffer+index);

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

			if (!strcmp(request_info.request_method, "POST")) {
				HTTP_ParsePOSTRequest(&request_info);
			}

            if (!strcmp(request_info.request_method, "PUT")) {
                HTTP_ParsePUTRequest(&request_info, require_json_response);
            }

			HTTPGETRequest output = HTTP_ParseGETRequest(&request_info, require_json_response);
			if (!output.data_to_send_length) {
				HTTP_Send404Page(client_socket, request_info.decoded_route);
				closesocket(client_socket);

                free(request_info.decoded_route);
                free(request_info.request_body);
				continue;
			}

			FreeStringArray(parsed_header.array, parsed_header.count);

			send(client_socket, output.http_response_header, (int)strlen(output.http_response_header), 0);
			send(client_socket, output.data_to_send, output.data_to_send_length, 0);

			printf("[SERVER] Bytes sent: %d\n", init_send_result);
			printf("[SERVER] Data sent (HEADER): %s\n", output.http_response_header);
			printf("[SERVER] Data sent (DATA): %s\n", output.data_to_send);
			printf("[SERVER] Data sent (Strlen DATA): %zd\n", strlen(output.data_to_send));
			printf("[SERVER] Data sent (Predicted DATA): %d\n", output.data_to_send_length);

			free(output.http_response_header);
			free(output.data_to_send);

            free(request_info.decoded_route);
            free(request_info.request_body);
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
