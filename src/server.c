#include "global.h"
#include "http_request.h"
#include "file_handling.h"

int main(void) {
	HTTP_Initialize();

	char* dirs[] = {"static/", "../Web Server/static", "good", "C:/something", "C:/good_morning/ls", "C:\\Dev\\Real Work\\Web Server\\static\\"};
	HTTP_SetSearchDirectories(dirs, ArrayCount(dirs));
    HTTP_SetDefaultPUTDirectory("static");

	HTTP_HandleRoute("GET", "/", "static/first_page.html");
	HTTP_HandleRedirectRoute("GET", "/other main website", "/");

	printf("\n");
	HTTP_RunServer("127.0.0.1", "8000");



/* 	cJSON* thing = cJSON_CreateObject(); */
/* 	cJSON_AddItemToObject(thing, "some", cJSON_CreateString("true")); */
/* 	cJSON_AddItemToObject(thing, "other", cJSON_CreateBool(1)); */


/*     cJSON* elem = thing->child->next; */
/*     printf("BEFORE: %s\n", cJSON_Print(thing)); */
/*     cJSON* string_obj = cJSON_CreateString("IT WORKS!!"); */
/*     string_obj->string = malloc(strlen(elem->string)+1); */
/*     /1* memset(string_obj->string, 0, strlen(elem->string)+1); *1/ */
/*     strcpy(string_obj->string, elem->string); */
/*     cJSON_ReplaceItemViaPointer(thing, elem, string_obj); */
/*     elem = string_obj; */

/*     printf("After: %s\n", cJSON_Print(thing)); */

/*     printf("Finished!\n"); */


	return 0;
}
