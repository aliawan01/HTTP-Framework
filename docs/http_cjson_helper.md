# http\_cjson\_helper.h



----------

## `HTTP_cJSON_TurnObjectIntoArray`

### Description
Wraps a given cJSON object inside an array and associates it with a specified key. This function creates a new cJSON object, adds an array to it with the key being the given `array_name`, and places the provided object inside of that array.

### Parameters
- `array_name` *(char\*)* -  array_name The name of the array to create in the cJSON object.

- `obj` *(cJSON\*)* -  obj The cJSON object to be placed inside the array.

### Returns
- `cJSON*` - Returns a new cJSON object containing the array with the specified object inside, and the key being the string contained in `array_name`.

----------

## `HTTP_cJSON_IsObjectEmpty`

### Description
Checks whether a given cJSON object is empty. A cJSON object is considered empty if it is NULL, has no children, or only contains the key "root" with no value.

### Parameters
- `object` *(cJSON\*)* -  object The cJSON object to check.

### Returns
- `bool` - Returns true if the object is empty, otherwise false.

----------

## `HTTP_cJSON_TurnStringToType`

### Description
Converts a string representation of data into an appropriate cJSON type. The function detects numbers, booleans ("true"/"false"), and "null" values. If none of these match, it treats the input as a string.

### Parameters
- `data` *(char\*)* -  data The string to convert into a cJSON type.

### Returns
- `cJSON*` - Returns a cJSON object representing the detected type.

----------

## `HTTP_cJSON_GetStringValue`

### Description
Retrieves the string value of a given cJSON key-value pair. If the value is a string, then the value is returned directly. If it is a integer or a float, it is converted to a string.

### Parameters
- `arena` *(Arena\*)* -  arena The Arena used for temporary string storage.

- `elem` *(cJSON\*)* -  elem The cJSON key-value pair to extract the string value from.

### Returns
- `char*` - Returns a string representation of the value in the key-value pair.

----------

## `HTTP_cJSON_AddPermissionArrayToObject`

### Description
Adds an array of permissions to a given cJSON object under the key "permissions".

### Parameters
- `object` *(cJSON\*)* -  object The cJSON object to which the permissions array will be added.

- `permissions` *(StringArray)* -  permissions The StringArray containing the permission strings.

### Returns
- `void` - None