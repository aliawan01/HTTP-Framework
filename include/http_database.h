#pragma once

#ifdef __cplusplus
extern "C" { 
#endif

typedef struct {
    cJSON* json;
    bool   convert_types;
} JSONAndTypeFormat;

/*
   @desc Creates a database file at the given file path.
   @param file_path Path to where the database should be created.
*/
HTTPEXPORTFUNC void   HTTP_CreateDatabase(char* file_path);

/*
   @desc Closes the database connection if it is initialized.
         This function ensures that the database is closed gracefully,
         releasing any resources associated with it.
*/
HTTPEXPORTFUNC void HTTP_CloseDatabase(void);

/*
   @desc Runs a SQL query on the connected database.
         If the `list_response` flag is set to false then the function returns
         a cJSON object containing key-value pairs. If it is set to true then 
         all of the key-value pairs will be stored in an array with the key `root`
         and the final cJSON object will returned. This function can also optionally
         convert data types if `convert_types` is true.
   @param sql_query The SQL query string to execute.
   @param list_response Flag indicating whether the result should be returned as a list with key `root` or a single object.
   @param convert_types Flag indicating whether data types should be converted (e.g., numbers, booleans, `null` values).
   @return Returns a cJSON object representing the query result. If an error occurs, NULL is returned.
*/
HTTPEXPORTFUNC cJSON* HTTP_RunSQLQuery(char* sql_query, bool list_response, bool convert_types);

/*
   @desc Inserts the provided JSON data into the database.
         This function iterates through the JSON array and generates
         an `INSERT INTO` SQL query to insert the data into the corresponding
         table in the database.
   @param json_obj The cJSON object (or array of objects) to insert into the database.
   @return This function does not return any value. If an error occurs during insertion,
           an error message is printed.
*/
HTTPEXPORTFUNC void HTTP_InsertJSONIntoDatabase(cJSON* json_obj);

/*
   @desc Checks if the database connection is currently established.
         This function returns true if the database is connected, otherwise false.
   @return Returns true if the database is connected, false otherwise.
*/
HTTPEXPORTFUNC bool HTTP_IsDatabaseConnected(void);

#ifdef __cplusplus
}
#endif
