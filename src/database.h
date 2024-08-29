#pragma once

void HTTP_CreateDatabase(char* file_path);
void HTTP_CloseDatabase(void);
cJSON* HTTP_RunSQLQuery(char* sql_query, bool list_response);
void HTTP_InsertJSONIntoDatabase(cJSON* json_obj);
