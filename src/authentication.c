#include "http_util.h"
#include "http_file_handling.h"
#include "http_string_handling.h"
#include "http_request.h"
#include "http_database.h"
#include "http_authentication.h"

void HTTP_Auth_SessionEnable(char* auth_table_name,
                             StringArray login_data,
                             StringArray login_data_types,
                             StringArray user_data,
                             StringArray user_data_types,
                             SessionMaxTimeout* timeout) {

    if (session_auth.initialized) {
        printf("[WARNING] HTTP_Auth_SessionEnable() Already enabled Session Authentication using table: `%s`\n", session_auth.auth_table_name);
        return;
    }

    if (!HTTP_IsDatabaseConnected()) {
        printf("[ERROR] HTTP_Auth_SessionEnable() Cannot use session authentication as no database has been attached, try running HTTP_CreateDatabase().\n");
        return;
    }

    // TODO: Remove this after testing.
    HTTP_RunSQLQuery("DROP TABLE SessionAuthTable;", false, false);
    
    char check_table_exists_query[256] = {0};
    sprintf(check_table_exists_query, "SELECT name FROM sqlite_master WHERE type='table' AND name='{%s}'", auth_table_name);
    cJSON* table_already_exists = HTTP_RunSQLQuery(check_table_exists_query, false, false);
    if (table_already_exists == NULL) {
        printf("[ERROR] HTTP_Auth_SessionEnable() Error occured when running SQL to check if the table already exists in the database.\n");
        return;
    }

    if (HTTP_cJSON_IsObjectEmpty(table_already_exists)) {
        printf("[WARNING] HTTP_Auth_SessionEnable() A table with the name `%s` already exists in the database so we cannot create a new table there. This table is being used for session authentication.\n", auth_table_name);
    }


    // TODO: Allow the user to specify fields to generate sessionID from.
    //       If the table already exists and there are fields which need to be changed
    //       then use the ALTER command to do so.
    if (login_data.count > login_data_types.count) {
        printf("[ERROR] HTTP_Auth_SessionEnable() Not each login data field has a specified type.\n");
        return;
    }
    else if (login_data.count < login_data_types.count) {
        printf("[ERROR] HTTP_Auth_SessionEnable() Too many login data types for the number of fields.\n");
        return;
    }

    if (user_data.count > user_data_types.count) {
        printf("[ERROR] HTTP_Auth_SessionEnable() Not each user data field has a specified type.\n");
        return;
    }
    else if (user_data.count < user_data_types.count) {
        printf("[ERROR] HTTP_Auth_SessionEnable() Too many user data types for the number of fields.\n");
        return;
    }

    if (timeout != NULL) {
        bool timeout_checks_failed = false;

        // TODO: sizeof(SessionMaxTimeout)/sizeof(int) won't work if we start adding other types into SessionMaxTimeout.
        for (int i = 0; i < sizeof(SessionMaxTimeout)/sizeof(int); i++) {
            if (*((int*)timeout+i) < 0) {
                printf("[ERROR] HTTP_Auth_SessionEnable() Timeout value for %s is less than 0: `%d`.\n", timeout_field_names[i], *((int*)timeout+i));
                timeout_checks_failed = true;
            }
        }

        if (timeout_checks_failed) {
            return;
        }
    }


    for (int i = 0; i < login_data.count; i++) {
        if (!strcmp(login_data.array[i], "SessionID")) {
            printf("[ERROR] HTTP_Auth_SessionEnable() One of the fields in the login data contains the name `SessionID` which conflicts with the field the system uses to calculate when a session is expired, please rename this field.\n");
            return;
        }
        else if (!strcmp(login_data.array[i], "__Session_ExpiryDate")) {
            printf("[ERROR] HTTP_Auth_SessionEnable() One of the fields in the login data contains the name `__Session_ExpiryDate` which conflicts with the field the system uses to calculate when a session is expired, please rename this field.\n");
            return;
        }
        else if (!strcmp(login_data.array[i], "__Session_Permissions")) {
            printf("[ERROR] HTTP_Auth_SessionEnable() One of the fields in the login data contains the name `__Session_Permissions` which conflicts with the field the system uses to keep track of the permissions of each user, please rename this field.\n");
            return;
        }
    }

    for (int i = 0; i < user_data.count; i++) {
        if (!strcmp(user_data.array[i], "SessionID")) {
            printf("[ERROR] HTTP_Auth_SessionEnable() One of the fields in the user data contains the name `SessionID` which conflicts with the field the system uses to calculate when a session is expired, please rename this field.\n");
            return;
        }
        else if (!strcmp(user_data.array[i], "__Session_ExpiryDate")) {
            printf("[ERROR] HTTP_Auth_SessionEnable() One of the fields in the user data contains the name `__Session_ExpiryDate` which conflicts with the field the system uses to calculate when a session is expired, please rename this field.\n");
            return;
        }
        else if (!strcmp(user_data.array[i], "__Session_Permissions")) {
            printf("[ERROR] HTTP_Auth_SessionEnable() One of the fields in the login data contains the name `__Session_Permissions` which conflicts with the field the system uses to keep track of the permissions of each user, please rename this field.\n");
            return;
        }
    }

    char create_auth_table_query[512] = {0};

    sprintf(create_auth_table_query, "CREATE TABLE IF NOT EXISTS %s(SessionID VARCHAR(256) PRIMARY KEY, __Session_ExpiryDate TEXT, __Session_Permissions TEXT", auth_table_name);

    StringArray fields_to_insert[2] = { login_data, user_data };
    StringArray types_to_insert[2] = { login_data_types, user_data_types };
    for (int x = 0; x < ArrayCount(fields_to_insert); x++) {
        for (int i = 0; i < fields_to_insert[x].count; i++) {
            if (ContainsWhitespace(fields_to_insert[x].array[i])) {
                printf("[ERROR] HTTP_Auth_SessionEnable() Field: `%s` at index: `%d` contains a whitespace which is not allowed.\n", fields_to_insert[x].array[i], i);
                return;
            }
            strcat(create_auth_table_query, ", '");
            strcat(create_auth_table_query, fields_to_insert[x].array[i]);
            strcat(create_auth_table_query, "' ");
            strcat(create_auth_table_query, types_to_insert[x].array[i]);
        }
    }

    strcat(create_auth_table_query, ");");
    printf("create_auth_table_query: %s\n", create_auth_table_query);

    cJSON* output = HTTP_RunSQLQuery(create_auth_table_query, false, true);
    if (output != NULL) {
        printf("[INFO] HTTP_Auth_SessionEnable() Enabled Session Authentication using table: `%s`\n", auth_table_name);
        session_auth = (SessionAuthInfo) {
            .initialized = true,
            .auth_table_name = HTTP_StringDup(allocator.permanent_arena, auth_table_name),
            .login_data = login_data,
            .user_data = user_data,
            .timeout = timeout == NULL ? (SessionMaxTimeout) { .days = 30 } : *timeout // NOTE: If no timeout is specified, the default timeout is set to 30 days.
        };
    }
    else {
        printf("[ERROR] HTTP_Auth_SessionEnable() Error when creating the table.\n");
    }
}

