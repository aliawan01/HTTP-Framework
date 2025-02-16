#include "http_util.h"
#include "http_request.h"
#include "http_file_handling.h"
#include "http_string_handling.h"
#include "http_database.h"
#include "http_authentication.h"
#include "thread_pool.h"

void second_page_handler(Arena* arena, HTTPRequestInfo* request_info, HTTPResponse* response) {
    printf("second_page_handler: `%s`\n", request_info->user_permission); 

    HTTP_SetContentTypeHeader("text/html");
    cJSON* elem = NULL;
    cJSON_ArrayForEach(elem, HTTP_Auth_GetAllSessionID(true, false)) {
        printf("[second_page_handler] deleted user with session ID: `%s`\n", elem->valuestring);
        HTTP_Auth_DeleteUserAtSessionToken(elem->valuestring);
    }

    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "oloooo", "ofofjlaa", 20, NULL, "/", NULL, false, true);
    response->response_body = HTTP_StrLit("<h1>Deleted Users</h1><p>Trying to fillout some space</p>");
}

void root_page_handler(Arena* arena, HTTPRequestInfo* request_info, HTTPResponse* response) {
    if (!strcmp(request_info->request_method, "POST") && request_info->is_json_request) {
        HTTP_RunSQLQuery("DELETE FROM Info", false, true);
        cJSON* converted_array = HTTP_cJSON_TurnObjectIntoArray("Info", request_info->json_request_body);
        HTTP_InsertJSONIntoDatabase(converted_array);
    }

    HTTP_Set404Page("static/ginger.jpg");

    cJSON* result = HTTP_RunSQLQuery("SELECT DISTINCT fname, lname, pname FROM Info", false, true);
    HTTP_SetContentTypeHeader("text/html");

    /* HTTP_AddHeaderToHeaderDict(arena, &response->headers, "Set-Cookie", "name=John"); */
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "good", "hello", -1, NULL, NULL, NULL, false, false);
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "morning", "world", -1, NULL, NULL, NULL, false, false);
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "some", "hi", -1, NULL, "/", NULL, false, true);

    char* first_session_id  = HTTP_Auth_AddUserIfNotExists("writer,blogger", "Umpa", "Lumpa", 15, "Cats", "Somewhere in America!", 102.149981, "something", "not good", "really not good", NULL, NULL, "good");
    char* second_session_id = HTTP_Auth_AddUserIfNotExists("user", "Google", "Carmack", 29, "Dogs", "Somewhere in the Milky Way!", "harrow!", "something else", "not good", "really not good", NULL, NULL, "good");
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "SessionID", first_session_id, -1, NULL, NULL, NULL, false, true);
    // 0 Deletes the cookie!!!
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "oloooo", "ofofjlaa", 20, NULL, "/", NULL, false, true);

    response->response_body = HTTP_TemplateTextFromFile(arena, request_info, result, "static/first_page.html");

    HTTP_HandleRoute(StrArrayLit("global"), "GET", "/ooga", false, second_page_handler);
    HTTP_DeleteRouteForAllMethod("/ooga", false);
}

void login_form_handler(Arena* arena, HTTPRequestInfo* request_info, HTTPResponse* response) {
    printf("login_form_handler: `%s`\n", request_info->user_permission); 
    printf("got into login form handler!");

    HTTP_SetContentTypeHeader("text/html");

    cJSON* user_details = cJSON_CreateObject();
    cJSON_AddStringToObject(user_details, "username", "Google");
    cJSON_AddStringToObject(user_details, "password", "Carmack");
    cJSON_AddStringToObject(user_details, "age", "29");

    cJSON* login_and_user_data = HTTP_Auth_GetLoginAndUserDataAtSessionToken(HTTP_Auth_GetSessionIDAtLoginDetails(user_details), true, false, true);
    printf("login and user data: `%s`\n", cJSON_Print(login_and_user_data));
    response->response_body = HTTP_TemplateTextFromFile(arena, request_info, login_and_user_data, "static/login_form.html");
}

int main(void) {
    if (sqlite3_threadsafe() == 0) {
        fprintf(stderr, "[ERROR] The version of sqlite3 you are is not thread safe, please try to recompile it with the following flag: `SQLITE_THREADSAFE=1`.\n");
        return -1;
    }

    HTTP_Initialize();

    HTTP_CreateDatabase("new.db");
    HTTP_RunSQLQuery("CREATE TABLE IF NOT EXISTS Info(fname TEXT, lname TEXT, pname TEXT, custom TEXT, name TEXT, surname TEXT)", false, false);

    SessionMaxTimeout timeout = {
        .years = 10,
        .months = 20,
        .days = 40,
        .hours = 29,
        .minutes = 10,
        .seconds = 2000
    };

    HTTP_Auth_SessionEnable("SessionAuthTable",
            StrArrayLit("username", "password", "age"), 
            StrArrayLit("STRING", "STRING", "INTEGER"),
            StrArrayLit("Pet", "Residence", "Favourite_Decimal", "other_info"),
            StrArrayLit("STRING", "BLOB", "REAL", "STRING"),
            NULL);

    char* dirs[] = {"something_elsdafj", "static/", };
    HTTP_SetSearchDirectories(dirs, ArrayCount(dirs));

    HTTP_HandleRoute(StrArrayLit("global", "second"), "GET", "/", false, root_page_handler);
    /* HTTP_HandleRoute(StrArrayLit("global", "second"), "GET", "/", true, second_page_handler); */
    HTTP_HandleRoute(StrArrayLit("global"), "POST", "/", false, root_page_handler);

    HTTP_HandleRoute(StrArrayLit("user"), "GET", "/login_page", false, login_form_handler);
    HTTP_HandleRoute(StrArrayLit("writer"), "GET", "/second_page", false, login_form_handler);

    HTTP_HandleRoute(StrArrayLit("global"), "GET", "/good", true, root_page_handler);
    HTTP_HandleRoute(StrArrayLit("global"), "POST", "/good", true, root_page_handler);
    HTTP_HandleRoute(StrArrayLit("global"), "UPDATE", "/good", true, root_page_handler);

    HTTP_HandleRoute(StrArrayLit("global"), "GET", "/som[0-9]*", true, second_page_handler);
    HTTP_HandleRoute(StrArrayLit("global"), "GET", "/som[0-9]*", false, root_page_handler);


    HTTP_DeleteRouteForAllMethod("/good", true);
    HTTP_Set404Page("static/error_page.html");

    /* HTTP_HandleRedirectRoute("GET", "/other main website", "/"); */

    printf("\n");

    HTTP_RunServer("8000", "vendor/certs/cert.pem", "vendor/certs/key.pem");

    return 0;
}
