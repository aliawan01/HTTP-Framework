#!/bin/bash

mkdir -p build && cmake -Bbuild -G Ninja && pushd build && ninja & popd
