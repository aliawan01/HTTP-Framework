# http\_request.h



----------

## `HTTP_Initialize`

### Description
Initializes the HTTPS server and sets up necessary memory arenas, thread context and cJSON hooks.

### Returns
- `void` - None

----------

## `*response_func)`

### Description
Handles the registration of a new route with specified permissions and callback function. This function ensures the route is valid and does not already exist before adding it.

 - NOTE: A route path can be duplicated with different/same callback functions if the method being used is different.
 e.g. the following snippet is valid. `HTTP_HandleRoute(StrArrayLit("global"), "GET", "/a_route", true, first_callback);\n HTTP_HandleRoute(StrArrayLit("global"), "POST", "/a_route", true, second_callback);`

### Parameters
- `permissions` *(StringArray)* -  permissions The permissions required for the route.

- `method` *(char\*)* -  method The HTTP method (GET, POST, etc.) for the route.

- `route` *(char\*)* -  route The route path.

- `is_regex_route` *(bool)* -  is_regex_route Flag to indicate if the route uses regular expressions.

- `response_func` *(void)* -  response_func The callback function to handle responses for the route.

### Returns
- `bool` - Returns true if the route was successfully added, false if there was an error or conflict.

----------

## `HTTP_DeleteRouteForMethod`

### Description
Deletes a specific route for the specified HTTP method. The route will be removed if we can find one which matches the specified method and route.

### Parameters
- `method` *(char\*)* -  method The HTTP method (GET, POST, etc.) for the route to delete.

- `route` *(char\*)* -  route The route path to delete.

- `is_regex_route` *(bool)* -  is_regex_route Flag to indicate if the route is a regex route.

### Returns
- `bool` - Returns true if the route was successfully deleted, false otherwise.

----------

## `HTTP_DeleteRouteForAllMethod`

### Description
Deletes a specific route for all HTTP methods. All routes which match the specified attributes will be removed.

### Parameters
- `route` *(char\*)* -  route The route path to delete.

- `is_regex_route` *(bool)* -  is_regex_route Flag to indicate if the route is a regex route.

### Returns
- `bool` - Returns true if the routes was successfully deleted, false otherwise.

----------

## `HTTP_SetSearchDirectories`

### Description
Sets the search directories for the HTTP server. This function allows specifying directories to search for files when serving requests.

### Parameters
- `dirs` *(char\*)* -  dirs Array of directory paths.

- `dirs_size` *(size_t)* -  dirs_size The number of directories in the `dirs` array.

### Returns
- `void` - None

----------

## `HTTP_Set404Page`

### Description
Sets the path for the custom 404 error page. If a valid file path is provided, it will be used as the error page, otherwise the default 404 error page will be used.

### Parameters
- `path_to_error_page` *(char\*)* -  path_to_error_page The file path to the 404 error page (should be a html file).

### Returns
- `bool` - Returns true if the 404 page was successfully set, false if the file does not exist.

----------

## `HTTP_RunServer`

### Description
Starts the HTTP server on the specified port with SSL encryption. This function sets up the server, initializes OpenSSL, and begins accepting client connections. If the certificate or private key file paths are invalid then the server will not statup and will return 1.

### Parameters
- `server_port` *(char\*)* -  server_port The port on which the server will run.

- `path_to_certificate` *(char\*)* -  path_to_certificate The file path to the SSL certificate.

- `path_to_private_key` *(char\*)* -  path_to_private_key The file path to the SSL private key.

### Returns
- `int` - Returns 0 if the server starts successfully, or a 1 if the server setup fails.

----------

## `HTTP_AddHeaderToHeaderDict`

### Description
Adds a header to the HTTP response header dictionary. This function adds a key-value pair to the header dictionary, which will be included in the HTTP response (should only be used inside callback functions).

### Parameters
- `arena` *(Arena\*)* -  arena The Arena to allocate memory for the header.

- `header_dict` *(HeaderDict\*)* -  header_dict The dictionary to store the HTTP headers.

- `key` *(char\*)* -  key The header key.

- `value` *(char\*)* -  value The header value.

### Returns
- `void` - This function does not return any value.

----------

## `HTTP_AddCookieToCookieJar`

### Description
Adds a cookie to the cookie jar. This function creates a new cookie with the specified attributes and adds it to the provided cookie jar (should only be used inside of callback functions).

### Parameters
- `arena` *(Arena\*)* -  arena The Arena to allocate memory for the cookie.

- `cookie_jar` *(CookieJar\*)* -  cookie_jar The cookie jar to which the cookie will be added.

- `key` *(char\*)* -  key The cookie key.

- `value` *(char\*)* -  value The cookie value.

- `max_age` *(int64_t)* -  max_age The maximum age of the cookie in seconds.

- `expires` *(char\*)* -  expires The expiry date of the cookie.

- `path` *(char\*)* -  path The route for which the cookie is valid.

- `domain` *(char\*)* -  domain The domain for which the cookie is valid.

- `secure` *(bool)* -  secure Flag indicating whether the cookie is secure.

- `http_only` *(bool)* -  http_only Flag indicating whether the cookie should only be used with HTTP.

### Returns
- `void` - None

----------

## `HTTP_TemplateText`

### Description
Templates text by replacing variables in the file with actual values from the HTTP request and JSON values. This function processes template files (e.g. html) and replaces placeholders with corresponding values from the request or values from any JSON data that has been sent (should only be used inside of a callback function).

### Parameters
- `arena` *(Arena\*)* -  arena The Arena for memory allocation.

- `request_info` *(HTTPRequestInfo\*)* -  request_info The HTTPRequestInfo object passed into the callback function.

- `variables` *(cJSON\*)* -  variables A cJSON object containing the variables to be replaced.

- `source` *(String\*)* -  source The source string to be modified (you can get the contents of a file as a string using `HTTP_GetFileContents`), the source string will be modified in place.

### Returns
- `void` - None

----------

## `HTTP_TemplateTextFromFile`

### Description
Loads and templates a file by replacing variables with actual values from the HTTP request and JSON values. This function loads the file, templates it by replacing placeholders with actual values, and returns the modified string.

### Parameters
- `arena` *(Arena\*)* -  arena The Arena for memory allocation.

- `request_info` *(HTTPRequestInfo\*)* -  request_info The HTTPRequestInfo object passed into the callback function.

- `variables` *(cJSON\*)* -  variables A cJSON object containing the variables to be replaced.

- `file_path` *(char\*)* -  file_path The path to the file to be templated.

### Returns
- `String` - Returns the processed content as a String.

----------

## `HTTP_CreateDateString`

### Description
Creates a date string in a specific format. This function creates a date string in the format: "Day, DD Month YYYY HH:MM:SS GMT".

### Parameters
- `arena` *(Arena\*)* -  arena The arena for memory allocation.

- `day_name` *(String)* -  day_name The day of the week (e.g., Mon, Tue, etc.).

- `day_num` *(int)* -  day_num The day of the month (1-31).

- `month` *(String)* -  month The month (e.g., Jan, Feb, etc.).

- `year` *(int)* -  year The year (e.g., 2022).

- `hour` *(int)* -  hour The hour of the day (0-23).

- `minute` *(int)* -  minute The minute (0-59).

- `second` *(int)* -  second The second (0-59).

### Returns
- `char*` - Returns the formatted date string.