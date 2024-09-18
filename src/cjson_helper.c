#include "util.h"

cJSON* HTTP_cJSON_TurnObjectIntoArray(char* array_name, cJSON* obj) {
    cJSON* json_obj = cJSON_CreateObject();
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, obj);
    cJSON_AddItemToObject(json_obj, array_name, array);
    return json_obj;
}

bool HTTP_cJSON_IsObjectEmpty(cJSON* object) {
    if (object == NULL) {
        return true;
    }
    else if (object->child == NULL) {
        return true;
    }
    else if (!strcmp(object->child->string, "root") &&
             object->child->next == NULL &&
             object->child->child == NULL) {

        return true;
    }
    else {
        return false;
    }
}

cJSON* HTTP_cJSON_TurnStringToType(char* data) {
    cJSON* item = NULL;

    if (IsInteger(data)) {
        char* endptr;
        item = cJSON_CreateNumber(strtod(data, &endptr));
    }
    else if (!strcmp(data, "true")) {
        item = cJSON_CreateBool(1);
    }
    else if (!strcmp(data, "false")) {
        item = cJSON_CreateBool(0);
    }
    else if (!strcmp(data, "null")) {
        item = cJSON_CreateNull();
    }
    else {
        item = cJSON_CreateString(data);
    }

    return item;
}

char* HTTP_cJSON_GetStringValue(Arena* arena, cJSON* elem) {
    char* string_buf = PushString(arena, 256);
    char* string_val = cJSON_GetStringValue(elem);

    if (string_val) {
        return string_val;
    }

    double float_val = cJSON_GetNumberValue(elem);
    if (float_val - (int)float_val == 0) {
        sprintf(string_buf, "%d", (int)float_val);
    }
    else {
        sprintf(string_buf, "%f", float_val);
    }

    return string_buf;
}
