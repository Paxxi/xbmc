@ECHO OFF

PUSHD %~dp0\..
CALL download-dependencies.bat win10-arm64
POPD
