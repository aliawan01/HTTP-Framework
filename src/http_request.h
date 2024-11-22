#pragma once

#include "string_handling.h"

#define MAX_HTTP_REQUEST_METHOD_SIZE 20
#define MAX_HTTP_ROUTE_SIZE 512
#define MAX_HTTP_QUERY_STRING_SIZE MAX_HTTP_ROUTE_SIZE
#define MAX_HTTP_REQUEST_SIZE 5024
#define INITIAL_GLOBAL_ROUTE_CALLBACK_ARRAY_SIZE 1000

#define HTTP_SetContentTypeHeader(type) HTTP_AddHeaderToHeaderDict(arena, &response->headers, "Content-Type", type)

typedef Dict HeaderDict;
typedef Dict CookiesDict;

// TODO: May not be using this and can just get rid of it.
typedef struct HTTPGetRequest {
	char* http_response_header;
	char* data_to_send;
	int   data_to_send_length;
} HTTPGETRequest;

typedef struct {
    char original_route[MAX_HTTP_ROUTE_SIZE];
    // Will point to the string at either stripped_route or 
    // decoded_route depending upon if the original route contains 
    // a query string.
    char* route_to_use;
    // Decoded URI encoding.
    char* decoded_route;
    // Stripped away the query string.
    char stripped_route[MAX_HTTP_ROUTE_SIZE];
    char request_method[MAX_HTTP_REQUEST_METHOD_SIZE];
    char query_string[MAX_HTTP_QUERY_STRING_SIZE];
    union {
        char* request_body;
        cJSON* json_request_body;
    };

    HeaderDict headers;
    CookiesDict cookies;
    bool is_json_request;
    bool contains_query_string;
    char* session_id;
    char* user_permission;
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
    char*    key;
    char*    value;
    int64_t  max_age;
    char*    expires;
    char*    path;
    char*    domain;
    bool     secure;
    bool     http_only;
} Cookie;

typedef struct {
    Cookie* cookies;
    int count;
} CookieJar;

typedef struct {
    enum HTTPStatusCode status_code;
    HeaderDict headers;
    CookieJar cookie_jar;
    String response_body;
} HTTPResponse;

typedef struct {
    char* method;
	char* route;
    StringArray permissions;
    bool is_regex_route;
    void (*response_func)(Arena*, HTTPRequestInfo*, HTTPResponse*);
} HTTPRouteCallback;

global_variable HTTPRouteCallback* global_route_callback_array;
global_variable int global_route_callback_index;

global_variable char** search_dirs;
global_variable int search_dirs_size;

void   HTTP_Initialize(void);
bool   HTTP_HandleRoute(StringArray permissions, char* method, char* route, bool is_regex_route, void (*response_func)(Arena* arena, HTTPRequestInfo*, HTTPResponse*));
bool   HTTP_DeleteRouteForMethod(char* method, char* route, bool is_regex_route);
bool   HTTP_DeleteRouteForAllMethod(char* route, bool is_regex_route);
void   HTTP_SetSearchDirectories(char* dirs[], size_t dirs_size);
int    HTTP_Send404Page(SSL* ssl, char* route);
int    HTTP_RunServer(char* server_port, char* path_to_certificate, char* path_to_private_key);
void   HTTP_AddHeaderToHeaderDict(Arena* arena, HeaderDict* header_dict, char* key, char* value);
void   HTTP_AddCookieToCookieJar(Arena* arena, CookieJar* cookie_jar, char* key, char* value, int64_t max_age, char* expires, char* path, char* domain, bool secure, bool http_only);
void   HTTP_TemplateText(Arena* arena, HTTPRequestInfo* request_info, cJSON* variables, String* source);
String HTTP_TemplateTextFromFile(Arena* arena, HTTPRequestInfo* request_info, cJSON* variables, char* file_path);
char*  HTTP_CreateDateString(Arena* arena, String day_name, int day_num, String month, int year, int hour, int minute, int second);
// TODO: Maybe we can move this to a better place.
void   CreateHTTPResponseFunc(ThreadContext ctx, SSL* ssl);
