#include "util.h"
#include "http_request.h"
#include "file_handling.h"
#include "string_handling.h"
#include "cjson_helper.h"
#include "database.h"

void second_page_handler(Arena* arena, HTTPRequestInfo* request_info, HTTPResponse* response) {
    response->status_code = OK_200;

    HTTP_AddHeaderToHeaderDict(arena, &response->headers, "Content-Type", "text/html");
    response->response_body = "<h1>Recieved it</h1><p>Trying to fillout some space</p>";
    response->response_body_length = strlen("<h1>Recieved it</h1><p>Trying to fillout some space</p>");
}

void root_page_handler(Arena* arena, HTTPRequestInfo* request_info, HTTPResponse* response) {
    response->status_code = OK_200;

    if (!strcmp(request_info->request_method, "POST") && request_info->is_json_request) {
        HTTP_RunSQLQuery("DELETE FROM Info", false);
        cJSON* converted_array = HTTP_cJSON_TurnObjectIntoArray("Info", request_info->json_request_body);
        HTTP_InsertJSONIntoDatabase(converted_array);
    }

    cJSON* result = HTTP_RunSQLQuery("SELECT DISTINCT fname, lname, pname FROM Info", false);
    HTTP_AddHeaderToHeaderDict(arena, &response->headers, "Content-Type", "text/html");
    response->response_body = HTTP_TemplateTextFromFile(arena, request_info, result, "static/first_page.html");
    response->response_body_length = strlen(response->response_body);
    HTTP_HandleRoute("GET", "/ooga", false, second_page_handler);
}


int main(void) {
	HTTP_Initialize();

    HTTP_CreateDatabase("new.db");
    HTTP_RunSQLQuery("CREATE TABLE IF NOT EXISTS Info(fname TEXT, lname TEXT, pname TEXT, custom TEXT, name TEXT, surname TEXT)", false);

	char* dirs[] = {"something_elsdafj", "static/", "../Web Server/static", "good", "C:/something", "C:/good_morning/ls", "C:\\Dev\\Real Work\\Web Server\\static\\"};
	HTTP_SetSearchDirectories(dirs, ArrayCount(dirs));

	/* HTTP_HandleRoute("GET", "/", "static/first_page.html"); */
	HTTP_HandleRoute("GET", "/", false, root_page_handler);
	HTTP_HandleRoute("POST", "/", false, root_page_handler);

	HTTP_HandleRoute("GET", "/good", false, root_page_handler);
	HTTP_HandleRoute("POST", "/good", true, root_page_handler);
	HTTP_HandleRoute("UPDATE", "/good", true, root_page_handler);

	HTTP_HandleRoute("GET", "/something[0-9]+", false, second_page_handler);
	HTTP_HandleRoute("GET", "/something[0-9]+", true, root_page_handler);


    HTTP_DeleteRouteForAllMethod("/good", true);

	/* HTTP_HandleRedirectRoute("GET", "/other main website", "/"); */

	printf("\n");
	HTTP_RunServer("127.0.0.1", "8000");
    HTTP_CloseDatabase();

	return 0;
}
