#pragma once

typedef struct {
    cJSON* json;
    bool   convert_types;
} JSONAndTypeFormat;

HTTPEXPORTFUNC void   HTTP_CreateDatabase(char* file_path);
HTTPEXPORTFUNC void   HTTP_CloseDatabase(void);
HTTPEXPORTFUNC cJSON* HTTP_RunSQLQuery(char* sql_query, bool list_response, bool convert_types);
HTTPEXPORTFUNC void   HTTP_InsertJSONIntoDatabase(cJSON* json_obj);
HTTPEXPORTFUNC bool   HTTP_IsDatabaseConnected(void);
