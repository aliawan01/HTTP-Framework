#pragma once

cJSON* HTTP_cJSON_TurnObjectIntoArray(char* array_name, cJSON* obj);
bool   HTTP_cJSON_IsObjectEmpty(cJSON* object);
cJSON* HTTP_cJSON_TurnStringToType(char* data);
char*  HTTP_cJSON_GetStringValue(Arena* arena, cJSON* elem);