bool HTTP_Auth_SessionCheckIsEnabled(void) {
    if (session_auth.initialized) {
        return true;
    }
    else {
        return false;
    }
}

static void GenerateTimeoutField(char* timeout_field_string, int count) {
    Assert(count > 255);

    strcat(timeout_field_string, "datetime('now'");

    char timeout_buf[128] = {0};
    for (int i = 0; i < sizeof(SessionMaxTimeout)/sizeof(int); i++) {
        int value = *((int*)(&session_auth.timeout)+i);
        if (value != 0) {
            sprintf(timeout_buf, ", '+%d %s'", value, timeout_field_names[i]);
            strcat(timeout_field_string, timeout_buf);
        }
    }

    strcat(timeout_field_string, ")");
}

static bool CheckDataFieldsAreCorrect(cJSON* new_data, StringArray fields, char* calling_function_name, char* data_type, bool check_for_missing_fields) {
    bool found_matching_fields_array[64] = {0};

    cJSON* elem = NULL;
    cJSON_ArrayForEach(elem, new_data) {
        for (int i = 0; i < fields.count; i++) {
            if (!strcmp(elem->string, fields.array[i])) {
                if (cJSON_IsNull(elem)) {
                    printf("[ERROR] %s The %s data field `%s` contains a NULL value which is invalid.\n", calling_function_name, data_type, elem->string);
                    return false;
                }
                else if (!found_matching_fields_array[i]) {
                    found_matching_fields_array[i] = true;
                    break;
                }
                else {
                    printf("[ERROR] %s The %s data field `%s` contains a duplicate entry which is not allowed.\n", calling_function_name, data_type, elem->string);
                    return false;
                }
            }
        }
    }

    if (check_for_missing_fields) {
        bool contains_missing_fields = false;
        for (int i = 0; i < fields.count; i++) {
            if (!found_matching_fields_array[i]) {
                printf("[ERROR] %s The %s data is missing the field: `%s`\n", calling_function_name, data_type, fields.array[i]);
                contains_missing_fields = true;
            }
        }

        if (contains_missing_fields) {
            return false;
        }
    }

    return true;
}

