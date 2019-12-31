@ECHO OFF

PUSHD %~dp0\..
CALL vswhere.bat arm64
IF ERRORLEVEL 1 (
  ECHO ERROR! BuildSetup.bat: Something went wrong when calling vswhere.bat
  POPD
  EXIT /B 1
)

SET cmakeGenerator=Visual Studio %vsver%
SET TARGET_ARCHITECTURE=arm64

CALL BuildSetup.bat %*
POPD
