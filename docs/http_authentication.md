# http\_authentication.h



----------

## `HTTP_Auth_SessionEnable`

### Description
Enables Session Authentication, this function creates a table in the connected database (make sure you connect to a database using `HTTP_CreateDatabase` before running this function) with the login fields and user fields provided.

 - TIP: It may be useful to use the `StrArrayLit` macro to intialize arrays for parameters which have type `StringArray` (see code examples to understand how this works).

### Parameters
- `auth_table_name` *(char\*)* -  Name of the table to create/store the login and user data.

- `login_data` *(StringArray)* -  Name of the fields to store the login data.

- `login_data_types` *(StringArray)* -  Name of the types of each of the fields to store the login data.

- `user_data` *(StringArray)* -  Name of the fields to store the user's data.

- `user_data_types` *(StringArray)* -  Name of the types of each of the fields to store the user's data.

- `timeout` *(SessionMaxTimeout\*)* -  The amount of time a Session Token is valid for, if NULL then the timeout is set to 30 days.

### Returns
- `void` - None

----------

## `HTTP_Auth_SessionCheckIsEnabled`

### Description
Checks whether Session Authentication is enabled.

### Returns
- `bool` - A boolean telling us whether Session Authentication is enabled.

----------

## `__HTTP_Auth_AddUserIfNotExists`

### Description
WARNING: Don't call this function directly refer the macro `HTTP_Auth_AddUserIfNotExists` instead.

### Returns
- `char*` - None

----------

## `HTTP_Auth_cJSON_AddUserIfNotExists`

### Description
Adds a user into authentication database the with the information provided as a cJSON object, which contains valid values for all of the login fields (the user fields are optional).

### Parameters
- `arena` *(Arena\*)* -  An arena to store any data required (use the arena passed into the route callback function).

- `data` *(cJSON\*)* -  A cJSON object containing all of the login fields (as strings) with valid values, the user fields can be optionally added.

### Returns
- `char*` - Returns the Session ID of the user if successful, otherwise NULL.

----------

## `HTTP_Auth_GenerateNewSessionTokenIfExpired`

### Description
Checks if a user with the Session ID specified exists in the authentication database. If it does and the token is expired it will generate a new token and update it in the database, otherwise the token will be left as is.

### Parameters
- `arena` *(Arena\*)* -  An arena to store any data required (use the arena passed into the route callback function).

- `check_session_id` *(char\*)* -  Session ID to check is expired.

### Returns
- `char*` - Returns the new Session ID of the user if successful, otherwise NULL.

----------

## `HTTP_Auth_DeleteUserAtSessionToken`

### Description
Check if the user with the Session ID specified exists in the authentication database. If it does then the user is deleted from the database.

### Parameters
- `session_id` *(char\*)* -  Session ID of the user we are trying to delete.

### Returns
- `void` - None

----------

## `HTTP_Auth_CheckSessionIDExpired`

### Description
If a user with the Session ID specified exists in the authentication database, we will check whether their token is expired.

### Parameters
- `session_id` *(char\*)* -  Session ID to check is expired.

### Returns
- `bool` - Returns a boolean telling us whether the Session ID is expired, if the user doesn't exist then it will return false.

----------

## `HTTP_Auth_CheckSessionIDExists`

### Description
Checks if the user with the Session ID specified exists in the authentication database.

### Parameters
- `session_id` *(char\*)* -  Session ID to check exists.

### Returns
- `bool` - Returns a boolean telling us whether the Session ID is exists.

----------

## `HTTP_Auth_RefreshTokenExpiryDate`

### Description
Checks if a user with the Session ID specified exists in the authentication database. If they do then we will refresh the expiry date of the their token without changing their Session ID.

### Parameters
- `session_id` *(char\*)* -  Session ID to refresh expiry date.

### Returns
- `bool` - Returns a boolean telling us whether we were successful in refreshing the expiry date of the user at the Session ID specified.

----------

## `HTTP_Auth_GetUserDataAtSessionToken`

### Description
Retrieves the user data associated with a Session ID.

### Parameters
- `session_id` *(char\*)* -  The Session ID to fetch the user data for.

- `remove_null_values` *(bool)* -  Flag to specify whether key-value pairs with null values should be removed from the response.

- `list_response` *(bool)* -  Flag to indicate whether key-value pairs should be stored in a list.

- `convert_types` *(bool)* -  Flag to specify whether values should be converted to their respective types, if false all keys and values will be stored as strings.

### Returns
- `cJSON*` - Returns a cJSON object containing the user data associated with the Session ID. If no data is found or an error occurs, it returns NULL.