char* __HTTP_Auth_AddUserIfNotExists(Arena* arena, ...) {
    if (!session_auth.initialized) {
        printf("[ERROR] __HTTP_Auth_AddUserIfNotExists() Haven't enabled authentication, enable it by calling the HTTP_Auth_SessionEnable() function.\n");
        return NULL;
    }

    va_list args;
    va_start(args, arena);

    char  user_exists_query_string[2056] = {0};
    char  insert_user_query_string[2056] = {0};
    char  insert_user_query_string_arguments[1024] = {0};
    char  insert_user_query_arguments_array[124][512] = {0};
    char  next_string[512] = {0};

    int null_count = 0;
    int count;
    bool finished = false;
    for (int i = 0; !finished; i++) {
        switch (va_arg(args, int)) {
            case VARIADIC_INT16:
                sprintf(next_string, "%hd", va_arg(args, int16_t));
                break;
            case VARIADIC_UINT16:
                sprintf(next_string, "%hu", va_arg(args, uint16_t));
                break;
            case VARIADIC_INT32:
                sprintf(next_string, "%d", va_arg(args, int32_t));
                break;
            case VARIADIC_UINT32:
                sprintf(next_string, "%u", va_arg(args, uint32_t));
                break;
            case VARIADIC_INT64:
                sprintf(next_string, "%lld", va_arg(args, int64_t));
                break;
            case VARIADIC_UINT64:
                sprintf(next_string, "%llu", va_arg(args, uint64_t));
                break;
            case VARIADIC_CHAR:
                sprintf(next_string, "%c", va_arg(args, char));
                break;
            case VARIADIC_LONG_DOUBLE:
                sprintf(next_string, "%Lf", va_arg(args, long double)); 
                break;
            case VARIADIC_DOUBLE:
                sprintf(next_string, "%f", va_arg(args, double));
                break;
            case VARIADIC_STRING:
                sprintf(next_string, "%s", va_arg(args, char*));
                break;
            case VARIADIC_NULL: 
                va_arg(args, void*);
                printf("Received a variadic null at index: `%d`\n", i);
                null_count++;
                continue;
                break;
            case VARIADIC_UNKNOWN:
                printf("[ERROR] __HTTP_Auth_AddUserIfNotExists(): Got VARIADIC_UNKNOWN at index: `%d`\n", i);
                Assert(false);
                break;
            case VARIADIC_END:
                printf("Finished looping at index: `%d`\n", i);
                count = i;
                finished = true;
                break;
        };

        Assert(strlen(next_string) < 512);
        strcpy(insert_user_query_arguments_array[i], next_string);
    }

    printf("Original Count: %d, NULL Count: %d, Not-NULL Count: %d\n\n", count, null_count, count-null_count);

    if (count-null_count < session_auth.login_data.count+1) {
        printf("[ERROR] HTTP_Auth_AddUserIfNotExists() Not enough valid arguments to identify the new user, required valid arguments: `%d`, supplied valid arguments: `%d`, arguments which have a value of NULL: `%d`.\n", session_auth.login_data.count+1, count-null_count, null_count);
        return NULL;
    }

    sprintf(user_exists_query_string, "SELECT * FROM %s WHERE ", session_auth.auth_table_name);
    sprintf(insert_user_query_string, "INSERT INTO %s(SessionID, __Session_Permissions, ", session_auth.auth_table_name);

    char* session_id = PushString(arena, HTTP_SESSION_ID_LENGTH);
    HTTP_Gen256ByteRandomNum(session_id, HTTP_SESSION_ID_LENGTH);

    char* permissions_string = insert_user_query_arguments_array[0][0] == 0 ? "global" : insert_user_query_arguments_array[0]; 
    sprintf(insert_user_query_string_arguments, "VALUES ('%s', '%s', ", session_id, permissions_string);

    int i = 0;
    for (; i < session_auth.login_data.count; i++) {
        if (insert_user_query_arguments_array[i+1][0] == 0) {
            printf("[ERROR] HTTP_Auth_AddUserIfNotExists() The required login field `%s` has been set to NULL, please change it to an valid value.\n", session_auth.login_data.array[i]);
            return NULL;
        }

        strcat(user_exists_query_string, session_auth.login_data.array[i]);
        strcat(user_exists_query_string, "='");
        strcat(user_exists_query_string, insert_user_query_arguments_array[i+1]);
        strcat(user_exists_query_string, "'");

        strcat(insert_user_query_string, "'");
        strcat(insert_user_query_string, session_auth.login_data.array[i]);
        strcat(insert_user_query_string, "'");

        strcat(insert_user_query_string_arguments, "'");
        strcat(insert_user_query_string_arguments, insert_user_query_arguments_array[i+1]);
        strcat(insert_user_query_string_arguments, "'");

        strcat(insert_user_query_string_arguments, ", ");
        if (i < session_auth.login_data.count-1) {
            strcat(user_exists_query_string, " AND ");
            strcat(insert_user_query_string, ", ");
        }
    }

    strcat(user_exists_query_string, ";");

    int max_count = session_auth.login_data.count+session_auth.user_data.count; 
    for (int x = 0; i < count && i < max_count; i++, x++) {
        if (insert_user_query_arguments_array[i+1][0] == 0) {
            continue;
        }

        strcat(insert_user_query_string, ", '");
        strcat(insert_user_query_string, session_auth.user_data.array[x]);
        strcat(insert_user_query_string, "'");

        strcat(insert_user_query_string_arguments, "'");
        strcat(insert_user_query_string_arguments, insert_user_query_arguments_array[i+1]);
        strcat(insert_user_query_string_arguments, "', ");
    }

    strcat(insert_user_query_string, ", '__Session_ExpiryDate') ");

    char timeout_field_string[256] = {0};
    GenerateTimeoutField(timeout_field_string, 256);

    strcat(insert_user_query_string_arguments, timeout_field_string);
    strcat(insert_user_query_string_arguments, ");");

    Assert(strlen(insert_user_query_string)+strlen(insert_user_query_string_arguments) < 2056);

    strcat(insert_user_query_string, insert_user_query_string_arguments);

    printf("Full query_string: `%s`\n", insert_user_query_string);
    printf("Full Select query: %s\n", user_exists_query_string);

    cJSON* user_exists_query_result = HTTP_RunSQLQuery(user_exists_query_string, false, false);

    if (user_exists_query_result == NULL) {
        printf("[ERROR] HTTP_Auth_AddUserIfNotExists() An error occured when running the SQL query to check if the user exists. Perhaps you have inserted an incorrect data-type into a field?\n");
        return NULL;
    }

    if (HTTP_cJSON_IsObjectEmpty(user_exists_query_result)) {
        printf("It is empty! Adding user to the database\n");
        printf("SessionID being inserted: %s\n", session_id);

        if (HTTP_RunSQLQuery(insert_user_query_string, false, false) == NULL) {
            printf("[ERROR] HTTP_Auth_AddUserIfNotExists() An error occured when running the SQL query to insert a new user into the database.\n");
            return NULL;
        }

        return session_id;
    }

    return NULL;
}

