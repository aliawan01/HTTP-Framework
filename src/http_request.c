#include "server.h"

global HTTPRouteInfo* route_info_array;
global int route_info_array_index;
// This is the number of elements which have been added above
// INITIAL_ROUTE_INFO_ARRAY_SIZE
global int num_of_added_elements;

static void HTTP_Initialize(void) {
	route_info_array = malloc(sizeof(HTTPRouteInfo)*INITIAL_ROUTE_INFO_ARRAY_SIZE);
	memset(route_info_array, 0, sizeof(HTTPRouteInfo)*INITIAL_ROUTE_INFO_ARRAY_SIZE);
	route_info_array_index = -1;
	num_of_added_elements = 0;
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

static char* HTTP_GenHeaderFromFile(char* path_to_file) {
	// Extracting file type into a separate string.
	int index = 0;
	for (;path_to_file[index] != '.'; index++);
	char file_type[24] = {0};
	
	index++;
	for (int file_type_index = 0; index != strlen(path_to_file); index++, file_type_index++) {
		file_type[file_type_index] = path_to_file[index];
	}

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

		char receiving_buffer[2056];
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

			printf("Parsed Method: %s\n", parsed_request_method);
			printf("Requested Route: %s\n", parsed_request_route);

			// Responding to the request with the data in route_info_array if
			// an entry for the route exists.
			char* data_to_send = {0};
			int data_to_send_length;
			char* http_response_header = {0};
			char* path_to_file_at_route = {0};

			for (int index = 0; index < route_info_array_index+1; index++) {
				if (!strcmp(route_info_array[index].route, parsed_request_route)) {
					path_to_file_at_route = route_info_array[index].path_to_file;
				}
			}

			// Route doesn't exist.
			if (path_to_file_at_route == NULL) {
				HTTP_Send404Page(client_socket, parsed_request_route);
				closesocket(client_socket);
				continue;
			}

			data_to_send = HTTP_GetFileContents(path_to_file_at_route);
			data_to_send_length = HTTP_FindFileSize(path_to_file_at_route);

			// File was deleted or it's name was changed during runtime.
			if (data_to_send_length == -1) {
				HTTP_Send404Page(client_socket, parsed_request_route);
				closesocket(client_socket);
				continue;
			}

			// Not a recognised file type.
			http_response_header = HTTP_GenHeaderFromFile(path_to_file_at_route);
			if (http_response_header == NULL) {
				HTTP_Send404Page(client_socket, parsed_request_route);
				closesocket(client_socket);
				printf("[ERROR] Couldn't generate header for %s\n", path_to_file_at_route);
				continue;
			}

			printf("Html Code: %s\n", data_to_send);

			send(client_socket, http_response_header, (int)strlen(http_response_header), 0);
			send(client_socket, data_to_send, data_to_send_length, 0);

			printf("[SERVER] Bytes sent: %d\n", init_send_result);
			printf("[SERVER] Data sent (HEADER): %s\n", http_response_header);
			printf("[SERVER] Data sent (DATA): %s\n", data_to_send);

			free(data_to_send);
			free(http_response_header);
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