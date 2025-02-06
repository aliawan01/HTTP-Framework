@echo off

mkdir "vendor/windows"
mkdir "vendor/certs"
mkdir "vendor/include"

cd vendor\windows

git clone https://github.com/DaveGamble/cJSON cjson
git clone https://github.com/kokke/tiny-regex-c regex

curl -L https://www.sqlite.org/2025/sqlite-dll-win-x64-3480000.zip --output sqlite-dll.zip
curl -L https://www.sqlite.org/2025/sqlite-amalgamation-3480000.zip --output sqlite-source.zip

mkdir openssl-dir
cd openssl-dir

curl -L https://download.firedaemon.com/FireDaemon-OpenSSL/openssl-3.4.0.zip --output openssl.zip

tar -xf openssl.zip

move x64\lib\libcrypto.lib ..\
move x64\lib\libssl.lib ..\

move x64\include\openssl ..\..\include

cd ..

cd cjson

cl /nologo /c cJSON.c
cl /nologo /c cJSON_Utils.c

lib /nologo /out:cjson.lib cJSON.obj
lib /nologo /out:cjson_utils.lib cJSON_Utils.obj

move /y cjson.lib ..
move /y cjson_utils.lib ..

move /y cJSON.h ..\..\include
move /y cJSON_Utils.h ..\..\include

cd ..\regex

cl /nologo /c re.c

lib /nologo /out:re.lib re.obj

move /y re.lib ..

move /y re.h ..\..\include

cd ..

tar -xf sqlite-dll.zip
tar -xf sqlite-source.zip

lib /def:sqlite3.def /out:sqlite3.lib /machine:x64

cd sqlite-amalgamation-3480000
move sqlite3.h ..\..\include
cd ..

del /f *.exp *.def sqlite-dll.zip sqlite-source.zip
rmdir /s /q cjson regex sqlite-amalgamation-3480000 openssl-dir

cd ..\certs

curl -L https://github.com/FiloSottile/mkcert/releases/download/v1.4.4/mkcert-v1.4.4-windows-amd64.exe --output mkcert.exe

mkcert -install
mkcert update-ca-certificates
mkcert -key-file key.pem -cert-file cert.pem localhost 

del /f mkcert.exe update*

cd ..\..\
misc\build