char* HTTP_Auth_cJSON_AddUserIfNotExists(Arena* arena, cJSON* data) {
    if (!session_auth.initialized) {
        printf("[ERROR] HTTP_Auth_cJSON_AddUserIfNotExists() Haven't enabled authentication, enable it by calling the HTTP_Auth_SessionEnable() function.\n");
        return NULL;
    }
    else if (!CheckDataFieldsAreCorrect(data, session_auth.login_data, "HTTP_Auth_cJSON_AddUserIfNotExists()", "login", true)) {
        return NULL;
    }
    else if (!CheckDataFieldsAreCorrect(data, session_auth.user_data, "HTTP_Auth_cJSON_AddUserIfNotExists()", "user", false)) {
        return NULL;
    }

    cJSON* elem = NULL;
    cJSON_ArrayForEach(elem, data) {
        if (cJSON_IsObject(elem)) {
            printf("[ERROR] HTTP_Auth_cJSON_AddUserIfNotExists() The JSON data you have provided contains nested objects which are not supported.\n");
            return NULL;
        }
    }

    Temp scratch = GetScratch(&arena, 1);

    char permission_string[1024] = {0};
    bool contains_permissions = false;
    cJSON_ArrayForEach(elem, data) {
        if (!strcmp(elem->string, "permissions")) {
            contains_permissions = true;
            char* has_permission_string = cJSON_GetStringValue(elem); 
            if (has_permission_string) {
                strcpy(permission_string, has_permission_string);
            }
            else if (cJSON_IsArray(elem)) {
                // NOTE: Array is empty.
                if (elem->child == NULL) {
                    contains_permissions = false;
                    break;
                }

                cJSON* permission_elem = NULL;
                cJSON_ArrayForEach(permission_elem, elem) {
                    if (cJSON_IsObject(permission_elem)) {
                        printf("[ERROR] HTTP_Auth_cJSON_AddUserIfNotExists() The permissions you have specified contains nested objects which is invalid.\n");
                        DeleteScratch(scratch);
                        return NULL;
                    }
                    else if (cJSON_IsArray(permission_elem)) {
                        printf("[ERROR] HTTP_Auth_cJSON_AddUserIfNotExists() The permissions you have specified contains a nested array which is invalid.\n");
                        DeleteScratch(scratch);
                        return NULL;
                    }

                    strcat(permission_string, HTTP_cJSON_GetStringValue(scratch.arena, permission_elem));
                    strcat(permission_string, ",");
                }
            }
            else {
                printf("[ERROR] HTTP_Auth_cJSON_AddUserIfNotExists() The permissions you have specified is neither a string or an array of strings, therefore it is invalid.\n");
                return NULL;
                DeleteScratch(scratch);
            }
            
            break;
        }
    }

    if (!contains_permissions) {
        strcpy(permission_string, "global");
    }

    printf("permissions: `%s`\n", permission_string);

    char user_exists_query_string[2056] = {0};
    char insert_user_query_string[2056] = {0};
    char insert_user_query_string_arguments[1024] = {0};

    char* session_id = PushString(arena, HTTP_SESSION_ID_LENGTH);
    HTTP_Gen256ByteRandomNum(session_id, HTTP_SESSION_ID_LENGTH);

    char timeout_field_string[256] = {0};
    GenerateTimeoutField(timeout_field_string, 256);

    sprintf(user_exists_query_string, "SELECT * FROM %s WHERE ", session_auth.auth_table_name);
    sprintf(insert_user_query_string, "INSERT INTO %s(SessionID, __Session_ExpiryDate, __Session_Permissions", session_auth.auth_table_name);
    sprintf(insert_user_query_string_arguments, ") VALUES ('%s', %s, '%s'", session_id, timeout_field_string, permission_string);

    bool prev_user_exists_element_added = false;
    cJSON_ArrayForEach(elem, data) {
        for (int i = 0; i < session_auth.login_data.count; i++) {
            if (!strcmp(elem->string, session_auth.login_data.array[i])) {
                if (prev_user_exists_element_added) {
                    strcat(user_exists_query_string, " AND ");
                }
                else {
                    prev_user_exists_element_added = true;
                }

                strcat(user_exists_query_string, session_auth.login_data.array[i]);
                strcat(user_exists_query_string, "='");
                strcat(user_exists_query_string, HTTP_cJSON_GetStringValue(scratch.arena, elem));
                strcat(user_exists_query_string, "'");

                strcat(insert_user_query_string, ", '");
                strcat(insert_user_query_string, session_auth.login_data.array[i]);
                strcat(insert_user_query_string, "'");

                strcat(insert_user_query_string_arguments, ", '");
                strcat(insert_user_query_string_arguments, HTTP_cJSON_GetStringValue(scratch.arena, elem));
                strcat(insert_user_query_string_arguments, "'");
            }
        }

        for (int i = 0; i < session_auth.user_data.count; i++) {
            if (!strcmp(elem->string, session_auth.user_data.array[i])) {

                strcat(insert_user_query_string, ", '");
                strcat(insert_user_query_string, session_auth.user_data.array[i]);
                strcat(insert_user_query_string, "'");

                strcat(insert_user_query_string_arguments, ", '");
                strcat(insert_user_query_string_arguments, HTTP_cJSON_GetStringValue(scratch.arena, elem));
                strcat(insert_user_query_string_arguments, "'");
            }
        }
    }

    strcat(user_exists_query_string, ";");
    strcat(insert_user_query_string_arguments, ");");
    strcat(insert_user_query_string, insert_user_query_string_arguments);

    printf("cJSON Check User exists query: `%s`\n", user_exists_query_string);
    printf("cJSON Insert User query: `%s`\n", insert_user_query_string);

    cJSON* user_exists_query_result = HTTP_RunSQLQuery(user_exists_query_string, false, false);
    if (user_exists_query_result == NULL) {
        printf("[ERROR] HTTP_Auth_cJSON_AddUserIfNotExists() An error occured when running the SQL query to check if the user exists. Perhaps you have inserted an incorrect data-type into a field?\n");
    }
    else if (HTTP_cJSON_IsObjectEmpty(user_exists_query_result)) {
        printf("cJSON It is empty! Adding user to the database\n");
        printf("cJSON SessionID being inserted: %s\n", session_id);

        if (HTTP_RunSQLQuery(insert_user_query_string, false, false) == NULL) {
            printf("[ERROR] HTTP_Auth_cJSON_AddUserIfNotExists() An error occured when running the SQL query to insert a new user into the database.\n");
        }
        else {
            DeleteScratch(scratch);
            return session_id;
        }
    }

    DeleteScratch(scratch);
    return NULL;
}

void HTTP_Auth_DeleteUserAtSessionToken(char* session_id) {
    char query_string[1024] = {0};
    sprintf(query_string, "DELETE FROM %s WHERE SessionID='%s';", session_auth.auth_table_name, session_id);

    if (HTTP_RunSQLQuery(query_string, false, true) == NULL) {
        printf("[ERROR] HTTP_Auth_DeleteUserAtSessionToken() An error occured when running the SQL code for deleting the user with the Session ID provided.\n");
    }
}

void HTTP_Auth_CookiesDict_DeleteUserAtSessionToken(CookiesDict cookies) {
    for (int i = 0; i < cookies.count; i++) {
        if (!strcmp(cookies.keys[i], "SessionID")) {
            HTTP_Auth_DeleteUserAtSessionToken(cookies.values[i]);
            return;
        }
    }
}

char* HTTP_Auth_CookiesDict_GenerateNewSessionTokenIfExpired(Arena* arena, CookiesDict cookies) {
    for (int i = 0; i < cookies.count; i++) {
        if (!strcmp(cookies.keys[i], "SessionID")) {
            return HTTP_Auth_GenerateNewSessionTokenIfExpired(arena, cookies.values[i]);
        }
    }

    return NULL;
}

