#include "server.h"

#include "file_handling.c"
#include "escape_characters.c"
#include "http_request.c"


int main(void) {
	HTTP_Initialize();

	char* dirs[] = {"static/", "../Web Server/static", "good", "C:/something", "C:/good_morning/ls", "C:\\Dev\\Real Work\\Web Server\\static\\"};
	HTTP_SetSearchDirectories(dirs, ArrayCount(dirs));
	HTTP_HandleRoute("GET", "/", "static/first_page.html");
	HTTP_HandleRedirectRoute("GET", "/other main website", "/");

	printf("\n");
	HTTP_RunServer("127.0.0.1", "8000");

	
	return 0;
}
