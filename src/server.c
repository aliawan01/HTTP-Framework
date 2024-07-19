#include "server.h"

#include "http_request.c"

int main(void) {
	HTTP_Initialize();

	HTTP_HandleRoute("GET", "/", "static/first_page.html");
	HTTP_HandleRoute("GET", "/favicon.ico", "static/favicon.ico");
	HTTP_HandleRoute("GET", "/ginger.jpg", "static/ginger.jpg");
	HTTP_HandleRedirectRoute("GET", "/otherGinger", "/ginger.jpg");

	printf("\n");
	HTTP_RunServer("127.0.0.1", "8000");

	return 0;
}
