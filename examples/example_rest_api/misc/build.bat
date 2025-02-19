@echo off

xcopy /s /e /y ..\..\include include\
xcopy /s /e /y ..\..\vendor vendor\
xcopy /s /e /y ..\static static\
if not exist build mkdir build
cmake -Bbuild -G Ninja && pushd build && ninja & popd