----------

## `HTTP_Auth_GetLoginDataAtSessionToken`

### Description
Retrieves the login data associated with a Session ID.

### Parameters
- `session_id` *(char\*)* -  The Session ID to fetch the login data for.

- `list_response` *(bool)* -  Flag to indicate whether key-value pairs should be stored in a list.

- `convert_types` *(bool)* -  Flag to specify whether values should be converted to their respective types, if false all keys and values will be stored as strings.

### Returns
- `cJSON*` - Returns a cJSON object containing the login data associated with the Session ID. If no data is found or an error occurs, it returns NULL.

----------

## `HTTP_Auth_GetLoginAndUserDataAtSessionToken`

### Description
Retrieves both login and user data associated with a Session ID.

### Parameters
- `session_id` *(char\*)* -  The Session ID to fetch the login and user data for.

- `remove_null_values` *(bool)* -  Flag to specify whether key-value pairs with null values should be removed from the response.

- `list_response` *(bool)* -  Flag to indicate whether key-value pairs should be stored in a list.

- `convert_types` *(bool)* -  Flag to specify whether values should be converted to their respective types, if false all keys and values will be stored as strings.

### Returns
- `cJSON*` - Returns a cJSON object containing both login and user data for the Session ID. If no data is found or an error occurs, it returns NULL.

----------

## `HTTP_Auth_GetSessionIDAtLoginDetails`

### Description
Retrieves the Session ID associated with the provided login details.

### Parameters
- `login_data` *(cJSON\*)* -  A cJSON object containing the login data (fields and their corresponding values).

### Returns
- `char*` - Returns the Session ID if the login details match an entry in the database, or NULL if no matching Session ID is found or an error occurs.

----------

## `HTTP_Auth_SetLoginDataAtSessionToken`

### Description
Updates the login data for a given Session ID. This function checks if the Session ID exists and if the provided login data is valid (all the login fields must be defined and have valid corresponding values) before updating it.

### Parameters
- `arena` *(Arena\*)* -  The arena to store any temporary data required during processing.

- `session_id` *(char\*)* -  The Session ID for which the login data needs to be updated.

- `new_login_data` *(cJSON\*)* -  The new login data in the form of a cJSON object (all login fields must be defined with valid values).

### Returns
- `char*` - Returns the new Session ID if successful, otherwise NULL if there was an error.

----------

## `HTTP_Auth_SetUserDataAtSessionToken`

### Description
Updates the user data for a given Session ID. This function checks if the Session ID exists and if the provided user data is valid before updating it.

### Parameters
- `arena` *(Arena\*)* -  The arena to store any temporary data required during processing.

- `session_id` *(char\*)* -  The Session ID for which the user data needs to be updated.

- `new_user_data` *(cJSON\*)* -  The new user data in the form of a cJSON object.

### Returns
- `char*` - Returns the new Session ID if successful, otherwise NULL if there was an error.

----------

## `HTTP_Auth_SetLoginAndUserDataAtSessionToken`

### Description
Updates both login and user data for a given Session ID. This function updates both sets of data (login and user) for the specified Session ID. All login fields must be defined and have valid values, user fields can be optionally filled out. If either update fails, the function will return NULL.

### Parameters
- `arena` *(Arena\*)* -  The arena to store any temporary data required during processing.

- `session_id` *(char\*)* -  The Session ID for which both login and user data needs to be updated.

- `new_data` *(cJSON\*)* -  A cJSON object containing both the new login and user data (all login fields must be defined with valid values).

### Returns
- `char*` - Returns the new Session ID if successful, otherwise NULL if there was an error.

----------

## `HTTP_Auth_ExpireSessionID`

### Description
Expires a Session ID by updating its expiry date in the database. This function checks if the Session ID exists and is not already expired before attempting to expire it.

### Parameters
- `session_id` *(char\*)* -  The Session ID to expire.

### Returns
- `bool` - Returns true if the Session ID was successfully expired, otherwise false if there was an error or if the Session ID was already expired.

----------

## `HTTP_Auth_StringArray_GetPermissionsAtSessionID`

### Description
Retrieves the permissions associated with a given Session ID. This function queries the database to retrieve the list of permissions for the specified Session ID and returns them as a StringArray.

### Parameters
- `arena` *(Arena\*)* -  The arena to store temporary data for string splitting.

- `session_id` *(char\*)* -  The Session ID to retrieve permissions for.

### Returns
- `StringArray` - Returns a StringArray containing the permissions associated with the Session ID. If no permissions are found or an error occurs, it returns a StringArray with the array member set to NULL and a count of 0.

----------

