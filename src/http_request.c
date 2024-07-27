#include "server.h"

static void HTTP_Initialize(void) {
	route_info_array = malloc(sizeof(HTTPRouteInfo)*INITIAL_ROUTE_INFO_ARRAY_SIZE);
	memset(route_info_array, 0, sizeof(HTTPRouteInfo)*INITIAL_ROUTE_INFO_ARRAY_SIZE);
	route_info_array_index = -1;
	num_of_added_elements = 0;

	global_variable_key_value_pairs_array = malloc(sizeof(char*)*INITIAL_GLOBAL_KEY_VALUE_PAIRS_ARRAY_SIZE);
	memset(global_variable_key_value_pairs_array, 0, sizeof(char*)*INITIAL_GLOBAL_KEY_VALUE_PAIRS_ARRAY_SIZE);
	global_key_value_pairs_index = -1;
	global_key_value_pairs_added_elements = 0;
}

static bool HTTP_HandleRoute(char* method, char* route, char* path_to_data) {
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
	if (route_info_array_index > INITIAL_ROUTE_INFO_ARRAY_SIZE-1) {
		printf("Realloc.\n");
		num_of_added_elements++;
		route_info_array = realloc(route_info_array, sizeof(HTTPRouteInfo)*(INITIAL_ROUTE_INFO_ARRAY_SIZE + num_of_added_elements));
	}
	
	route_info_array[route_info_array_index] = (HTTPRouteInfo) {
		.route = route,
		.path_to_file = path_to_data
	};
	printf("Added Route: %s, Path to File: %s\n", route_info_array[route_info_array_index].route, route_info_array[route_info_array_index].path_to_file);
	return true;
}

static void HTTP_SetSearchDirectories(char* dirs[], size_t dirs_size) {
	if (dirs != NULL) {
		search_dirs = malloc(sizeof(char*)*dirs_size);
		search_dirs_size = dirs_size;
		for (int index = 0; index < search_dirs_size; index++) {
			search_dirs[index] = malloc(strlen(dirs[index]));
			strcpy(search_dirs[index], dirs[index]);
		}
	}
}

