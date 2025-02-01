#!/usr/bin/env bash

# NOTE(ali): Installing dependencies
sudo apt install build-essential openssl libssl-dev sqlite3 \
                 libsqlite3-dev libnss3-tools mkcert

mkdir -p vendor/linux
mkdir -p vendor/certs
mkdir -p vendor/include

cd vendor/linux

git clone https://github.com/DaveGamble/cJSON cjson
git clone https://github.com/kokke/tiny-regex-c regex

# NOTE(ali): Building cJSON libs.
cd cjson
cmake . -DENABLE_CJSON_UTILS=On -DBUILD_SHARED_LIBS=Off
make

mv libcjson.a ..
mv libcjson_utils.a ..
mv cJSON.h ../../include
mv cJSON_Utils.h ../../include


# NOTE(ali): Building tiny-regex-c lib.
cd ../regex
gcc -c re.c -o re.o
ar rcs libre.a re.o
mv libre.a ..
mv re.h ../../include

# NOTE(ali): Cleaning up
cd ..
rm -rf regex cjson

# NOTE(ali): Creating certificates
cd ../certs
mkcert -install
mkcert update-ca-certificates
# NOTE(ali): When building for another website change the name from localhost to the URL to your website e.g  www.example.com
mkcert -key-file key.pem -cert-file cert.pem localhost 
rm update*

cd ../..
misc/build.sh
