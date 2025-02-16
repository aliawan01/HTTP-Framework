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

/*
   @desc Enables Session Authentication, this function creates a table in 
         the connected database (make sure you connect to a database using 
         `HTTP_CreateDatabase` before running this function) with the login 
         fields and user fields provided.\n\n
         - TIP: It may be useful to use the `StrArrayLit` macro to intialize 
                arrays for parameters which have type `StringArray` (see code examples
                to understand how this works).

    @param auth_table_name  Name of the table to create/store the login and user data.
    @param login_data       Name of the fields to store the login data.
    @param login_data_types Name of the types of each of the fields to store the login data.
    @param user_data        Name of the fields to store the user's data.
    @param user_data_types  Name of the types of each of the fields to store the user's data.
    @param timeout          The amount of time a Session Token is valid for, if NULL then 
                            the timeout is set to 30 days.
*/
HTTPEXPORTFUNC void   HTTP_Auth_SessionEnable(char* auth_table_name, StringArray login_data, StringArray login_data_types, StringArray user_data, StringArray user_data_types, SessionMaxTimeout* timeout);
/*
   @desc Checks whether Session Authentication is enabled.
   @return A boolean telling us whether Session Authentication is enabled.
*/
HTTPEXPORTFUNC bool   HTTP_Auth_SessionCheckIsEnabled(void);
/*
   @desc WARNING: Don't call this function directly refer the macro `HTTP_Auth_AddUserIfNotExists` instead.
*/
HTTPEXPORTFUNC char*  __HTTP_Auth_AddUserIfNotExists(Arena* arena, ...);
/*
   @desc Adds a user into authentication database the with the information provided as a cJSON object,
         which contains valid values for all of the login fields (the user fields are optional).
   @param arena An arena to store any data required (use the arena passed into the route callback function).
   @param data  A cJSON object containing all of the login fields (as strings) with valid values, the user fields can be optionally added.
   @return Returns the Session ID of the user if successful, otherwise NULL.
*/
HTTPEXPORTFUNC char*  HTTP_Auth_cJSON_AddUserIfNotExists(Arena* arena, cJSON* data);
/*
   @desc Checks if a user with the Session ID specified exists in the authentication database. 
         If it does and the token is expired it will generate a new token and update it in the database,
         otherwise the token will be left as is.
   @param arena An arena to store any data required (use the arena passed into the route callback function).
   @param check_session_id Session ID to check is expired.
   @return Returns the new Session ID of the user if successful, otherwise NULL.
*/
HTTPEXPORTFUNC char*  HTTP_Auth_GenerateNewSessionTokenIfExpired(Arena* arena, char* check_session_id);
/*
   @desc Check if the user with the Session ID specified exists in the authentication database. 
         If it does then the user is deleted from the database.
   @param session_id Session ID of the user we are trying to delete.
*/
HTTPEXPORTFUNC void   HTTP_Auth_DeleteUserAtSessionToken(char* session_id);
/*
   @desc If a user with the Session ID specified exists in the authentication database, we will
         check whether their token is expired.
   @param session_id Session ID to check is expired.
   @return Returns a boolean telling us whether the Session ID is expired, if the user doesn't
           exist then it will return false.
*/
HTTPEXPORTFUNC bool   HTTP_Auth_CheckSessionIDExpired(char* session_id);
/*
   @desc Checks if the user with the Session ID specified exists in the authentication database.
   @param session_id Session ID to check exists.
   @return Returns a boolean telling us whether the Session ID is exists.
*/
HTTPEXPORTFUNC bool   HTTP_Auth_CheckSessionIDExists(char* session_id);
/*
   @desc Checks if a user with the Session ID specified exists in the authentication database.
         If they do then we will refresh the expiry date of the their token without changing
         their Session ID.
   @param session_id Session ID to refresh expiry date.
   @return Returns a boolean telling us whether we were successful in refreshing the expiry date of the user at the Session ID specified.
*/
HTTPEXPORTFUNC bool   HTTP_Auth_RefreshTokenExpiryDate(char* check_session_id);
/*
   @desc Retrieves the user data associated with a Session ID.
   @param session_id The Session ID to fetch the user data for.
   @param remove_null_values Flag to specify whether key-value pairs with null values should be removed from the response.
   @param list_response Flag to indicate whether key-value pairs should be stored in a list.
   @param convert_types Flag to specify whether values should be converted to their respective types,
                        if false all keys and values will be stored as strings.
   @return Returns a cJSON object containing the user data associated with the Session ID.
           If no data is found or an error occurs, it returns NULL.
*/
HTTPEXPORTFUNC cJSON* HTTP_Auth_GetUserDataAtSessionToken(char* session_id, bool remove_null_values, bool list_response, bool convert_types);