char* HTTP_Auth_GenerateNewSessionTokenIfExpired(Arena* arena, char* check_session_id) {
    char query_string[1024] = {0};

    sprintf(query_string, "SELECT SessionID, __Session_ExpiryDate FROM %s WHERE SessionID='%s' AND __Session_ExpiryDate < datetime('now');", session_auth.auth_table_name, check_session_id);
    cJSON* session_token_valid_query_result = HTTP_RunSQLQuery(query_string, false, true);
    if (session_token_valid_query_result == NULL) {
        printf("[ERROR] HTTP_Auth_GenerateNewSessionTokenIfExpired() An error occured when running the SQL code to check if the user's Session ID was expired.\n");
        return NULL;
    }

    if (!HTTP_cJSON_IsObjectEmpty(session_token_valid_query_result)) {
        printf("[INFO] HTTP_Auth_GenerateNewSessionTokenIfExpired() Old Session ID: `%s` expired, generating a new Session ID.\n", check_session_id);

        char* session_id = PushString(arena, HTTP_SESSION_ID_LENGTH);
        HTTP_Gen256ByteRandomNum(session_id, HTTP_SESSION_ID_LENGTH);

        char timeout_field_string[256] = {0};
        GenerateTimeoutField(timeout_field_string, 256);

        sprintf(query_string, "UPDATE %s SET SessionID='%s', __Session_ExpiryDate=%s WHERE SessionID='%s';", session_auth.auth_table_name, session_id, timeout_field_string, check_session_id);

        if (HTTP_RunSQLQuery(query_string, false, true) == NULL) {
            printf("[ERROR] HTTP_Auth_GenerateNewSessionTokenIfExpired() An error occured when running the SQL code to create a new session id.\n");
            return NULL;
        }
        else {
            printf("[INFO] HTTP_Auth_GenerateNewSessionTokenIfExpired() Successfully created new Session ID: `%s`\n", session_id);
        }

        return session_id;
    }

    return NULL;
}

bool HTTP_Auth_CheckSessionIDExpired(char* session_id) {
    char query_string[1024] = {0};
    sprintf(query_string, "SELECT SessionID FROM %s WHERE SessionID='%s' AND __Session_ExpiryDate < datetime('now');", session_auth.auth_table_name, session_id);

    cJSON* session_token_expired_result = HTTP_RunSQLQuery(query_string, false, true);
    if (session_token_expired_result == NULL) {
        printf("[ERROR] HTTP_Auth_CheckSessionIDIsExpired() An error occured when running the SQL code to check if the user's Session ID was expired.\n");
        return false;
    }

    if (!HTTP_cJSON_IsObjectEmpty(session_token_expired_result)) {
        return true;
    }
    else {
        return false;
    }
}

bool HTTP_Auth_CheckSessionIDExists(char* session_id) {
    char query_string[1024] = {0};
    sprintf(query_string, "SELECT SessionID FROM %s WHERE SessionID='%s';", session_auth.auth_table_name, session_id);

    cJSON* session_token_exists_result = HTTP_RunSQLQuery(query_string, false, true);
    if (session_token_exists_result == NULL) {
        printf("[ERROR] HTTP_Auth_CheckSessionIDExists() An error occured when running the SQL code to check if the user's Session ID exists.\n");
        return false;
    }

    if (!HTTP_cJSON_IsObjectEmpty(session_token_exists_result)) {
        return true;
    }
    else {
        return false;
    }
}

bool HTTP_Auth_RefreshTokenExpiryDate(char* check_session_id) {
    char query_string[1024] = {0};

    sprintf(query_string, "SELECT SessionID FROM %s WHERE SessionID='%s';", session_auth.auth_table_name, check_session_id);

    cJSON* session_token_valid_query_result = HTTP_RunSQLQuery(query_string, false, true);
    if (session_token_valid_query_result == NULL) {
        printf("[ERROR] HTTP_Auth_GenerateNewSessionToken() An error occured when running the SQL code to check if the user's Session ID was expired.\n");
        return false;
    }

    if (!HTTP_cJSON_IsObjectEmpty(session_token_valid_query_result)) {
        printf("[INFO] HTTP_Auth_RefreshTokenExpiryDate() Refreshing Expiry Date for Session ID: `%s`.\n", check_session_id);

        char timeout_field_string[256] = {0};
        GenerateTimeoutField(timeout_field_string, 256);

        sprintf(query_string, "UPDATE %s SET __Session_ExpiryDate=%s WHERE SessionID='%s';", session_auth.auth_table_name, timeout_field_string, check_session_id);

        if (HTTP_RunSQLQuery(query_string, false, true) == NULL) {
            printf("[ERROR] HTTP_Auth_RefreshTokenExpiryDate() An error occured when running the SQL code to refresh the Expiry Date of the Session ID.\n");
            return false;
        }
        else {
            printf("[INFO] HTTP_Auth_RefreshTokenExpiryDate() Successfully refreshed Expiry Date for Session ID: `%s`\n", check_session_id);
        }

        return true;

    }

    return false;
}

bool HTTP_Auth_CookiesDict_RefreshTokenExpiryDate(CookiesDict cookies) {
    for (int i = 0; i < cookies.count; i++) {
        if (!strcmp(cookies.keys[i], "SessionID")) {
            return HTTP_Auth_RefreshTokenExpiryDate(cookies.values[i]);
        }
    }

    return false;
}

static cJSON* GetDataAtSessionID(char* session_id, StringArray field_data, bool list_response, bool convert_types) {
    char query_string[1024] = "SELECT ";

    for (int i = 0; i < field_data.count; i++) {
        strcat(query_string, field_data.array[i]);
        if (i < field_data.count-1) {
            strcat(query_string, ", ");
        }
    }

    strcat(query_string, " FROM ");
    strcat(query_string, session_auth.auth_table_name);
    strcat(query_string, " WHERE SessionID='");
    strcat(query_string, session_id);
    strcat(query_string, "';");

    printf("Full query_string: `%s`\n", query_string);

    return HTTP_RunSQLQuery(query_string, list_response, convert_types);
}

