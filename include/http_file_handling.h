#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

/*
   @desc Checks if a file exists at the given file path.
   @param file_name The name of the file to check exists.
   @return Returns true if the file exists, false otherwise.
*/
HTTPEXPORTFUNC bool HTTP_FileExists(char* file_name);

/*
   @desc Deletes a file at the given file path and prints a message
         indicating success or failure.
   @param file_name The name of the file to delete.
   @return Returns true if the file was successfully deleted, false otherwise.
*/
HTTPEXPORTFUNC bool HTTP_DeleteFile(char* file_name);

// TODO(ali): Should we say we add 1 to the final number? Does that give the
//            correct or incorrect file size?
/*
   @desc Finds the size of a file at the given file path in bytes.
   @param file_name The name of the file to check.
   @return Returns the size of the file in bytes, or -1 if an error
           occurs (e.g. file was not found).
*/
HTTPEXPORTFUNC int64_t HTTP_FindFileSize(char *file_name);

/*
   @desc Retrieves the contents of a file as a string.
   @param arena The Arena to allocate memory for the file contents.
   @param file_name The name of the file to read from.
   @return Returns a String containing the file contents. If the file
           cannot be opened, an empty String is returned (contains
           NULL for the string and sets the count to 0).
*/
HTTPEXPORTFUNC String HTTP_GetFileContents(Arena* arena, char* file_name);

/*
   @desc Overwrites the contents of a file with new data.
   @param file_name The name of the file to overwrite.
   @param new_file_contents The new contents to write to the file.
   @return Returns true if the file contents were successfully overwritten, 
           false otherwise. This function also prints an error if we couldn't
           open the file.
*/
HTTPEXPORTFUNC bool HTTP_OverwriteFileContents(char* file_name, char* new_file_contents);

#ifdef __cplusplus
}
#endif
