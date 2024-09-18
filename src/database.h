#pragma once

typedef struct {
    cJSON* json;
    bool   convert_types;
} JSONAndTypeFormat;

void   HTTP_CreateDatabase(char* file_path);
void   HTTP_CloseDatabase(void);
cJSON* HTTP_RunSQLQuery(char* sql_query, bool list_response, bool convert_types);
void   HTTP_InsertJSONIntoDatabase(cJSON* json_obj);
bool   HTTP_IsDatabaseConnected(void);