cJSON* HTTP_Auth_GetUserDataAtSessionToken(char* session_id, bool remove_null_values, bool list_response, bool convert_types) {
    if (!HTTP_Auth_CheckSessionIDExists(session_id)) {
        printf("[ERROR] HTTP_Auth_GetUserDataAtSessionToken() No user exists with the Session ID: `%s`\n", session_id);
        return NULL;
    }

    cJSON* user_data_result = GetDataAtSessionID(session_id, session_auth.user_data, list_response, convert_types);
    if (user_data_result == NULL) {
        printf("[ERROR] HTTP_Auth_GetUserDataAtSessionToken() An error occured when running the SQL code to retrieve the user data associated with the Session ID.\n");
        return NULL;
    }

    if (remove_null_values && !HTTP_cJSON_IsObjectEmpty(user_data_result)) {
        if (list_response) {
            bool do_redo = false;
            cJSON* array = NULL;
            cJSON_ArrayForEach(array, user_data_result) {
                cJSON* object = NULL;
                cJSON_ArrayForEach(object, array) {
                    cJSON* elem = NULL;
                    cJSON_ArrayForEach(elem, object) {
                        list_obj_redo:
                        do_redo = false;
                        if (cJSON_IsNull(elem)) {
                            cJSON* original_elem = elem;
                            if (elem->next != NULL) {
                                elem = elem->next;
                                do_redo = true;
                            }
                            else {
                                elem = elem->prev;
                            }
                            cJSON_DeleteItemFromObject(object, original_elem->string);
                            if (object->child == NULL) {
                                break;
                            }
                            if (do_redo) {
                                goto list_obj_redo;
                            }
                        }
                    }
                }
            }
        }
        else {
            bool do_redo = false;
            cJSON* elem = NULL;
            cJSON_ArrayForEach(elem, user_data_result) {
                if (cJSON_IsNull(elem)) {
                    obj_redo:
                    do_redo = false;
                    if (cJSON_IsNull(elem)) {
                        cJSON* original_elem = elem;
                        if (elem->next != NULL) {
                            elem = elem->next;
                            do_redo = true;
                        }
                        else {
                            elem = elem->prev;
                        }
                        cJSON_DeleteItemFromObject(user_data_result, original_elem->string);
                        if (user_data_result->child == NULL) {
                            break;
                        }
                        if (do_redo) {
                            goto obj_redo;
                        }
                    }
                }
            }
        }
    }

    return user_data_result;
}

cJSON* HTTP_Auth_GetLoginDataAtSessionToken(char* session_id, bool list_response, bool convert_types) {
    if (!HTTP_Auth_CheckSessionIDExists(session_id)) {
        printf("[ERROR] HTTP_Auth_GetLoginDataAtSessionToken() No user exists with the Session ID: `%s`\n", session_id);
        return NULL;
    }

    cJSON* login_data_result = GetDataAtSessionID(session_id, session_auth.login_data, list_response, convert_types);
    if (login_data_result == NULL) {
        printf("[ERROR] HTTP_Auth_GetLoginDataAtSessionToken() An error occured when running the SQL code to retrieve the login data associated with the Session ID.\n");
        return NULL;
    }

    return login_data_result;
}

cJSON* HTTP_Auth_GetLoginAndUserDataAtSessionToken(char* session_id, bool remove_null_values, bool list_response, bool convert_types) {
    if (!HTTP_Auth_CheckSessionIDExists(session_id)) {
        printf("[ERROR] HTTP_Auth_GetLoginAndUserDataAtSessionToken() No user exists with the Session ID: `%s`\n", session_id);
        return NULL;
    }

    cJSON* login_data_result = HTTP_Auth_GetLoginDataAtSessionToken(session_id, list_response, convert_types);
    cJSON* user_data_result = HTTP_Auth_GetUserDataAtSessionToken(session_id, remove_null_values, list_response, convert_types);

    if (list_response) {
        cJSON* user_data_array = NULL;
        cJSON_ArrayForEach(user_data_array, user_data_result) {
            cJSON* user_data_object = NULL;
            cJSON_ArrayForEach(user_data_object, user_data_array) {
                if (!HTTP_cJSON_IsObjectEmpty(user_data_object)) {
                    cJSON_AddItemToArray(login_data_result->child,  cJSON_Duplicate(user_data_object, true));
                }
            }
        }
    }
    else {
        cJSONUtils_MergePatch(login_data_result, user_data_result);
    }

    return login_data_result;
}


static char* SetDataAtSessionID(Arena* arena, char* session_id, cJSON* new_data, StringArray fields, bool check_for_missing_fields) {
    char* calling_function_name = check_for_missing_fields ? "HTTP_Auth_SetLoginDataAtSessionToken()" : "HTTP_Auth_SetUserDataAtSessionToken()";
    char* data_type = check_for_missing_fields ? "login" : "user";

    if (!HTTP_Auth_CheckSessionIDExists(session_id)) {
        printf("[ERROR] %s No user exists with the Session ID: `%s`\n", calling_function_name, session_id);
        return NULL;
    }

    if (!cJSON_IsObject(new_data)) {
        printf("[ERROR] %s The %s data must just be a JSON object, the one you have provided is not.\n", calling_function_name, data_type);
        return NULL;
    } 

    if (!CheckDataFieldsAreCorrect(new_data, fields, calling_function_name, data_type, check_for_missing_fields)) {
        return NULL;
    }

    Temp scratch = GetScratch(&arena, 1);

    char* new_session_id = PushString(arena, HTTP_SESSION_ID_LENGTH);
    HTTP_Gen256ByteRandomNum(new_session_id, HTTP_SESSION_ID_LENGTH);

    char timeout_field_string[256] = {0};
    GenerateTimeoutField(timeout_field_string, 256);

    char query_string[2056] = {0};
    sprintf(query_string, "UPDATE %s SET SessionID='%s', __Session_ExpiryDate=%s", session_auth.auth_table_name, new_session_id, timeout_field_string);

    cJSON* elem = NULL;
    cJSON_ArrayForEach(elem, new_data) {
        for (int i = 0; i < fields.count; i++) {
            if (!strcmp(elem->string, fields.array[i])) {
                strcat(query_string, ", ");
                strcat(query_string, elem->string);
                strcat(query_string, "='");
                strcat(query_string, HTTP_cJSON_GetStringValue(scratch.arena, elem));
                strcat(query_string, "'");
            }
        }
    }

    strcat(query_string, " WHERE SessionID='");
    strcat(query_string, session_id);
    strcat(query_string, "';");

    printf("Final `%s` data query string: `%s`\n", data_type, query_string);

    if (HTTP_RunSQLQuery(query_string, false, false) == NULL) {
        printf("[ERROR] %s An error occured when running the SQL code to insert the the new %s data at the Session ID.\n", calling_function_name, data_type);
        DeleteScratch(scratch);
        return NULL;
    }

    DeleteScratch(scratch);
    return new_session_id;
}

