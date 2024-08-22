cmake_minimum_required(VERSION 3.16)
project(HTTPServer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.c)

include_directories(${PROJECT_SOURCE_DIR}/vendor/include)
find_library(RE_LIB re.lib cJSON.lib PATHS ${PROJECT_SOURCE_DIR}/vendor)
find_library(CJSON_LIB cJSON.lib PATHS ${PROJECT_SOURCE_DIR}/vendor)

add_definitions(-DDEBUG_BUILD)
add_compile_options(-fsanitize=address)
add_link_options(/INCREMENTAL:no)

add_executable(HTTPServer ${SOURCES})
target_link_libraries(HTTPServer ${RE_LIB} ${CJSON_LIB} Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib)
# if (MSVC) 
#     target_link_options(HTTPServer PUBLIC /LARGEADDRESSAWARE)

# endif()
