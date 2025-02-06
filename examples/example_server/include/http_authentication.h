#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

#include "map.h"
#define HTTP_SESSION_ID_LENGTH 258

enum SessionVariadicTypes {
    VARIADIC_END,
    VARIADIC_INT16,
    VARIADIC_UINT16,
    VARIADIC_INT32,
    VARIADIC_UINT32,
    VARIADIC_INT64,
    VARIADIC_UINT64,
    VARIADIC_CHAR,
    VARIADIC_LONG_DOUBLE,
    VARIADIC_DOUBLE,
    VARIADIC_STRING,
    VARIADIC_NULL,
    VARIADIC_UNKNOWN
};

global_variable char* timeout_field_names[] = { "years", "months", "days", "hours", "minutes", "seconds"};

typedef struct {
    int years;
    int months;
    int days;
    int hours;
    int minutes;
    int seconds;
} SessionMaxTimeout;

typedef struct {
    char* auth_table_name;
    StringArray login_data;
    StringArray user_data;
    SessionMaxTimeout timeout;
    bool  initialized;
} SessionAuthInfo;

// TODO: Maybe we don't need this?
typedef struct {
    char* session_id;
    StringArray* login_data;
    StringArray* user_data;
} SessionAuthUser;

global_variable SessionAuthInfo session_auth;

HTTPEXPORTFUNC void   HTTP_Auth_SessionEnable(char* auth_table_name, StringArray login_data, StringArray login_data_types, StringArray user_data, StringArray user_data_types, SessionMaxTimeout* timeout);
HTTPEXPORTFUNC bool   HTTP_Auth_SessionCheckIsEnabled(void);
HTTPEXPORTFUNC char*  __HTTP_Auth_AddUserIfNotExists(Arena* arena, ...);
HTTPEXPORTFUNC char*  HTTP_Auth_cJSON_AddUserIfNotExists(Arena* arena, cJSON* data);
HTTPEXPORTFUNC char*  HTTP_Auth_GenerateNewSessionTokenIfExpired(Arena* arena, char* check_session_id);
HTTPEXPORTFUNC void   HTTP_Auth_DeleteUserAtSessionToken(char* session_id);
HTTPEXPORTFUNC bool   HTTP_Auth_CheckSessionIDExpired(char* session_id);
HTTPEXPORTFUNC bool   HTTP_Auth_CheckSessionIDExists(char* session_id);
HTTPEXPORTFUNC bool   HTTP_Auth_RefreshTokenExpiryDate(char* check_session_id);
HTTPEXPORTFUNC cJSON* HTTP_Auth_GetUserDataAtSessionToken(char* session_id, bool remove_null_values, bool list_response, bool convert_types);
HTTPEXPORTFUNC cJSON* HTTP_Auth_GetLoginDataAtSessionToken(char* session_id, bool list_response, bool convert_types);
HTTPEXPORTFUNC cJSON* HTTP_Auth_GetLoginAndUserDataAtSessionToken(char* session_id, bool remove_null_values, bool list_response, bool convert_types);
HTTPEXPORTFUNC char*  HTTP_Auth_GetSessionIDAtLoginDetails(cJSON* login_data);
HTTPEXPORTFUNC char*  HTTP_Auth_SetLoginDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_login_data);
HTTPEXPORTFUNC char*  HTTP_Auth_SetUserDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_user_data);
HTTPEXPORTFUNC char*  HTTP_Auth_SetLoginAndUserDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_data);
HTTPEXPORTFUNC bool   HTTP_Auth_ExpireSessionID(char* session_id);
HTTPEXPORTFUNC StringArray HTTP_Auth_StringArray_GetPermissionsAtSessionID(Arena* arena, char* session_id);
HTTPEXPORTFUNC cJSON* HTTP_Auth_cJSON_GetPermissionsAtSessionID(char* session_id);
HTTPEXPORTFUNC bool   HTTP_Auth_CheckPermissionAllowedAtSessionID(char* session_id, char* permission);
HTTPEXPORTFUNC bool   HTTP_Auth_DeletePermissionAtSessionID(char* session_id, char* permission);
HTTPEXPORTFUNC bool   HTTP_Auth_AddPermissionAtSessionID(char* session_id, char* permission);
HTTPEXPORTFUNC cJSON* HTTP_Auth_GetAllSessionID(bool remove_expired, bool list_response);

HTTPEXPORTFUNC char*  HTTP_Auth_CookiesDict_GenerateNewSessionTokenIfExpired(Arena* arena, CookiesDict cookies);
HTTPEXPORTFUNC void   HTTP_Auth_CookiesDict_DeleteUserAtSessionToken(CookiesDict cookies);
HTTPEXPORTFUNC bool   HTTP_Auth_CookiesDict_RefreshTokenExpiryDate(CookiesDict cookies);

#define GetTypeVariadicArgs(value) _Generic((value),  \
        char*                 : VARIADIC_STRING,      \
        short                 : VARIADIC_INT16,       \
        unsigned short        : VARIADIC_UINT16,      \
        int                   : VARIADIC_INT32,       \
        unsigned int          : VARIADIC_UINT32,      \
        long int              : VARIADIC_INT32,       \
        unsigned long int     : VARIADIC_UINT32,      \
        long long int         : VARIADIC_INT64,       \
        unsigned long long int: VARIADIC_UINT64,      \
        char                  : VARIADIC_CHAR,        \
        float                 : VARIADIC_DOUBLE,      \
        double                : VARIADIC_DOUBLE,      \
        long double           : VARIADIC_LONG_DOUBLE, \
        void*                 : VARIADIC_NULL,        \
        default               : VARIADIC_UNKNOWN      \
        ), (value)

#define HTTP_Auth_AddUserIfNotExists(...) __HTTP_Auth_AddUserIfNotExists(HTTP_Thread_GetContext().recycle_arena, MAP_LIST(GetTypeVariadicArgs, ##__VA_ARGS__), VARIADIC_END)

#ifdef __cplusplus
}
#endif
