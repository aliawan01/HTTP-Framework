#include "util.h"
#include "http_request.h"
#include "file_handling.h"
#include "string_handling.h"
#include "database.h"
#include "authentication.h"

void second_page_handler(Arena* arena, HTTPRequestInfo* request_info, HTTPResponse* response) {
    response->status_code = OK_200;

    HTTP_AddHeaderToHeaderDict(arena, &response->headers, "Content-Type", "text/html");
    response->response_body = HTTP_StrLit("<h1>Recieved it</h1><p>Trying to fillout some space</p>");
}

void root_page_handler(Arena* arena, HTTPRequestInfo* request_info, HTTPResponse* response) {
    response->status_code = OK_200;

    if (!strcmp(request_info->request_method, "POST") && request_info->is_json_request) {
        HTTP_RunSQLQuery("DELETE FROM Info", false, true);
        cJSON* converted_array = HTTP_cJSON_TurnObjectIntoArray("Info", request_info->json_request_body);
        HTTP_InsertJSONIntoDatabase(converted_array);
    }

    cJSON* result = HTTP_RunSQLQuery("SELECT DISTINCT fname, lname, pname FROM Info", false, true);
    HTTP_AddHeaderToHeaderDict(arena, &response->headers, "Content-Type", "text/html");

    /* HTTP_AddHeaderToHeaderDict(arena, &response->headers, "Set-Cookie", "name=John"); */
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "good", "hello", -1, NULL, NULL, NULL, false, false);
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "morning", "world", -1, NULL, NULL, NULL, false, false);
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "some", "hi", -1, NULL, "/", NULL, false, true);
    // 0 Deletes the cookie!!!
    HTTP_AddCookieToCookieJar(arena, &response->cookie_jar, "oloooo", "ofofjlaa", 20, NULL, "/", NULL, false, true);

    response->response_body = HTTP_TemplateTextFromFile(arena, request_info, result, "static/first_page.html");

    HTTP_HandleRoute("GET", "/ooga", false, second_page_handler);
}


int main(void) {
	HTTP_Initialize();

    /* printf("%s\n", HTTP_CreateDateString(allocator.recycle_arena, StrLit("Mon"), 31, StrLit("Jun"), 2019, 7, 20, 27)); */


    HTTP_CreateDatabase("new.db");
    /* HTTP_RunSQLQuery("CREATE TABLE IF NOT EXISTS Info(fname TEXT, lname TEXT, pname TEXT, custom TEXT, name TEXT, surname TEXT)", false); */
    SessionMaxTimeout timeout = {
        .years = 10,
        .months = 20,
        .days = 40,
        .hours = 29,
        .minutes = 10,
        .seconds = 2000
    };

    HTTP_Auth_SessionEnable("SessionAuthTable",
                            StrArrayLit({"username", "password", "age"}), 
                            StrArrayLit({"STRING", "STRING", "INTEGER"}),
                            StrArrayLit({"Pet", "Residence", "Favourite_Decimal", "other_info"}),
                            StrArrayLit({"STRING", "BLOB", "REAL", "STRING"}),
                            NULL);

    char* session_token = HTTP_Auth_AddUserIfNotExists("John", "Smith", 19.32, "Cats", "good", true, 99.87, "goody22", NULL, NULL, "good");
    cJSON* create_user_data = cJSON_CreateObject();
    cJSON_AddStringToObject(create_user_data, "Favourite_Decimal", "I love decimals!"); 
    cJSON_AddNumberToObject(create_user_data, "age", -1239);
    cJSON_AddStringToObject(create_user_data, "Pet", "avalon");
    cJSON_AddStringToObject(create_user_data, "Residence", "50");
    cJSON_AddNumberToObject(create_user_data, "some", 111);
    cJSON_AddStringToObject(create_user_data, "password", "BlowTorch");
    cJSON_AddStringToObject(create_user_data, "username", "Jonathan");
    cJSON_AddNumberToObject(create_user_data, "good stuff", 1020);
    cJSON_AddNumberToObject(create_user_data, "good stuff", 999);
    cJSON_AddStringToObject(create_user_data, "other_info", "nice!"); 

    /* char* session_token = HTTP_Auth_cJSON_AddUserIfNotExists(allocator.recycle_arena, create_user_data); */

    printf("Created Session Token: `%s`\n", session_token);

    /* CookiesDict cookie_dict = { (char*[]){"SessionID"}, (char*[]){session_token}, 1}; */
    /* char* new_session_token = HTTP_Auth_CookiesDict_GenerateNewSessionTokenIfExpired(allocator.recycle_arena, cookie_dict); */
    /* printf("new_session_token: %s\n", new_session_token); */

    /* printf("Token exists: %d\n", HTTP_Auth_CheckSessionIDExists(session_token)); */
    /* printf("Token expired: %d\n", HTTP_Auth_CheckSessionIDExpired(session_token)); */
    /* printf("Token refresh expiry date: %d\n", HTTP_Auth_RefreshTokenExpiryDate(session_token)); */

    /* printf("User Data at token: `%s`\n", cJSON_Print(HTTP_Auth_GetUserDataAtSessionToken(session_token, true, true, true))); */
    /* printf("Login Data at token: `%s`\n", cJSON_Print(HTTP_Auth_GetLoginDataAtSessionToken(session_token, true, false))); */

    /* printf("Final: `%s`\n", cJSON_Print(HTTP_Auth_GetLoginAndUserDataAtSessionToken(session_token, true, true, true))); */




/*     cJSON* new_user_obj = cJSON_CreateObject(); */
/*     cJSON_AddStringToObject(new_user_obj, "username", "John"); */
/*     cJSON_AddStringToObject(new_user_obj, "password", "Haack"); */
/*     cJSON_AddNumberToObject(new_user_obj, "age", 101); */

/*     cJSON* new_login_obj = cJSON_CreateObject(); */
/*     cJSON_AddStringToObject(new_login_obj, "username", "good"); */
/*     cJSON_AddStringToObject(new_login_obj, "password", "ajfla"); */
/*     cJSON_AddStringToObject(new_login_obj, "age", "nota"); */
/*     cJSON_AddNumberToObject(new_login_obj, "Pet", 20); */
/*     cJSON_AddStringToObject(new_login_obj, "Residence", "sumbao..."); */
/*     cJSON_AddStringToObject(new_login_obj, "Favourite_Decimal", "googa!!"); */
/*     cJSON_AddNumberToObject(new_login_obj, "other_info", 104.135803291); */

/*     printf("[BEFORE] Session ID For Login Data: `%s`\n", HTTP_Auth_GetSessionIDAtLoginDetails(allocator.recycle_arena, new_user_obj)); */
/*     HTTP_Auth_SetLoginAndUserDataAtSessionToken(allocator.recycle_arena, session_token, new_login_obj); */
/*     printf("[AFTER 1] Session ID For Login Data: `%s`\n", HTTP_Auth_GetSessionIDAtLoginDetails(allocator.recycle_arena, new_user_obj)); */
/*     printf("[AFTER 2] Session ID For Login Data: `%s`\n", HTTP_Auth_GetSessionIDAtLoginDetails(allocator.recycle_arena, new_login_obj)); */
    /* HTTP_Auth_SetUserDataAtSessionToken(allocator.recycle_arena, session_token, new_login_obj); */
    /* printf("Changing login data: %s\n", HTTP_Auth_SetLoginDataAtSessionToken(allocator.recycle_arena, session_token, new_login_obj)); */


	return 0;
}