char* HTTP_Auth_SetLoginDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_login_data) {
    return SetDataAtSessionID(arena, session_id, new_login_data, session_auth.login_data, true);
}

char* HTTP_Auth_SetUserDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_user_data) {
    return SetDataAtSessionID(arena, session_id, new_user_data, session_auth.user_data, false);
}

char* HTTP_Auth_SetLoginAndUserDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_data) {
    char* new_session_id;
    new_session_id = HTTP_Auth_SetLoginDataAtSessionToken(arena, session_id, new_data);

    if (new_session_id == NULL) {
        printf("[ERROR] HTTP_Auth_SetLoginAndUserDataAtSessionToken() An error occured when trying insert the new login data at the Session ID.\n");
        return NULL;
    }

    new_session_id = HTTP_Auth_SetUserDataAtSessionToken(arena, new_session_id, new_data);
    if (new_session_id == NULL) {
        printf("[ERROR] HTTP_Auth_SetLoginAndUserDataAtSessionToken() An error occured when trying insert the new user data at the Session ID.\n");
        return NULL;
    }

    return new_session_id;
}

char* HTTP_Auth_GetSessionIDAtLoginDetails(cJSON* login_data) {
    if (!cJSON_IsObject(login_data)) {
        printf("[ERROR] HTTP_Auth_GetSessionIDAtLoginDetails() The login data must just be a JSON object, the one you have provided is not.\n");
        return NULL;
    } 

    if (!CheckDataFieldsAreCorrect(login_data, session_auth.login_data, "HTTP_Auth_GetSessionIDAtLoginDetails()", "login", true)) {
        return NULL;
    }

    Temp scratch = GetScratch(0, 0);

    char query_string[2056] = {0};
    sprintf(query_string, "SELECT SessionID FROM %s WHERE ", session_auth.auth_table_name);

    cJSON* elem = NULL;
    cJSON_ArrayForEach(elem, login_data) {
        for (int i = 0; i < session_auth.login_data.count; i++) {
            if (!strcmp(elem->string, session_auth.login_data.array[i])) {
                strcat(query_string, elem->string);
                strcat(query_string, "='");
                strcat(query_string, HTTP_cJSON_GetStringValue(scratch.arena, elem));
                strcat(query_string, "'");
                if (i < session_auth.login_data.count-1) {
                    strcat(query_string, " AND ");
                }
            }
        }
    }

    printf("Get Session ID at login data query: `%s`\n", query_string);

    cJSON* session_id_query_result = HTTP_RunSQLQuery(query_string, false, false);
    if (session_id_query_result == NULL) {
        printf("[ERROR] HTTP_Auth_GetSessionIDAtLoginDetails() An error occured when running the SQL code to retreive the Session ID associated with the login data.\n");
        DeleteScratch(scratch);
        return NULL;
    }
    else if (HTTP_cJSON_IsObjectEmpty(session_id_query_result)) {
        printf("[INFO] HTTP_Auth_GetSessionIDAtLoginDetails() Could not find a Session ID associated with the login data provided.\n");
        DeleteScratch(scratch);
        return NULL;
    }

    DeleteScratch(scratch);
    return session_id_query_result->child->valuestring; 
}

bool HTTP_Auth_ExpireSessionID(char* session_id) {
    if (!HTTP_Auth_CheckSessionIDExists(session_id) ||
        HTTP_Auth_CheckSessionIDExpired(session_id)) {
        return false;
    }

    char query_string[2056] = {0};
    sprintf(query_string, "UPDATE %s SET __Session_ExpiryDate=datetime('now', '-200 years') WHERE SessionID='%s';", session_auth.auth_table_name, session_id);
    if (HTTP_RunSQLQuery(query_string, false, false) == NULL) {
        printf("[ERROR] HTTP_Auth_ExpireSessionID() An error occured when running the SQL code to expire the Session ID.\n");
        return false;
    }
    else {
        return true;
    }
}

StringArray HTTP_Auth_StringArray_GetPermissionsAtSessionID(Arena* arena, char* session_id) {
    if (!HTTP_Auth_CheckSessionIDExists(session_id)) {
        return (StringArray) {NULL, 0};
    }

    char query_string[2056] = {0};
    sprintf(query_string, "SELECT __Session_Permissions FROM %s WHERE SessionID='%s';", session_auth.auth_table_name, session_id);
    cJSON* permission_query_result = HTTP_RunSQLQuery(query_string, false, true);
    if (permission_query_result == NULL) {
        printf("[ERROR] HTTP_Auth_StringArray_GetPermissionsAtSessionID() An error occured when running the SQL code to get the permissions at the Session ID.\n");
        return (StringArray) {NULL, 0};
    }

    return StrSplitStringOnSeparator(arena, permission_query_result->child->valuestring, ",");
}