/*
   @desc Retrieves the login data associated with a Session ID.
   @param session_id The Session ID to fetch the login data for.
   @param list_response Flag to indicate whether key-value pairs should be stored in a list.
   @param convert_types Flag to specify whether values should be converted to their respective types,
                        if false all keys and values will be stored as strings.
   @return Returns a cJSON object containing the login data associated with the Session ID.
           If no data is found or an error occurs, it returns NULL.
*/
HTTPEXPORTFUNC cJSON* HTTP_Auth_GetLoginDataAtSessionToken(char* session_id, bool list_response, bool convert_types);

/*
   @desc Retrieves both login and user data associated with a Session ID.
   @param session_id The Session ID to fetch the login and user data for.
   @param remove_null_values Flag to specify whether key-value pairs with null values should be removed from the response.
   @param list_response Flag to indicate whether key-value pairs should be stored in a list.
   @param convert_types Flag to specify whether values should be converted to their respective types,
                        if false all keys and values will be stored as strings.
   @return Returns a cJSON object containing both login and user data for the Session ID.
           If no data is found or an error occurs, it returns NULL.
*/
HTTPEXPORTFUNC cJSON* HTTP_Auth_GetLoginAndUserDataAtSessionToken(char* session_id, bool remove_null_values, bool list_response, bool convert_types);

/*
   @desc Retrieves the Session ID associated with the provided login details.
   @param login_data A cJSON object containing the login data (fields and their corresponding values).
   @return Returns the Session ID if the login details match an entry in the database, 
           or NULL if no matching Session ID is found or an error occurs.
*/
HTTPEXPORTFUNC char*  HTTP_Auth_GetSessionIDAtLoginDetails(cJSON* login_data);

/*
   @desc Updates the login data for a given Session ID.
         This function checks if the Session ID exists and if the provided login data is valid
         (all the login fields must be defined and have valid corresponding values) before updating it.
   @param arena The arena to store any temporary data required during processing.
   @param session_id The Session ID for which the login data needs to be updated.
   @param new_login_data The new login data in the form of a cJSON object (all login fields must be defined with valid values).
   @return Returns the new Session ID if successful, otherwise NULL if there was an error.
*/
HTTPEXPORTFUNC char*  HTTP_Auth_SetLoginDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_login_data);

/*
   @desc Updates the user data for a given Session ID.
         This function checks if the Session ID exists and if the provided user data
         is valid before updating it.
   @param arena The arena to store any temporary data required during processing.
   @param session_id The Session ID for which the user data needs to be updated.
   @param new_user_data The new user data in the form of a cJSON object.
   @return Returns the new Session ID if successful, otherwise NULL if there was an error.
*/
HTTPEXPORTFUNC char*  HTTP_Auth_SetUserDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_user_data);

/*
   @desc Updates both login and user data for a given Session ID.
         This function updates both sets of data (login and user) for the specified Session ID.
         All login fields must be defined and have valid values, user fields can be optionally filled out.
         If either update fails, the function will return NULL.
   @param arena The arena to store any temporary data required during processing.
   @param session_id The Session ID for which both login and user data needs to be updated.
   @param new_data A cJSON object containing both the new login and user data (all login fields must be defined with 
                   valid values).
   @return Returns the new Session ID if successful, otherwise NULL if there was an error.
*/
HTTPEXPORTFUNC char*  HTTP_Auth_SetLoginAndUserDataAtSessionToken(Arena* arena, char* session_id, cJSON* new_data);

/*
   @desc Expires a Session ID by updating its expiry date in the database.
         This function checks if the Session ID exists and is not already expired before attempting to expire it.
   @param session_id The Session ID to expire.
   @return Returns true if the Session ID was successfully expired, otherwise false if there was an error or if the Session ID was already expired.
*/
HTTPEXPORTFUNC bool HTTP_Auth_ExpireSessionID(char* session_id);

/*
   @desc Retrieves the permissions associated with a given Session ID.
         This function queries the database to retrieve the list of permissions for
         the specified Session ID and returns them as a StringArray.
   @param arena The arena to store temporary data for string splitting.
   @param session_id The Session ID to retrieve permissions for.
   @return Returns a StringArray containing the permissions associated with the Session ID.
           If no permissions are found or an error occurs, it returns a StringArray with the
           array member set to NULL and a count of 0.
*/
HTTPEXPORTFUNC StringArray HTTP_Auth_StringArray_GetPermissionsAtSessionID(Arena* arena, char* session_id);

