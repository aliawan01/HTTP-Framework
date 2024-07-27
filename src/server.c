#include "server.h"

#include "string_handling.c"
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

	// char* puts_file = HTTP_GetFileContents("static/puts_output.txt");
	// int index = 0;
	// while (puts_file[index] != '\r' && puts_file[index+1] != '\n') {
	// 	index++;
	// }
	// index += 2;

	// printf("%s\n\n", puts_file+index);
	// StringArray parsed_header = ParseHeaderIntoKeyValuePairString(puts_file+index);
	// for (int i = 0; i < parsed_header.count; i += 2) {
	// 	printf("Header: %s, Value: %s\n", parsed_header.array[i], parsed_header.array[i+1]);
	// }

	// cJSON* json_output = cJSON_Parse("{\"something\": 10, \"custom\": [40, 50, 60, {\"older\": \"than\"}]}");
	// printf("%s\n", cJSON_Print(json_output));




	return 0;
}
