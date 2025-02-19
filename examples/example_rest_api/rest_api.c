#include "http_util.h"
#include "http_request.h"
#include "http_file_handling.h"
#include "http_string_handling.h"
#include "http_database.h"
#include "http_authentication.h"

void root_page_handler(Arena* arena, HTTPRequestInfo* request_info, HTTPResponse* response) {
    HTTP_SetContentTypeHeader("application/json");
    if (HTTP_RequestMethodCompareAndIsJSON("GET")) {
        cJSON* allUsers = HTTP_RunSQLQuery("SELECT * FROM Users", true, true);
        char* responseBodyString = cJSON_PrintUnformatted(allUsers);

        response->response_body = (String){responseBodyString, strlen(responseBodyString)};
    }
    else if (HTTP_RequestMethodCompareAndIsJSON("POST")) {
        cJSON* converted_array = HTTP_cJSON_TurnObjectIntoArray("Users", request_info->json_request_body);
        HTTP_InsertJSONIntoDatabase(converted_array);
        cJSON* responseObj = cJSON_CreateObject();
        cJSON_AddStringToObject(responseObj, "result", "Successfully added user to database.");
        char* responseObjString = cJSON_PrintUnformatted(responseObj);
        response->response_body = (String){responseObjString, strlen(responseObjString)};
    }
    else if (HTTP_RequestMethodCompareAndIsJSON("DELETE")) {
        char* deleteQueryString = PushString(arena, 500);
        strcpy(deleteQueryString, "DELETE FROM Users WHERE");

        int numOfDefinedVars = 0;
        cJSON* elem = NULL;
        cJSON_ArrayForEach(elem, request_info->json_request_body) {
            if (!numOfDefinedVars) {
                strcat(deleteQueryString, " ");
            }
            else {
                strcat(deleteQueryString, " AND ");
            }
            strcat(deleteQueryString, elem->string);
            strcat(deleteQueryString, "='");
            strcat(deleteQueryString, HTTP_cJSON_GetStringValue(arena, elem));
            strcat(deleteQueryString, "'");
            numOfDefinedVars += 1;
        }

        strcat(deleteQueryString, ";");

        cJSON* result = HTTP_RunSQLQuery(deleteQueryString, false, false);

        cJSON* responseObj = cJSON_CreateObject();
        if (result != NULL) {
            cJSON_AddStringToObject(responseObj, "result", "Deleted user from the database.");
        }
        else {
            cJSON_AddStringToObject(responseObj, "result", "Did not provide enough variables to delete user or provided invalid variables.");
        }

        char* responseString = cJSON_PrintUnformatted(responseObj);
        response->response_body = (String) {responseString, strlen(responseString)};
    }
    else {
        HTTP_SetContentTypeHeader("text/html");
        response->response_body = HTTP_GetFileContents(arena, "static/rest_api_example.html");
    }
}

int main(void) {
    if (sqlite3_threadsafe() == 0) {
        fprintf(stderr, "[ERROR] The version of sqlite3 you are is not thread safe, please try to recompile it with the following flag: `SQLITE_THREADSAFE=1`.\n");
        return -1;
    }

    HTTP_Initialize();

    HTTP_CreateDatabase("user_info.db");
    HTTP_RunSQLQuery("CREATE TABLE IF NOT EXISTS Users(fname TEXT, lname TEXT, age INTEGER , hobby TEXT)", false, false);
    /* HTTP_RunSQLQuery("INSERT INTO Users VALUES ('John', 'Doe', 83, 'Gym')", false, false); */

    char* dirs[] = { "static/", };
    HTTP_SetSearchDirectories(dirs, ArrayCount(dirs));

    HTTP_HandleRoute(StrArrayLit("global"), "GET",    "/", false, root_page_handler);
    HTTP_HandleRoute(StrArrayLit("global"), "POST",   "/", false, root_page_handler);
    HTTP_HandleRoute(StrArrayLit("global"), "PUT",    "/", false, root_page_handler);
    HTTP_HandleRoute(StrArrayLit("global"), "DELETE", "/", false, root_page_handler);

    HTTP_Set404Page("static/error_page.html");

    printf("\n");

    HTTP_RunServer("8000", "vendor/certs/cert.pem", "vendor/certs/key.pem");

    return 0;
}
