# http\_file\_handling.h



----------

## `HTTP_FileExists`

### Description
Checks if a file exists at the given file path.

### Parameters
- `file_name` *(char\*)* -  file_name The name of the file to check exists.

### Returns
- `bool` - Returns true if the file exists, false otherwise.

----------

## `HTTP_DeleteFile`

### Description
Deletes a file at the given file path and prints a message indicating success or failure.

### Parameters
- `file_name` *(char\*)* -  file_name The name of the file to delete.

### Returns
- `bool` - Returns true if the file was successfully deleted, false otherwise.

----------

## `HTTP_FindFileSize`

### Description
Finds the size of a file at the given file path in bytes.

### Parameters
- `file_name` *(char)* -  file_name The name of the file to check.

### Returns
- `int64_t` - Returns the size of the file in bytes, or -1 if an error occurs (e.g. file was not found).

----------

## `HTTP_GetFileContents`

### Description
Retrieves the contents of a file as a string.

### Parameters
- `arena` *(Arena\*)* -  arena The Arena to allocate memory for the file contents.

- `file_name` *(char\*)* -  file_name The name of the file to read from.

### Returns
- `String` - Returns a String containing the file contents. If the file cannot be opened, an empty String is returned (contains NULL for the string and sets the count to 0).

----------

## `HTTP_OverwriteFileContents`

### Description
Overwrites the contents of a file with new data.

### Parameters
- `file_name` *(char\*)* -  file_name The name of the file to overwrite.

- `new_file_contents` *(char\*)* -  new_file_contents The new contents to write to the file.

### Returns
- `bool` - Returns true if the file contents were successfully overwritten, false otherwise. This function also prints an error if we couldn't open the file.