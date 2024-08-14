#include "util.h"
#include "http_request.h"
#include "file_handling.h"
#include "string_handling.h"

void root_page_handler(Allocator* allocator, HTTPRequestInfo* request_info, HTTPResponse* response) {
    response->status_code = OK_200;
    response->headers = HTTP_StringDup(allocator->global_arena, "Content-Type: text/html");
    response->response_body = HTTP_GetFileContents(allocator->global_arena, "static/first_page.html");
    response->response_body_length = HTTP_FindFileSize("static/first_page.html");
    response->enable_templating = false;
}

int main(void) {
	HTTP_Initialize();

	char* dirs[] = {"static/", "../Web Server/static", "good", "C:/something", "C:/good_morning/ls", "C:\\Dev\\Real Work\\Web Server\\static\\"};
	HTTP_SetSearchDirectories(dirs, ArrayCount(dirs));
    HTTP_SetDefaultPUTDirectory("static");

	/* HTTP_HandleRoute("GET", "/", "static/first_page.html"); */
	HTTP_HandleRoute("GET", "/", root_page_handler);
	/* HTTP_HandleRedirectRoute("GET", "/other main website", "/"); */

	printf("\n");
	HTTP_RunServer("127.0.0.1", "8000");

	return 0;
}