cJSON* HTTP_Auth_cJSON_GetPermissionsAtSessionID(char* session_id) {
    if (!HTTP_Auth_CheckSessionIDExists(session_id)) {
        return NULL;
    }

    char query_string[2056] = {0};
    sprintf(query_string, "SELECT __Session_Permissions FROM %s WHERE SessionID='%s';", session_auth.auth_table_name, session_id);
    cJSON* permission_query_result = HTTP_RunSQLQuery(query_string, false, true);
    if (permission_query_result == NULL) {
        printf("[ERROR] HTTP_Auth_cJSON_GetPermissionsAtSessionID() An error occured when running the SQL code to get the permissions at the Session ID.\n");
        return NULL;
    }

    Temp scratch = GetScratch(0, 0);
    StringArray permissions_string_array = StrSplitStringOnSeparator(scratch.arena, permission_query_result->child->valuestring, ",");
    cJSON* permissions_obj = cJSON_CreateObject();
    cJSON* permissions_array = cJSON_AddArrayToObject(permissions_obj, "permissions");
    for (int i = 0; i < permissions_string_array.count; i++) {
        cJSON_AddItemToArray(permissions_array, cJSON_CreateString(permissions_string_array.array[i]));
    }

    DeleteScratch(scratch);

    return permissions_obj;
}


bool HTTP_Auth_CheckPermissionAllowedAtSessionID(char* session_id, char* permission) {
    Temp scratch = GetScratch(0, 0);

    StringArray permissions_array = HTTP_Auth_StringArray_GetPermissionsAtSessionID(scratch.arena, session_id);

    for (int i = 0; i < permissions_array.count; i++) {
        if (!strcmp(permissions_array.array[i], permission)) {
            DeleteScratch(scratch);
            return true;
        }
    }

    DeleteScratch(scratch);
    return false;
}

bool HTTP_Auth_DeletePermissionAtSessionID(char* session_id, char* permission) {
    Temp scratch = GetScratch(0, 0);

    StringArray permissions_array = HTTP_Auth_StringArray_GetPermissionsAtSessionID(scratch.arena, session_id);
    if (!permissions_array.count) {
        DeleteScratch(scratch);
        return false;
    }

    StringArray removed_permissions_array = {
        .array = PushArray(scratch.arena, char*, 64)
    };

    bool found_matches = false;
    for (int i = 0; i < permissions_array.count; i++) {
        if (!strcmp(permissions_array.array[i], permission)) {
            found_matches = true;
        }
        else {
            removed_permissions_array.array[removed_permissions_array.count] = permissions_array.array[i];
            removed_permissions_array.count++;
        }
    }

    if (found_matches) {
        char* permission_string = ConvertStrArrayToString(scratch.arena, removed_permissions_array, ",");
        char  query_string[1024] = {0};
        sprintf(query_string, "UPDATE %s SET __Session_Permissions='%s' WHERE SessionID='%s';", session_auth.auth_table_name, permission_string, session_id);
        if (HTTP_RunSQLQuery(query_string, false, false) == NULL) {
            printf("[ERROR] HTTP_Auth_DeletePermissionAtSessionID() An error occured when running the SQL code to delete the permission at the Session ID.\n");
        }
    }

    DeleteScratch(scratch);
    return found_matches;
}

bool HTTP_Auth_AddPermissionAtSessionID(char* session_id, char* permission) {
    Temp scratch = GetScratch(0, 0);

    StringArray permissions_array = HTTP_Auth_StringArray_GetPermissionsAtSessionID(scratch.arena, session_id);
    if (!permissions_array.count) {
        DeleteScratch(scratch);
        return false;
    }

    permissions_array.array[permissions_array.count] = permission;
    permissions_array.count++;

    char* permission_string = ConvertStrArrayToString(scratch.arena, permissions_array, ",");
    char  query_string[1024] = {0};
    sprintf(query_string, "UPDATE %s SET __Session_Permissions='%s' WHERE SessionID='%s';", session_auth.auth_table_name, permission_string, session_id);

    if (HTTP_RunSQLQuery(query_string, false, false) == NULL) {
        printf("[ERROR] HTTP_Auth_AddPermissionAtSessionID() An error occured when running the SQL code to add the permission at the Session ID.\n");
        DeleteScratch(scratch);
        return false;
    }


    DeleteScratch(scratch);
    return true;
}

bool HTTP_Auth_SetPermissionsAtSessionID(char* session_id, StringArray permissions) {
    if (!HTTP_Auth_CheckSessionIDExists(session_id)) {
        return false;
    }

    Temp scratch = GetScratch(0, 0);

    char* permission_string = ConvertStrArrayToString(scratch.arena, permissions, ",");
    char  query_string[1024] = {0};
    sprintf(query_string, "UPDATE %s SET __Session_Permissions='%s' WHERE SessionID='%s';", session_auth.auth_table_name, permission_string, session_id);

    if (HTTP_RunSQLQuery(query_string, false, false) == NULL) {
        printf("[ERROR] HTTP_Auth_SetPermissionsAtSessionID() An error occured when running the SQL code to set the new permissions at the Session ID.\n");
        DeleteScratch(scratch);
        return false;
    }

    DeleteScratch(scratch);
    return true;
}

cJSON* HTTP_Auth_GetAllSessionID(bool remove_expired, bool list_response) {
    Temp scratch = GetScratch(0, 0);

    char query_string[1024] = {0};
    if (remove_expired) {
        sprintf(query_string, "SELECT SessionID FROM %s WHERE __Session_ExpiryDate > datetime('now');", session_auth.auth_table_name);
    }
    else {
        sprintf(query_string, "SELECT SessionID FROM %s;", session_auth.auth_table_name);
    }

    printf("Getting all session id query string: `%s`\n", query_string);
    cJSON* session_id_query_result = HTTP_RunSQLQuery(query_string, list_response, false);
    if (session_id_query_result == NULL) {
        printf("[ERROR] HTTP_Auth_GetAllSessionID() An error occured when running the SQL code to retreive all the Session ID's.\n");
        DeleteScratch(scratch);
        return NULL;
    }

    DeleteScratch(scratch);
    return session_id_query_result; 
}
