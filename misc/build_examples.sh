#!/usr/bin/env bash

pushd examples
cd example_server
chmod +x misc/build.sh
misc/build.sh

cd ../example_rest_api
chmod +x misc/build.sh
misc/build.sh

cd ..
popd
