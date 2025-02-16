#include "http_util.h"
#include "http_file_handling.h"
#include "http_string_handling.h"
#include "http_database.h"

void HTTP_CreateDatabase(char* file_path) {
    http_database.initialized = true;

    int error_code;
    error_code = sqlite3_open(file_path, &http_database.database);

    if (HTTP_FindFileSize(file_path) == -1) {
        HTTP_Log(HTTP_INFO, "[INFO] HTTP_CreateDatabase() Found an existing database at the path: `%s`\n", file_path);
        return;
    }

    if (error_code != SQLITE_OK) {
        HTTP_Log(HTTP_ERROR, "[ERROR] HTTP_CreateDatabase() An error has occured: `%s`\n", sqlite3_errmsg(http_database.database));
    }
    else {
        HTTP_Log(HTTP_INFO, "[INFO] HTTP_CreateDatabase() Successfully created database at file path: `%s`\n", file_path);
    }
}

void HTTP_CloseDatabase(void) {
    if (http_database.initialized) {
        HTTP_Log(HTTP_INFO, "[INFO] HTTP_CloseDatabase() Successfully closed database.\n");
        sqlite3_close(http_database.database);
    }
    else {
        HTTP_Log(HTTP_WARNING, "[WARNING] HTTP_CloseDatabase() No database has been attached so it can't be closed.\n");
    }
}

static int SQLQueryNormalCallback(void* callback_data, int count, char** data, char** column) {
    JSONAndTypeFormat* cb_data = (JSONAndTypeFormat*)callback_data;
    cJSON* json_obj = cb_data->json;

    cJSON* item = NULL;
    for (int i = 0; i < count; i++) {
        if (data[i] == NULL) {
            item = cJSON_CreateNull();
        }
        else if (cb_data->convert_types) {
            item = HTTP_cJSON_TurnStringToType(data[i]);
        }
        else {
            item = cJSON_CreateString(data[i]);
        }

        cJSON_AddItemToObject(json_obj, column[i], item);
    }

    return 0;
}

static int SQLQueryListCallback(void* callback_data, int count, char** data, char** column) {
    JSONAndTypeFormat* cb_data = (JSONAndTypeFormat*)callback_data;
    cJSON* json_array = cb_data->json;
    cJSON* current_obj = cJSON_CreateObject();
    cJSON* item = NULL;

    for (int i = 0; i < count; i++) {
        if (data[i] == NULL) {
            item = cJSON_CreateNull();
        }
        else if (cb_data->convert_types) {
            item = HTTP_cJSON_TurnStringToType(data[i]);
        }
        else {
            item = cJSON_CreateString(data[i]);
        }

        cJSON_AddItemToObject(current_obj, column[i], item);
    }

    cJSON_AddItemToArray(json_array, current_obj);

    return 0;
}

cJSON* HTTP_RunSQLQuery(char* sql_query, bool list_response, bool convert_types) {
    cJSON* cjson_obj = cJSON_CreateObject();

    if (!http_database.initialized) {
        HTTP_Log(HTTP_ERROR, "[ERROR] HTTP_RunSQLQuery() cannot run query as no database has been attached.\n");
        return cjson_obj;
    }

    int result_code;
    char* error_message;

    JSONAndTypeFormat callback_data;
    callback_data.convert_types = convert_types;
    // TODO: Make cJSON use the recycle_arena or something like that so that we can eventually
    //       free the memory used form cJSON back to the arena.
    if (list_response) {
        cJSON* cjson_array = cJSON_CreateArray();
        callback_data.json = cjson_array;

        result_code = sqlite3_exec(http_database.database, sql_query, SQLQueryListCallback, &callback_data, &error_message);
        cJSON_AddItemToObject(cjson_obj, "root", cjson_array);
    }
    else {
        callback_data.json = cjson_obj;
        result_code = sqlite3_exec(http_database.database, sql_query, SQLQueryNormalCallback, &callback_data, &error_message);
    }
        
    if (result_code != SQLITE_OK) {
        HTTP_Log(HTTP_ERROR, "[ERROR] HTTP_RunSQLQuery() An error has occured: `%s`\n", error_message);
        sqlite3_free(error_message);
        return NULL;
    }

    return cjson_obj;
}

void HTTP_InsertJSONIntoDatabase(cJSON* json_obj) {
    Temp scratch = GetScratch(0, 0);

    cJSON* array = NULL;
    cJSON_ArrayForEach(array, json_obj) {
        if (!cJSON_IsArray(array)) {
            HTTP_Log(HTTP_ERROR, "[ERROR] HTTP_InsertJSONIntoDatabase() JSON object with key: `%s` is not an array.\n", array->string);
            DeleteScratch(scratch);
            return;
        }

        cJSON* query_obj = NULL;
        cJSON_ArrayForEach(query_obj, array) {
            if (!cJSON_IsObject(query_obj)) {
                HTTP_Log(HTTP_ERROR, "[ERROR] HTTP_InsertJSONIntoDatabase() JSON Object with key: `%s` is not an object.\n", query_obj->string);
                DeleteScratch(scratch);
                return;
            }

            char* insert_into_statement = PushString(scratch.arena, 5024);
            char* values_statement = PushString(scratch.arena, 1024);

            sprintf(insert_into_statement, "INSERT INTO %s(", array->string);
            strcat(values_statement, "VALUES (");

            cJSON* query_obj_data = NULL;
            bool first_object = false;
            cJSON_ArrayForEach(query_obj_data, query_obj) {
                if (cJSON_IsObject(query_obj_data)) {
                    HTTP_Log(HTTP_ERROR, "[ERROR] HTTP_InsertJSONIntoDatabase() JSON Object with key: `%s` is an nested object which is not supported.\n", query_obj_data->string);
                    DeleteScratch(scratch);
                    return;
                }

                if (!first_object) {
                    first_object = true;
                }
                else {
                    strcat(insert_into_statement, ", ");
                    strcat(values_statement, ",");
                }

                strcat(insert_into_statement, query_obj_data->string);
                strcat(values_statement, "'");
                strcat(values_statement, HTTP_cJSON_GetStringValue(scratch.arena, query_obj_data));
                strcat(values_statement, "'");
            }

            strcat(insert_into_statement, ") ");
            strcat(values_statement, ");");
            strcat(insert_into_statement, values_statement);

            if (HTTP_RunSQLQuery(insert_into_statement, false, true) == NULL) {
                HTTP_Log(HTTP_ERROR, "[ERROR] HTTP_InsertJSONIntoDatabase() An error occured when running the SQL code to insert JSON data into the database.");
            }
        }
    }

    DeleteScratch(scratch);
}

bool HTTP_IsDatabaseConnected(void) {
    if (http_database.initialized) {
        return true;
    }
    else {
        return false;
    }
}
