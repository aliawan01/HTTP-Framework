# TODO(ali): Clean this up and make it work on Windows.
cmake_minimum_required(VERSION 3.16)
project(BasicHTTPServerExample)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/vendor/include")
include_directories("${PROJECT_SOURCE_DIR}/include")

# TODO: Absolutely stupid fix this.
# set(CRYPTO_LIB "C:\\Program Files\\OpenSSL-Win64\\lib\\VC\\x64\\MD\\libcrypto.lib")
# set(SSL_LIB "C:\\Program Files\\OpenSSL-Win64\\lib\\VC\\x64\\MD\\libssl.lib")

if (MSVC) 
    set(LIB_FOLDER ${PROJECT_SOURCE_DIR}/../../vendor/windows)
    find_library(RE_LIB re.lib PATHS LIB_FOLDER)
    find_library(CJSON_LIB cJSON.lib PATHS LIB_FOLDER)
    find_library(CJSON_UTILS_LIB cjson_utils.lib PATHS LIB_FOLDER)
    find_library(SQLITE_LIB sqlite3.lib PATHS LIB_FOLDER)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
    # Required to make the MAP macro work properly
    add_compile_options(/Zc:preprocessor)
    add_compile_options(-fsanitize=address) # TODO: Remove after testing.
    # add_compile_options(/W3) # # TODO: Remove after testing.
    add_link_options(/INCREMENTAL:no)

    # TODO(ali): Should be done after add_executable().
    add_custom_command(target BasicHTTPServerExample post_build
        command ${cmake_command} -e copy_if_different
            ${PROJECT_SOURCE_DIR}/../../vendor/windows/sqlite3.dll
            $<target_file_dir:BasicHTTPServerExample>)

    add_custom_command(target BasicHTTPServerExample post_build
        command ${cmake_command} -e copy_if_different
        ${PROJECT_SOURCE_DIR}/../../build/http.dll # TODO(ali): Fix this name or maybe just install it onto the system.
            $<target_file_dir:BasicHTTPServerExample>)
else()
     set(LIB_FOLDER "${PROJECT_SOURCE_DIR}/vendor/linux")

     find_library(RE_LIB libre.a PATHS ${LIB_FOLDER})
     find_library(CJSON_LIB libcjson.a PATHS ${LIB_FOLDER})
     find_library(CJSON_UTILS_LIB libcjson_utils.a PATHS ${LIB_FOLDER})
     find_library(HTTP_LIB libHTTPServerLib.so)

    # add_compile_options(-fsanitize=address)

endif()


# add_executable(BasicHTTPServerExample ${SOURCES} "C:/Program Files/OpenSSL-Win64/include/openssl/applink.c")
add_executable(basichttpserverexample "server.c")
if (MSVC) 
    target_link_libraries(basichttpserverexample ${RE_LIB} ${CJSON_LIB} ${CRYPTO_LIB} ${SSL_LIB} ${CJSON_UTILS_LIB} ${SQLITE_LIB} Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib)
else()
    target_link_libraries(basichttpserverexample ${RE_LIB} ${CJSON_LIB} ${CJSON_UTILS_LIB} ${HTTP_LIB} pthread ssl crypto sqlite3)
endif()

# TODO(ali): Add this into the if (MSVC) statement
# if (MSVC) 
#     target_link_options(BasicHTTPServerExample PUBLIC /LARGEADDRESSAWARE)

# endif()
