@echo off

REM TODO: Should find a way to place certs in the project itself,
REM       and tell the user how to change them.
IF "%SSL_CERT_FILE%"=="" (
    set SSL_CERT_DIR=C:\tools\certs\
    set SSL_CERT_FILE=C:\tools\certs\certs.pem
)

cmake -Bbuild -G Ninja && pushd build && ninja & popd
