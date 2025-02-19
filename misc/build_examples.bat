@echo off

pushd examples
cd example_server
call "misc\build"

cd ..\example_rest_api
call "misc\build"
popd