/*
   @desc Retrieves the permissions associated with a given Session ID.
         This function queries the database to retrieve the permissions for the specified Session ID
         and returns them as a cJSON object.
   @param session_id The Session ID to retrieve permissions for.
   @return Returns a cJSON object containing the permissions for the Session ID, or NULL if an error occurs or no permissions are found.
*/
HTTPEXPORTFUNC cJSON* HTTP_Auth_cJSON_GetPermissionsAtSessionID(char* session_id);

/*
   @desc Checks whether a specific permission is allowed for a given Session ID.
         This function checks the permissions associated with the Session ID and verifies if the specified permission is included.
   @param session_id The Session ID to check permissions for.
   @param permission The permission to check for.
   @return Returns true if the permission is allowed, otherwise false.
*/
HTTPEXPORTFUNC bool HTTP_Auth_CheckPermissionAllowedAtSessionID(char* session_id, char* permission);

/*
   @desc Deletes a specific permission for the given Session ID.
         This function removes the specified permission from the Session ID's permission list in the database.
   @param session_id The Session ID from which to delete the permission.
   @param permission The permission to delete.
   @return Returns true if the permission was found and successfully deleted, otherwise false.
*/
HTTPEXPORTFUNC bool HTTP_Auth_DeletePermissionAtSessionID(char* session_id, char* permission);

/*
   @desc Adds a specific permission for the given Session ID.
         This function adds the specified permission to the Session ID's permission list in the database.
   @param session_id The Session ID to which the permission will be added.
   @param permission The permission to add.
   @return Returns true if the permission was successfully added, otherwise false.
*/
HTTPEXPORTFUNC bool HTTP_Auth_AddPermissionAtSessionID(char* session_id, char* permission);

/*
   @desc Sets the permissions for the given Session ID.
         This function updates the Session ID's permission list in the database with the new list of permissions provided.
   @param session_id The Session ID to set the permissions for.
   @param permissions The new list of permissions as a StringArray.
   @return Returns true if the permissions were successfully updated, otherwise false.
*/
HTTPEXPORTFUNC bool HTTP_Auth_SetPermissionsAtSessionID(char* session_id, StringArray permissions);

/*
   @desc Retrieves all Session IDs from the authentication database.
         This function can optionally filter out expired sessions based on the `remove_expired` flag.
   @param remove_expired Flag to specify whether to exclude expired Session IDs from the results.
   @param list_response Flag to indicate whether key-value pairs should be stored in a list.
   @return Returns a cJSON object containing the Session IDs, or NULL if an error occurs.
*/
HTTPEXPORTFUNC cJSON* HTTP_Auth_GetAllSessionID(bool remove_expired, bool list_response);

/*
   @desc This function looks for the "SessionID" in the provided cookies and, if found, checks if the token is expired
         in the authentication database. If the token is expired, a new Session ID is generated and returned.
   @param arena The arena to store any temporary data required during the process.
   @param cookies The cookies which should include "SessionID".
   @return Returns the new Session ID if the existing one was valid and expired and a new one was generated, otherwise returns NULL.
*/
HTTPEXPORTFUNC char* HTTP_Auth_CookiesDict_GenerateNewSessionTokenIfExpired(Arena* arena, CookiesDict cookies);

/*
   @desc Deletes the user associated with the Session ID stored in the provided cookies.
         This function looks for the "SessionID" in the cookies and deletes the corresponding user from the authentication database.
   @param cookies The cookies which should include "SessionID".
*/
HTTPEXPORTFUNC void HTTP_Auth_CookiesDict_DeleteUserAtSessionToken(CookiesDict cookies);

/*
   @desc This function looks for the "SessionID" in the provided cookies and, if found, checks if the token is expired
         in the authentication database. If the token is expired, then it will refresh it's expiry date.
   @param cookies The cookies which should include "SessionID".
   @return Returns true if the expiry date was successfully refreshed, otherwise false.
*/
HTTPEXPORTFUNC bool HTTP_Auth_CookiesDict_RefreshTokenExpiryDate(CookiesDict cookies);

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

/*
   @description Adds a new user into the database if a user with the login details specified doesn't already exist.
   @parameter permissions A StringArray of permissions for the new user
   @parameter rest_of_details The arguments provided will first map onto the login fields and then the user fields
                          there must be at least enough arguments to define all of the login fields,and the user
                          fields can be optionally defined (see code examples to understand how this works).
   @returns Returns the Session ID of the user if successful, otherwise NULL.
*/
#define HTTP_Auth_AddUserIfNotExists(...) __HTTP_Auth_AddUserIfNotExists(HTTP_Thread_GetContext().recycle_arena, MAP_LIST(GetTypeVariadicArgs, ##__VA_ARGS__), VARIADIC_END)

#ifdef __cplusplus
}
#endif