## `HTTP_Auth_cJSON_GetPermissionsAtSessionID`

### Description
Retrieves the permissions associated with a given Session ID. This function queries the database to retrieve the permissions for the specified Session ID and returns them as a cJSON object.

### Parameters
- `session_id` *(char\*)* -  The Session ID to retrieve permissions for.

### Returns
- `cJSON*` - Returns a cJSON object containing the permissions for the Session ID, or NULL if an error occurs or no permissions are found.

----------

## `HTTP_Auth_CheckPermissionAllowedAtSessionID`

### Description
Checks whether a specific permission is allowed for a given Session ID. This function checks the permissions associated with the Session ID and verifies if the specified permission is included.

### Parameters
- `session_id` *(char\*)* -  The Session ID to check permissions for.

- `permission` *(char\*)* -  The permission to check for.

### Returns
- `bool` - Returns true if the permission is allowed, otherwise false.

----------

## `HTTP_Auth_DeletePermissionAtSessionID`

### Description
Deletes a specific permission for the given Session ID. This function removes the specified permission from the Session ID's permission list in the database.

### Parameters
- `session_id` *(char\*)* -  The Session ID from which to delete the permission.

- `permission` *(char\*)* -  The permission to delete.

### Returns
- `bool` - Returns true if the permission was found and successfully deleted, otherwise false.

----------

## `HTTP_Auth_AddPermissionAtSessionID`

### Description
Adds a specific permission for the given Session ID. This function adds the specified permission to the Session ID's permission list in the database.

### Parameters
- `session_id` *(char\*)* -  The Session ID to which the permission will be added.

- `permission` *(char\*)* -  The permission to add.

### Returns
- `bool` - Returns true if the permission was successfully added, otherwise false.

----------

## `HTTP_Auth_SetPermissionsAtSessionID`

### Description
Sets the permissions for the given Session ID. This function updates the Session ID's permission list in the database with the new list of permissions provided.

### Parameters
- `session_id` *(char\*)* -  The Session ID to set the permissions for.

- `permissions` *(StringArray)* -  The new list of permissions as a StringArray.

### Returns
- `bool` - Returns true if the permissions were successfully updated, otherwise false.

----------

## `HTTP_Auth_GetAllSessionID`

### Description
Retrieves all Session IDs from the authentication database. This function can optionally filter out expired sessions based on the `remove_expired` flag.

### Parameters
- `remove_expired` *(bool)* -  Flag to specify whether to exclude expired Session IDs from the results.

- `list_response` *(bool)* -  Flag to indicate whether key-value pairs should be stored in a list.

### Returns
- `cJSON*` - Returns a cJSON object containing the Session IDs, or NULL if an error occurs.

----------

## `HTTP_Auth_CookiesDict_GenerateNewSessionTokenIfExpired`

### Description
This function looks for the "SessionID" in the provided cookies and, if found, checks if the token is expired in the authentication database. If the token is expired, a new Session ID is generated and returned.

### Parameters
- `arena` *(Arena\*)* -  The arena to store any temporary data required during the process.

- `cookies` *(CookiesDict)* -  The cookies which should include "SessionID".

### Returns
- `char*` - Returns the new Session ID if the existing one was valid and expired and a new one was generated, otherwise returns NULL.

----------

## `HTTP_Auth_CookiesDict_DeleteUserAtSessionToken`

### Description
Deletes the user associated with the Session ID stored in the provided cookies. This function looks for the "SessionID" in the cookies and deletes the corresponding user from the authentication database.

### Parameters
- `cookies` *(CookiesDict)* -  The cookies which should include "SessionID".

### Returns
- `void` - None

----------

## `HTTP_Auth_CookiesDict_RefreshTokenExpiryDate`

### Description
This function looks for the "SessionID" in the provided cookies and, if found, checks if the token is expired in the authentication database. If the token is expired, then it will refresh it's expiry date.

### Parameters
- `cookies` *(CookiesDict)* -  The cookies which should include "SessionID".

### Returns
- `bool` - Returns true if the expiry date was successfully refreshed, otherwise false.

----------

## Macro: `HTTP_Auth_AddUserIfNotExists`

### Description
Adds a new user into the database if a user with the login details specified doesn't already exist.

### Parameters
- `permissions` *(StringArray)* -  A StringArray of permissions for the new user.

- `rest_of_details` -  The arguments provided will first map onto the login fields and then the user fields there must be at least enough arguments to define all of the login fields, and the user fields can be optionally defined (see code examples to understand how this works).

### Returns
- `char*` - Returns the Session ID of the user if successful, otherwise NULL.
