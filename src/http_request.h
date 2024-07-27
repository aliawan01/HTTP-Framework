#pragma once
#define MAX_HTTP_REQUEST_METHOD_SIZE 30
#define MAX_HTTP_ROUTE_SIZE 512
#define INITIAL_ROUTE_INFO_ARRAY_SIZE 100
#define INITIAL_GLOBAL_KEY_VALUE_PAIRS_ARRAY_SIZE 200

typedef struct {
	char* route;
	char* path_to_file;
} HTTPRouteInfo;

typedef struct {
	char* http_response_header;
	char* data_to_send;
	int data_to_send_length;
} HTTPGETRequest;

global HTTPRouteInfo* route_info_array;
global int route_info_array_index;
// This is the number of elements which have been added above
// INITIAL_ROUTE_INFO_ARRAY_SIZE
global int num_of_added_elements;

global char** search_dirs;
global int search_dirs_size;

// TODO: Should be moved to a database really.
global char** global_variable_key_value_pairs_array;
global int global_key_value_pairs_index;
global int global_key_value_pairs_added_elements;
