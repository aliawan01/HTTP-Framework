#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

/*
   @desc Wraps a given cJSON object inside an array and associates it with a specified key.
         This function creates a new cJSON object, adds an array to it with the key being 
         the given `array_name`, and places the provided object inside of that array.
   @param array_name The name of the array to create in the cJSON object.
   @param obj The cJSON object to be placed inside the array.
   @return Returns a new cJSON object containing the array with the specified object inside, and the key being
           the string contained in `array_name`.
*/
HTTPEXPORTFUNC cJSON* HTTP_cJSON_TurnObjectIntoArray(char* array_name, cJSON* obj);

/*
   @desc Checks whether a given cJSON object is empty.
         A cJSON object is considered empty if it is NULL, has no children,
         or only contains the key "root" with no value.
   @param object The cJSON object to check.
   @return Returns true if the object is empty, otherwise false.
*/
HTTPEXPORTFUNC bool HTTP_cJSON_IsObjectEmpty(cJSON* object);

/*
   @desc Converts a string representation of data into an appropriate cJSON type.
         The function detects numbers, booleans ("true"/"false"), and "null" values.
         If none of these match, it treats the input as a string.
   @param data The string to convert into a cJSON type.
   @return Returns a cJSON object representing the detected type.
*/
HTTPEXPORTFUNC cJSON* HTTP_cJSON_TurnStringToType(char* data);

/*
   @desc Retrieves the string value of a given cJSON key-value pair.
         If the value is a string, then the value is returned directly.
         If it is a integer or a float, it is converted to a string.
   @param arena The Arena used for temporary string storage.
   @param elem The cJSON key-value pair to extract the string value from.
   @return Returns a string representation of the value in the key-value pair.
*/
HTTPEXPORTFUNC char* HTTP_cJSON_GetStringValue(Arena* arena, cJSON* elem);

/*
   @desc Adds an array of permissions to a given cJSON object under the key "permissions".
   @param object The cJSON object to which the permissions array will be added.
   @param permissions The StringArray containing the permission strings.
*/
HTTPEXPORTFUNC void HTTP_cJSON_AddPermissionArrayToObject(cJSON* object, StringArray permissions);

#ifdef __cplusplus
}
#endif
