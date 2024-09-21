cmake_minimum_required(VERSION 3.16)
project(HTTPServer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.c)

include_directories(${PROJECT_SOURCE_DIR}/vendor/include)
find_library(RE_LIB re.lib PATHS ${PROJECT_SOURCE_DIR}/vendor)
find_library(CJSON_LIB cJSON.lib PATHS ${PROJECT_SOURCE_DIR}/vendor)
find_library(CJSON_UTILS_LIB cjson_utils.lib PATHS ${PROJECT_SOURCE_DIR}/vendor)
find_library(SQLITE_LIB sqlite3.lib PATHS ${PROJECT_SOURCE_DIR}/vendor)

add_definitions(-DDEBUG_BUILD)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
if (MSVC) 
    # Required to make the MAP macro work properly
    add_compile_options(/Zc:preprocessor)
    add_compile_options(-fsanitize=address) # # TODO: Remove after testing.
    add_link_options(/INCREMENTAL:no)
endif()

add_executable(HTTPServer ${SOURCES})
target_link_libraries(HTTPServer ${RE_LIB} ${CJSON_LIB} ${CJSON_UTILS_LIB} ${SQLITE_LIB} Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib)

add_custom_command(TARGET HTTPServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/vendor/sqlite3.dll
        $<TARGET_FILE_DIR:HTTPServer>)

# if (MSVC) 
#     target_link_options(HTTPServer PUBLIC /LARGEADDRESSAWARE)

# endif()
