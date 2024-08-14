#pragma once
#define MAX_HTTP_REQUEST_METHOD_SIZE 20
#define MAX_HTTP_ROUTE_SIZE 512
#define MAX_HTTP_QUERY_STRING_SIZE MAX_HTTP_ROUTE_SIZE
#define MAX_HTTP_REQUEST_SIZE 2056
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
    char original_route[MAX_HTTP_ROUTE_SIZE];
    // Will point to the string at either stripped_route or 
    // decoded_route depending upon if the original route contains 
    // a query string.
    char* route_to_use;
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

enum HTTPStatusCode {
    OK_200,
    NOT_FOUND_404,
    CREATED_201,
    URI_TOO_LONG_414,
    CONTENT_TOO_LARGE_413
};

global_variable char* HTTP_StatusCodeStrings[] = {
    "200 OK",
    "404 Not Found",
    "201 Created",
    "414 URI Too Long",
    "413 Content Too Large"
};


typedef struct {
    enum HTTPStatusCode status_code;
    //@Heap Alloc
    char* headers;
    //@Heap Alloc
    char* response_body;
    int response_body_length;
    bool enable_templating;
} HTTPResponse;


typedef struct {
    Arena* global_arena;
    Arena* scratch_arena;
} Allocator;

global_variable Allocator* allocator;

typedef struct {
    char* method;
	char* route;
    void (*response_func)(Allocator*, HTTPRequestInfo*, HTTPResponse*);
	cJSON* data;
} HTTPRouteJSON;

global_variable HTTPRouteJSON* global_route_and_json_array;
global_variable int global_route_and_json_index;
//global_variable int global_route_and_json_added_elements;

global_variable char** search_dirs;
global_variable int search_dirs_size;

global_variable char* put_request_default_dir;

void HTTP_Initialize(void);
void HTTP_SetDefaultPUTDirectory(char* default_dir);
bool HTTP_HandleRoute(char* method, char* route, void (*response_func)(Allocator*, HTTPRequestInfo*, HTTPResponse*));
void HTTP_SetSearchDirectories(char* dirs[], size_t dirs_size);
bool HTTP_HandleRedirectRoute(char* method, char* origin_route, char* redirect_route);
void HTTP_Send404Page(SOCKET client_socket, char* route);
int HTTP_RunServer(char* server_ip_address, char* server_port);
