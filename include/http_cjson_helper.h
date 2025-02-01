#pragma once

HTTPEXPORTFUNC cJSON* HTTP_cJSON_TurnObjectIntoArray(char* array_name, cJSON* obj);
HTTPEXPORTFUNC bool   HTTP_cJSON_IsObjectEmpty(cJSON* object);
HTTPEXPORTFUNC cJSON* HTTP_cJSON_TurnStringToType(char* data);
HTTPEXPORTFUNC char*  HTTP_cJSON_GetStringValue(Arena* arena, cJSON* elem);
HTTPEXPORTFUNC void   HTTP_cJSON_AddPermissionArrayToObject(cJSON* object, StringArray permissions);
