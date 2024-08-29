#include "util.h"

cJSON* HTTP_cJSON_TurnObjectIntoArray(char* array_name, cJSON* obj) {
    cJSON* json_obj = cJSON_CreateObject();
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, obj);
    cJSON_AddItemToObject(json_obj, array_name, array);
    return json_obj;
}


