@echo off

if not exist build mkdir build
cmake -Bbuild -G Ninja && pushd build && ninja & popd
REM if not exist build (
REM     mkdir build
REM )

REM set LIBRARIES=Ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
REM set LIBRARIES=%LIBRARIES% ..\vendor\windows\cjson.lib ..\vendor\windows\cjson_utils.lib 

REM pushd build && cl /nologo /std:c11 /W3 /D_CRT_SECURE_NO_WARNINGS /D_CRT_RAND_S /DDEBUG_BUILD /I ..\include /I ..\vendor\include /I ..\src /Zi ..\lexer\main.c %LIBRARIES% && popd
