#pragma once
#define MAX_HTTP_REQUEST_METHOD_SIZE 30
#define MAX_HTTP_ROUTE_SIZE 512
#define INITIAL_ROUTE_INFO_ARRAY_SIZE 100

typedef struct {
	char method[MAX_HTTP_REQUEST_METHOD_SIZE];
	char route[MAX_HTTP_ROUTE_SIZE];
	// TODO: Will need to add headers here for `Connection-Type: keep-alive`
} HTTPRequest;

typedef struct {
	char* route;
	char* path_to_file;
} HTTPRouteInfo;
