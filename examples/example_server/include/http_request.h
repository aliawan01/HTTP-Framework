#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

#include "http_string_handling.h"

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
    CONTINUE_100                        = 0,
    SWITCHING_PROTOCOLS_101             = 1,
    PROCESSING_102                      = 2,
    EARLY_HINTS_103                     = 3,
    OK_200                              = 4,
    CREATED_201                         = 5,
    ACCEPTED_202                        = 6,
    NON_AUTHORITATIVE_INFORMATION_203   = 7,
    NO_CONTENT_204                      = 8,
    RESET_CONTENT_205                   = 9,
    PARTIAL_CONTENT_206                 = 10,
    MULTI_STATUS_207                    = 11,
    ALREADY_REPORTED_208                = 12,
    IM_USED_226                         = 13,
    MULTIPLE_CHOICES_300                = 14,
    MOVED_PERMANENTLY_301               = 15,
    FOUND_302                           = 16,
    SEE_OTHER_303                       = 17,
    NOT_MODIFIED_304                    = 18,
    USE_PROXY_305                       = 19,
    SWITCH_PROXY_306                    = 20,
    TEMPORARY_REDIRECT_307              = 21,
    PERMANENT_REDIRECT_308              = 22,
    BAD_REQUEST_400                     = 23,
    UNAUTHORIZED_401                    = 24,
    PAYMENT_REQUIRED_402                = 25,
    FORBIDDEN_403                       = 26,
    NOT_FOUND_404                       = 27,
    METHOD_NOT_ALLOWED_405              = 28,
    NOT_ACCEPTABLE_406                  = 29,
    PROXY_AUTHENTICATION_REQUIRED_407   = 30,
    REQUEST_TIMEOUT_408                 = 31,
    CONFLICT_409                        = 32,
    GONE_410                            = 33,
    LENGTH_REQUIRED_411                 = 34,
    PRECONDITION_FAILED_412             = 35,
    PAYLOAD_TOO_LARGE_413               = 36,
    URI_TOO_LONG_414                    = 37,
    UNSUPPORTED_MEDIA_TYPE_415          = 38,
    RANGE_NOT_SATISFIABLE_416           = 39,
    EXPECTATION_FAILED_417              = 40,
    IM_A_TEAPOT_418                     = 41,
    MISDIRECTED_REQUEST_421             = 42,
    UNPROCESSABLE_ENTITY_422            = 43,
    LOCKED_423                          = 44,
    FAILED_DEPENDENCY_424               = 45,
    TOO_EARLY_425                       = 46,
    UPGRADE_REQUIRED_426                = 47,
    PRECONDITION_REQUIRED_428           = 48,
    TOO_MANY_REQUESTS_429               = 49,
    REQUEST_HEADER_FIELDS_TOO_LARGE_431 = 50,
    UNAVAILABLE_FOR_LEGAL_REASONS_451   = 51,
    INTERNAL_SERVER_ERROR_500           = 52,
    NOT_IMPLEMENTED_501                 = 53,
    BAD_GATEWAY_502                     = 54,
    SERVICE_UNAVAILABLE_503             = 55,
    GATEWAY_TIMEOUT_504                 = 56,
    HTTP_VERSION_NOT_SUPPORTED_505      = 57,
    VARIANT_ALSO_NEGOTIATES_506         = 58,
    INSUFFICIENT_STORAGE_507            = 59,
    LOOP_DETECTED_508                   = 60,
    NOT_EXTENDED_510                    = 61,
    NETWORK_AUTHENTICATION_REQUIRED_51  = 62
};

global_variable char* HTTP_StatusCodeStrings[] = {
    "100 Continue",
    "101 Switching protocols",
    "102 Processing",
    "103 Early Hints",
    "200 OK",
    "201 Created",
    "202 Accepted",
    "203 Non-Authoritative Information",
    "204 No Content",
    "205 Reset Content",
    "206 Partial Content",
    "207 Multi-Status",
    "208 Already Reported",
    "226 IM Used",
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Found",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "306 Switch Proxy",
    "307 Temporary Redirect",
    "308 Permanent Redirect",
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Method Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Timeout",
    "409 Conflict",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Payload Too Large",
    "414 URI Too Long",
    "415 Unsupported Media Type",
    "416 Range Not Satisfiable",
    "417 Expectation Failed",
    "418 I'm a Teapot",
    "421 Misdirected Request",
    "422 Unprocessable Entity",
    "423 Locked",
    "424 Failed Dependency",
    "425 Too Early",
    "426 Upgrade Required",
    "428 Precondition Required",
    "429 Too Many Requests",
    "431 Request Header Fields Too Large",
    "451 Unavailable For Legal Reasons",
    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Unavailable",
    "504 Gateway Timeout",
    "505 HTTP Version Not Supported",
    "506 Variant Also Negotiates",
    "507 Insufficient Storage",
    "508 Loop Detected",
    "510 Not Extended",
    "511 Network Authentication Required"
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

global_variable char* path_to_404_page;

global_variable char** search_dirs;
global_variable int search_dirs_size;

HTTPEXPORTFUNC void   HTTP_Initialize(void);
HTTPEXPORTFUNC bool   HTTP_HandleRoute(StringArray permissions, char* method, char* route, bool is_regex_route, void (*response_func)(Arena* arena, HTTPRequestInfo*, HTTPResponse*));
HTTPEXPORTFUNC bool   HTTP_DeleteRouteForMethod(char* method, char* route, bool is_regex_route);
HTTPEXPORTFUNC bool   HTTP_DeleteRouteForAllMethod(char* route, bool is_regex_route);
HTTPEXPORTFUNC void   HTTP_SetSearchDirectories(char* dirs[], size_t dirs_size);
HTTPEXPORTFUNC bool   HTTP_Set404Page(char* path_to_error_page);
HTTPEXPORTFUNC int    HTTP_RunServer(char* server_port, char* path_to_certificate, char* path_to_private_key);
HTTPEXPORTFUNC void   HTTP_AddHeaderToHeaderDict(Arena* arena, HeaderDict* header_dict, char* key, char* value);
HTTPEXPORTFUNC void   HTTP_AddCookieToCookieJar(Arena* arena, CookieJar* cookie_jar, char* key, char* value, int64_t max_age, char* expires, char* path, char* domain, bool secure, bool http_only);
HTTPEXPORTFUNC void   HTTP_TemplateText(Arena* arena, HTTPRequestInfo* request_info, cJSON* variables, String* source);
HTTPEXPORTFUNC String HTTP_TemplateTextFromFile(Arena* arena, HTTPRequestInfo* request_info, cJSON* variables, char* file_path);
HTTPEXPORTFUNC char*  HTTP_CreateDateString(Arena* arena, String day_name, int day_num, String month, int year, int hour, int minute, int second);
// TODO: Maybe we can move this to a better place.
void   CreateHTTPResponseFunc(ThreadContext ctx, SSL* ssl);

#ifdef __cplusplus
}
#endif
