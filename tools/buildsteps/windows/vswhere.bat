@ECHO OFF

IF "%1"=="" (
  ECHO ERROR! vswhere.bat: architecture not specified
  EXIT /B 1
)

REM running vcvars more than once can cause problems; exit early if using the same configuration, error if different
IF "%VSWHERE_SET%"=="%*" (
  ECHO vswhere.bat: VC vars already configured for %VSWHERE_SET%
  GOTO :EOF
)
IF "%VSWHERE_SET%" NEQ "" (
  ECHO ERROR! vswhere.bat: VC vars are configured for %VSWHERE_SET%
  EXIT /B 1
)

REM Trick to make the path absolute
PUSHD %~dp0\..\..\..\project\BuildDependencies
SET builddeps=%CD%
POPD

SET arch=%1
SET vcarch=amd64
SET vcstore=%2
SET vcvars=no
SET sdkver=

SET vsver=

SET CPPDESKTOP=Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core
SET CPPDESKTOPNAME=Visual C++ core desktop features
SET CPPX64=Microsoft.VisualStudio.Component.VC.Tools.x86.x64
SET CPPX64NAME=VC++ 2017 version 15.9 v14.16 latest v141 tools
SET CPPX64NAME2019=MSVC v142 - VS 2019 C++ x64/x86 build tools (v14.24)
SET CPPSDK=Microsoft.VisualStudio.Component.Windows10SDK.17763
SET CPPSDKNAME=Windows 10 SDK (10.0.17763.0)

IF "%arch%" NEQ "x64" (
  SET vcarch=%vcarch%_%arch%
)

IF "%vcstore%"=="store" (
  SET sdkver=10.0.17763.0
)

REM It's not an issue to hard code this. This path cannot be changed even if a user
REM opts to install Visual Studio on another drive.
SET vswhere="C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"

FOR /f "tokens=* usebackq" %%i in (`%vswhere% -nologo -version [16^,17^) -property installationPath -requires %CPPDESKTOP% %CPPX64% %CPPSDK%`) do (
  echo "%%i"
  IF EXIST "%%i\VC\Auxiliary\Build\vcvarsall.bat" (
    SET vcvars="%%i\VC\Auxiliary\Build\vcvarsall.bat"
    SET vsver=16
  )
)

IF %vcvars%==no (
  FOR /f "tokens=* usebackq" %%i in (`%vswhere% -nologo -version [15^,16^) -property installationPath -requires %CPPDESKTOP% %CPPX64% %CPPSDK%`) do (
  echo "%%i"
    IF EXIST "%%i\VC\Auxiliary\Build\vcvarsall.bat" (
      SET vcvars="%%i\VC\Auxiliary\Build\vcvarsall.bat"
      SET vsver=15
    )
  )
)

IF %vcvars%==no (
  ECHO "ERROR! Could not find vcvarsall.bat for either VS 2017 or 2019"
  ECHO "ERROR! Kodi requires these workloads to be installed"
  ECHO "ERROR! For Visual Studio 2019"
  ECHO "ERROR! %CPPX64NAME2019% ID=%CPPX64%"
  ECHO "ERROR!"
  ECHO "ERROR! For Visual Studio 2017"
  ECHO "ERROR! %CPPX64NAME% ID=%CPPX64%"
  ECHO "ERROR!"
  ECHO "ERROR! For both versions"
  ECHO "ERROR! %CPPDESKTOPNAME% ID=%CPPDESKTOP%"
  ECHO "ERROR! %CPPSDKNAME% ID=%CPPSDK%"
  EXIT /B 1
)

REM vcvars changes the cwd so we need to store it and restore it
PUSHD %~dp0
CALL %vcvars% %vcarch% %vcstore% %sdkver%
POPD

IF ERRORLEVEL 1 (
  ECHO "ERROR! something went wrong when calling"
  ECHO %vcvars% %vcarch% %vcstore% %sdkver%
  EXIT /B 1
)

SET VSWHERE_SET=%*
