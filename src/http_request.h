#pragma once
#define MAX_HTTP_REQUEST_METHOD_SIZE 30
#define MAX_HTTP_ROUTE_SIZE 512
#define INITIAL_ROUTE_INFO_ARRAY_SIZE 100

typedef struct {
	char* route;
	char* path_to_file;
} HTTPRouteInfo;

global HTTPRouteInfo* route_info_array;
global int route_info_array_index;
// This is the number of elements which have been added above
// INITIAL_ROUTE_INFO_ARRAY_SIZE
global int num_of_added_elements;

global char** search_dirs;
global int search_dirs_size;
