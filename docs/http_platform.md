# http\_platform.h



----------

## `HTTP_CreateDir`

### Description
Creates a directory at the specified file path and returns the status of the operation.

### Parameters
- `file_path` *(char\*)* -  The path where the directory should be created.

### Returns
- `enum` - Returns a `HTTPCreateDirStatus` enum indicating the status of the directory creation operation.

----------

## `HTTP_DeleteDirRecursive`

### Description
Recursively deletes a directory and its contents. This function deletes the specified directory and any files or subdirectories inside it.

### Parameters
- `dir_name` *(char\*)* -  The name of the directory to delete.

### Returns
- `bool` - Returns true if the directory and its contents were successfully deleted, false otherwise.

----------

## `HTTP_Gen256ByteRandomNum`

### Description
Generates a 256 digit random number and stores it as a string in the buffer provided.

### Parameters
- `buffer` *(char\*)* -  The buffer to store the generated random number.

- `buffer_count` *(int)* -  The size of the buffer (should be at least 256 bytes).

### Returns
- `void` - None