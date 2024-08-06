#pragma once
#define MAX_HTTP_REQUEST_METHOD_SIZE 20
#define MAX_HTTP_ROUTE_SIZE 512
#define MAX_HTTP_QUERY_STRING_SIZE MAX_HTTP_ROUTE_SIZE
#define MAX_HTTP_REQUEST_SIZE 2056
#define INITIAL_ROUTE_INFO_ARRAY_SIZE 100
#define INITIAL_GLOBAL_ROUTE_AND_JSON_ARRAY_SIZE 200

typedef struct {
	char* route;
	char* path_to_file;
} HTTPRouteAndFilePath;

typedef struct {
	char* http_response_header;
	char* data_to_send;
	int data_to_send_length;
} HTTPGETRequest;

typedef struct {
	char* route;
	cJSON* data;
} HTTPRouteJSON;

typedef struct {
    char original_route[MAX_HTTP_ROUTE_SIZE];
    // Decoded URI encoding.
    //@Heap Alloc
    char* decoded_route;
    // Stripped away the query string.
    char stripped_route[MAX_HTTP_ROUTE_SIZE];
    char request_method[MAX_HTTP_REQUEST_METHOD_SIZE];
    char query_string[MAX_HTTP_QUERY_STRING_SIZE];
    //@Heap Alloc
    char* request_body;
    char content_type[128];
    bool contains_query_string;
} HTTPRequestInfo;

global HTTPRouteAndFilePath* route_info_array;
global int route_info_array_index;
// This is the number of elements which have been added above
// INITIAL_ROUTE_INFO_ARRAY_SIZE
global int num_of_added_elements;

global char** search_dirs;
global int search_dirs_size;

global HTTPRouteJSON* global_route_and_json_array;
global int global_route_and_json_index;
global int global_route_and_json_added_elements;

global char* put_request_default_dir;


void HTTP_Initialize(void);
void HTTP_SetDefaultPUTDirectory(char* default_dir);
bool HTTP_HandleRoute(char* method, char* route, char* path_to_data);
void HTTP_SetSearchDirectories(char* dirs[], size_t dirs_size);
bool HTTP_HandleRedirectRoute(char* method, char* origin_route, char* redirect_route);
void HTTP_Send404Page(SOCKET client_socket, char* route);
int HTTP_RunServer(char* server_ip_address, char* server_port);
