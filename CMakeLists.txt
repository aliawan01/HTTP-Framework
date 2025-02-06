cmake_minimum_required(VERSION 3.16)
project(HTTPServerLib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# TODO(ali): Turn this back into Release after we finished testing it.
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.c)

include_directories(${PROJECT_SOURCE_DIR}/vendor/include)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

# TODO(ali): Remove this when building for Release.
add_definitions(-DDEBUG_BUILD)

add_definitions(-DBUILD_LIB)
if (MSVC) 
    set(LIB_FOLDER "${PROJECT_SOURCE_DIR}/vendor/windows")
    set(WIN_OPENSSL_APPLINK_PATH "${PROJECT_SOURCE_DIR}/vendor/include/openssl/applink.c")

    find_library(RE_LIB re.lib PATHS ${LIB_FOLDER})
    find_library(CJSON_LIB cjson.lib PATHS ${LIB_FOLDER})
    find_library(CJSON_UTILS_LIB cjson_utils.lib PATHS ${LIB_FOLDER})
    find_library(SQLITE_LIB sqlite3.lib PATHS ${LIB_FOLDER})
    find_library(CRYPTO_LIB libcrypto.lib PATHS ${LIB_FOLDER})
    find_library(SSL_LIB libssl.lib PATHS ${LIB_FOLDER})

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
    # Required to make the MAP macro work properly
    add_compile_options(/Zc:preprocessor)
    add_compile_options(-fsanitize=address) # TODO: Remove after testing.
    # add_compile_options(/W3) # # TODO: Remove after testing.
    add_link_options(/INCREMENTAL:no)
    add_link_options(/DEBUG)
else()
    set(LIB_FOLDER "${PROJECT_SOURCE_DIR}/vendor/linux")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    set(WIN_OPENSSL_APPLINK_PATH "")

    find_library(RE_LIB libre.a PATHS ${LIB_FOLDER})
    find_library(CJSON_LIB libcjson.a PATHS ${LIB_FOLDER})
    find_library(CJSON_UTILS_LIB libcjson_utils.a PATHS ${LIB_FOLDER})

    # add_compile_options(-fsanitize=address)
endif()

add_library(HTTPServerLib SHARED ${SOURCES} ${WIN_OPENSSL_APPLINK_PATH})
if (MSVC) 
    target_link_libraries(HTTPServerLib ${RE_LIB} ${CJSON_LIB} ${CRYPTO_LIB} ${SSL_LIB} ${CJSON_UTILS_LIB} ${SQLITE_LIB} Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib)
    target_link_options(HTTPServerLib PUBLIC /LARGEADDRESSAWARE)
else()
    target_link_libraries(HTTPServerLib ${RE_LIB} ${CJSON_LIB} ${CJSON_UTILS_LIB} pthread ssl crypto sqlite3 "-Wl,--exclude-libs,ALL")
endif()
