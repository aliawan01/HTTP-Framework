cmake_minimum_required(VERSION 3.21)
project(BasicHTTPServerExample)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/vendor/include")
include_directories("${PROJECT_SOURCE_DIR}/include")

if (MSVC) 
    set(LIB_FOLDER "${PROJECT_SOURCE_DIR}/vendor/windows")
    set(WIN_OPENSSL_APPLINK_PATH "${PROJECT_SOURCE_DIR}/vendor/include/openssl/applink.c")

    find_library(RE_LIB re.lib PATHS ${LIB_FOLDER})
    find_library(CJSON_LIB cjson.lib PATHS ${LIB_FOLDER})
    find_library(CJSON_UTILS_LIB cjson_utils.lib PATHS ${LIB_FOLDER})
    find_library(SQLITE_LIB sqlite3.lib PATHS ${LIB_FOLDER})
    find_library(CRYPTO_LIB libcrypto.lib PATHS ${LIB_FOLDER})
    find_library(SSL_LIB libssl.lib PATHS ${LIB_FOLDER})

    find_library(HTTP_LIB HTTPServerLib.lib PATHS ../../build/)

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
    # NOTE(ali): Required to make the MAP macro work properly
    add_compile_options(/Zc:preprocessor)
    add_compile_options(/Zc:preprocessor)
    add_compile_options(/DEBUG)
    # add_compile_options(/W3) # TODO: Remove after testing.
    add_link_options(/INCREMENTAL:no)
else()
     set(LIB_FOLDER "${PROJECT_SOURCE_DIR}/vendor/linux")
     set(WIN_OPENSSL_APPLINK_PATH "")

     find_library(RE_LIB libre.a PATHS ${LIB_FOLDER})
     find_library(CJSON_LIB libcjson.a PATHS ${LIB_FOLDER})
     find_library(CJSON_UTILS_LIB libcjson_utils.a PATHS ${LIB_FOLDER})
     find_library(HTTP_LIB libHTTPServerLib.so PATHS "/usr/local/lib" "../../build")

    # add_compile_options(-fsanitize=address)
endif()

add_executable(basichttpserverexample "server.c" ${WIN_OPENSSL_APPLINK_PATH})
if (MSVC) 
    target_link_libraries(basichttpserverexample ${RE_LIB} ${CJSON_LIB} ${CRYPTO_LIB} ${SSL_LIB} ${CJSON_UTILS_LIB} ${SQLITE_LIB} ${HTTP_LIB} Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib)
    target_link_options(basichttpserverexample PUBLIC /LARGEADDRESSAWARE)

    find_file(HTTP_DLL NAMES HTTPServerLib.dll PATHS "C:/Windows/System32") 
    if (NOT HTTP_DLL) 
        add_custom_command(TARGET basichttpserverexample POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${PROJECT_SOURCE_DIR}/../../build/bin/HTTPServerLib.dll
            $<TARGET_FILE_DIR:basichttpserverexample>)
    endif()

    add_custom_command(TARGET basichttpserverexample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${PROJECT_SOURCE_DIR}/vendor/windows/sqlite3.dll
        $<TARGET_FILE_DIR:basichttpserverexample>)
else()
    target_link_libraries(basichttpserverexample ${RE_LIB} ${CJSON_LIB} ${CJSON_UTILS_LIB} ${HTTP_LIB} pthread ssl crypto sqlite3)
endif()


