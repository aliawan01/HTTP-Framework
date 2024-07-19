@echo off

del /q build
mkdir build
pushd build
set linkingLibraries = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
cl /Zi /nologo /Fe:server /I ..\vendor ..\src\server.c %linkingLibraries%
popd