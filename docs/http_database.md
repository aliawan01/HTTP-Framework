# http\_database.h



----------

## `HTTP_CreateDatabase`

### Description
Creates a database file at the given file path.

### Parameters
- `file_path` *(char\*)* -  Path to where the database should be created.

### Returns
- `void` - None

----------

## `HTTP_CloseDatabase`

### Description
Closes the database connection if it is initialized. This function ensures that the database is closed gracefully, releasing any resources associated with it.

### Returns
- `void` - None

----------

## `HTTP_RunSQLQuery`

### Description
Runs a SQL query on the connected database. If the `list_response` flag is set to false then the function returns a cJSON object containing key-value pairs. If it is set to true then all of the key-value pairs will be stored in an array with the key `root` and the final cJSON object will returned. This function can also optionally convert data types if `convert_types` is true.

### Parameters
- `sql_query` *(char\*)* -  The SQL query string to execute.

- `list_response` *(bool)* -  Flag indicating whether the result should be returned as a list with key `root` or a single object.

- `convert_types` *(bool)* -  Flag indicating whether data types should be converted (e.g., numbers, booleans, `null` values).

### Returns
- `cJSON*` - Returns a cJSON object representing the query result. If an error occurs, NULL is returned.

----------

## `HTTP_InsertJSONIntoDatabase`

### Description
Inserts the provided JSON data into the database. This function iterates through the JSON array and generates an `INSERT INTO` SQL query to insert the data into the corresponding table in the database.

### Parameters
- `json_obj` *(cJSON\*)* -  The cJSON object (or array of objects) to insert into the database.

### Returns
- `void` - This function does not return any value. If an error occurs during insertion, an error message is printed.

----------

## `HTTP_IsDatabaseConnected`

### Description
Checks if the database connection is currently established. This function returns true if the database is connected, otherwise false.

### Returns
- `bool` - Returns true if the database is connected, false otherwise.