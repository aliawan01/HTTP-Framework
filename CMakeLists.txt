cmake_minimum_required(VERSION 3.16)
project(HTTPServer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.c)
# file(GLOB SOURCES CONFIGURE_DEPENDS src/server.c src/platform.c)
include_directories(${PROJECT_SOURCE_DIR}/vendor/include)


# TODO: Absolutley stupid fix this.
# set(CRYPTO_LIB "C:\\Program Files\\OpenSSL-Win64\\lib\\VC\\x64\\MD\\libcrypto.lib")
# set(SSL_LIB "C:\\Program Files\\OpenSSL-Win64\\lib\\VC\\x64\\MD\\libssl.lib")

add_definitions(-DDEBUG_BUILD)
if (MSVC) 
    find_library(RE_LIB re.lib PATHS ${PROJECT_SOURCE_DIR}/vendor/windows)
    find_library(CJSON_LIB cJSON.lib PATHS ${PROJECT_SOURCE_DIR}/vendor/windows)
    find_library(CJSON_UTILS_LIB cjson_utils.lib PATHS ${PROJECT_SOURCE_DIR}/vendor/windows)
    find_library(SQLITE_LIB sqlite3.lib PATHS ${PROJECT_SOURCE_DIR}/vendor/windows)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
    # Required to make the MAP macro work properly
    add_compile_options(/Zc:preprocessor)
    add_compile_options(-fsanitize=address) # # TODO: Remove after testing.
    # add_compile_options(/W3) # # TODO: Remove after testing.
    add_link_options(/INCREMENTAL:no)

    add_custom_command(target httpserver post_build
        command ${cmake_command} -e copy_if_different
            ${project_source_dir}/vendor/windows/sqlite3.dll
            $<target_file_dir:httpserver>)
else()
    find_library(RE_LIB libre.a PATHS ${PROJECT_SOURCE_DIR}/vendor/linux)
    find_library(CJSON_LIB libcJSON.a PATHS ${PROJECT_SOURCE_DIR}/vendor/linux)
    find_library(CJSON_UTILS_LIB libcJSON_Utils.a PATHS ${PROJECT_SOURCE_DIR}/vendor/linux)

endif()

# add_executable(HTTPServer ${SOURCES} "C:/Program Files/OpenSSL-Win64/include/openssl/applink.c")
add_executable(HTTPServer ${SOURCES})
if (MSVC) 
    target_link_libraries(HTTPServer ${RE_LIB} ${CJSON_LIB} ${CRYPTO_LIB} ${SSL_LIB} ${CJSON_UTILS_LIB} ${SQLITE_LIB} Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib)
else()
    target_link_libraries(HTTPServer ${RE_LIB} ${CJSON_LIB} ${CJSON_UTILS_LIB} pthread ssl crypto sqlite3)
endif()

# TODO(ali): Add this into the if (MSVC) statement
# if (MSVC) 
#     target_link_options(HTTPServer PUBLIC /LARGEADDRESSAWARE)

# endif()