static bool HTTP_HandleRedirectRoute(char* method, char* origin_route, char* redirect_route) {
	// Checking if the route is in the correct format.
	if (origin_route[0] != '/') {
		printf("[ERROR] HTTP_HandleRedirectRoute() origin route given: `%s` is not in the correct format.\n", origin_route);
		return false;
	}

	bool redirect_route_exists = false;
	int redirect_route_index;

	for (int index = 0; index < route_info_array_index+1; index++) {
		if (!strcmp(route_info_array[index].route, origin_route)) {
			printf("[ERROR] HTTP_HandleRedirectRoute() origin route given already exists: %s\n", origin_route);
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
	if (route_info_array_index > INITIAL_ROUTE_INFO_ARRAY_SIZE-1) {
		printf("Realloc.\n");
		num_of_added_elements++;
		route_info_array = realloc(route_info_array, sizeof(HTTPRouteInfo)*(INITIAL_ROUTE_INFO_ARRAY_SIZE + num_of_added_elements));
	}
	
	route_info_array[route_info_array_index] = (HTTPRouteInfo) {
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
static void HTTP_Send404Page(SOCKET client_socket, char* route) {
	char* http_error_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><h1>404 couldn't find requested resource.</h1></html>";
	send(client_socket, http_error_header, (int)strlen(http_error_header), 0);
	printf("[SERVER] Sent 404 message for route: %s\n", route);
}

static void HTTP_ParsePOSTRequest(char* puts_request, char* decoded_route) {
	int index = 0;
	while (true) {
		char* string_from_current_pos = puts_request+index;
		if (index+4 < strlen(puts_request)-1) {
			char original_char = puts_request[index+4];
			puts_request[index+4] = 0;
			if (!strcmp(string_from_current_pos, "\r\n\r\n")) {
				puts_request[index+4] = original_char;
				index += 4;
				break;
			}

			puts_request[index+4] = original_char;
		}
		index++;
	}

	printf("[POST Request] %s\n", puts_request+index);
	StringArray request_key_value_pairs_array = ParseURIKeyValuePairString(puts_request+index);
	char** key_value_pairs_array = request_key_value_pairs_array.array;
	int max_key_value_pairs_array_matches = request_key_value_pairs_array.count;

	if (global_variable_key_value_pairs_array[global_key_value_pairs_index] != NULL) {
		global_key_value_pairs_index++;
		if (global_key_value_pairs_index > INITIAL_GLOBAL_KEY_VALUE_PAIRS_ARRAY_SIZE-1) {
			printf("Realloc global_variable_key_value_pairs_array.\n");
			global_key_value_pairs_added_elements++;
			global_variable_key_value_pairs_array = realloc(global_variable_key_value_pairs_array, sizeof(char*)*(INITIAL_GLOBAL_KEY_VALUE_PAIRS_ARRAY_SIZE + global_key_value_pairs_added_elements));
		}
	}

	bool found_existing_match;
	for (int i = 0; i < max_key_value_pairs_array_matches; i += 2) {
		found_existing_match = false;
		for (int x = 0; x < global_key_value_pairs_index; x += 2) {
			if (!strcmp(key_value_pairs_array[i], global_variable_key_value_pairs_array[x])) {
				found_existing_match = true;
				ResizeStringInStringArray(global_variable_key_value_pairs_array, x+1, key_value_pairs_array[i+1]);
			}
		}

		if (!found_existing_match) {
			PushNewStringToStringArray(global_variable_key_value_pairs_array, global_key_value_pairs_index, key_value_pairs_array[i]);

			global_key_value_pairs_index++;
			if (global_key_value_pairs_index > INITIAL_GLOBAL_KEY_VALUE_PAIRS_ARRAY_SIZE-1) {
				printf("Realloc global_variable_key_value_pairs_array.\n");
				global_key_value_pairs_added_elements++;
				global_variable_key_value_pairs_array = realloc(global_variable_key_value_pairs_array, sizeof(char*)*(INITIAL_GLOBAL_KEY_VALUE_PAIRS_ARRAY_SIZE + global_key_value_pairs_added_elements));
			}

			PushNewStringToStringArray(global_variable_key_value_pairs_array, global_key_value_pairs_index, key_value_pairs_array[i+1]);
			global_key_value_pairs_index++;
		}
	}

	FreeStringArray(key_value_pairs_array, max_key_value_pairs_array_matches);
}

static HTTPGETRequest HTTP_ParseGETRequest(char* request_path, StringArray parsed_header) {
	bool require_json_response = false;
	for (int index = 0; index < parsed_header.count; index += 2) {
		if (!strcmp(parsed_header.array[index], "Content-Type") && 
			!strcmp(parsed_header.array[index+1], "application/json")) {
				printf("------------------------------\n");
				printf("JUST GOT A JSON FORMAT REQUEST!!!\n");
				printf("------------------------------\n");
				require_json_response = true;
				break;
		}
	}

	int i = 0;
	bool contains_custom_vars = false;
	for (; i < strlen(request_path); i++) {
		if (request_path[i] == '?') {
			contains_custom_vars = true;
			break;
		}
	}

	if (contains_custom_vars) {
		request_path[i] = 0;
		i += 1;
	}
	
	// Responding to the request with the data in route_info_array if
	// an entry for the route exists.
	char* data_to_send = {0};
	int data_to_send_length;
	char* http_response_header = {0};
	char* path_to_file_at_route = {0};

	for (int index = 0; index < route_info_array_index+1; index++) {
		if (!strcmp(route_info_array[index].route, request_path)) {
			path_to_file_at_route = route_info_array[index].path_to_file;
		}
	}

	// Checking if the route can be mapped to a path to an existing file.
	char file_type[10] = {0};
	char temp_buffer[256] = {0};
	if (HTTP_ExtractFileTypeFromFilePath(request_path, file_type)) {
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

	if (contains_custom_vars) {
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
		StringArray request_key_value_pairs_array = ParseURIKeyValuePairString(request_path+i);
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

	bool found_matches = false;
	for (int i = 0; i < max_original_file_vars_matches; i++) {
		for (int x = 0; x < global_key_value_pairs_index; x += 2) {
			// Adding 2 to the trimmed file variable to remove `{{` at the start.
			if (!strcmp(trim_file_vars[i]+2, global_variable_key_value_pairs_array[x])) {
				found_matches = true;
				StrReplaceSubstringAllOccurance(&data_to_send, original_file_vars[i], global_variable_key_value_pairs_array[x+1]);
			}
		}
	}

	FreeStringArray(original_file_vars, max_original_file_vars_matches);
	FreeStringArray(trim_file_vars, max_original_file_vars_matches);

	if (found_matches) {
		data_to_send_length = strlen(data_to_send);
	}

	return (HTTPGETRequest) {
		.http_response_header = http_response_header,
		.data_to_send = data_to_send,
		.data_to_send_length = data_to_send_length
	};

}

static int HTTP_RunServer(char* server_ip_address, char* server_port) {
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

		char receiving_buffer[2056] = {0};
		int receiving_buffer_len = 2056;
		int init_send_result = 1;

		init_result = recv(client_socket, receiving_buffer, receiving_buffer_len, 0);
		if (init_result > 0) {
			printf("[SERVER] Bytes received: %d\n", init_result);
			printf("[SERVER] Data Received: %s\n", receiving_buffer);

			// Parsing HTTP Requset
			// Getting HTTP request method.
			char parsed_request_method[MAX_HTTP_REQUEST_METHOD_SIZE] = {0};

			int index = 0;
			for (;receiving_buffer[index] != ' '; index++) {
				parsed_request_method[index] = receiving_buffer[index];
			}

			// Getting the route.
			char parsed_request_route[MAX_HTTP_ROUTE_SIZE] = {0};

			index++;
			for (int route_index = 0; receiving_buffer[index] != ' '; index++, route_index++) {
				parsed_request_route[route_index] = receiving_buffer[index];
			}

			char* decoded_route = HTTP_DecodeURL(parsed_request_route);

			// Generating array of header keys and values
			while (receiving_buffer[index] != '\r' && receiving_buffer[index+1] != '\n') {
				index++;
			}
			index += 2;

			StringArray parsed_header = ParseHeaderIntoKeyValuePairString(receiving_buffer+index);

			if (!strcmp(parsed_request_method, "POST")) {
				HTTP_ParsePOSTRequest(receiving_buffer, decoded_route);
			}

			HTTPGETRequest output = HTTP_ParseGETRequest(decoded_route, parsed_header);
			if (!output.data_to_send_length) {
				HTTP_Send404Page(client_socket, decoded_route);
				closesocket(client_socket);
				free(decoded_route);
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
			free(decoded_route);

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
