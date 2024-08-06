@echo off

cmake -Bbuild -G Ninja && pushd build && ninja & popd
