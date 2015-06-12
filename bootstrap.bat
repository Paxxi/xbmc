@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
REM setup all paths
SET cur_dir=%CD%
SET base_dir=%CD%
SET builddeps_dir=%cur_dir%\project\BuildDependencies
SET proj_dir=%base_dir%\project\VS2010Express
SET bin_dir=%builddeps_dir%\bin
SET msys_bin_dir=%builddeps_dir%\msys\bin
REM read the version values from version.txt

FOR /f "tokens=1,2" %%i IN (version.txt) DO (
  IF 'APP_NAME'=='%%i' SET APP_NAME=%%j
  IF 'COMPANY_NAME'=='%%i' SET COMPANY=%%j
  IF 'WEBSITE'=='%%i' SET WEBSITE=%%j
)

rem ----Usage----
rem BuildSetup [clean|noclean]
rem clean to force a full rebuild
rem noclean to force a build without clean
rem noprompt to avoid all prompts
rem nomingwlibs to skip building all libs built with mingw
CLS
COLOR 1B
TITLE %APP_NAME% for Windows Build Script
rem ----PURPOSE----
rem - Create a working application build with a single click
rem -------------------------------------------------------------
rem Config
rem If you get an error that Visual studio was not found, SET your path for VSNET main executable.
rem -------------------------------------------------------------
rem  CONFIG START
SET buildmode=clean
SET promptlevel=prompt
SET buildmingwlibs=true
SET buildbinaryaddons=true
SET exitcode=0
SET useshell=rxvt
SET BRANCH=na
FOR %%b in (%1, %2, %3, %4, %5) DO (
  IF %%b==nomingwlibs SET buildmingwlibs=false
  IF %%b==nobinaryaddons SET buildbinaryaddons=false
  IF %%b==sh SET useshell=sh
)

SET buildconfig=Release
set WORKSPACE=%base_dir%

cd %builddeps_dir%
call DownloadBuildDeps.bat
call DownloadMingwBuildEnv.bat
cd %base_dir%

REM Check that we have cmake
cmake.exe 2>&1 > nul
IF NOT %errorlevel%==0 (
  SET DIETEXT=Cmake not found, make sure this is installed and exists on your path
  goto DIE
)

REM Load vsvars so we find our tools
IF DEFINED VS120COMNTOOLS (
  call "%VS120COMNTOOLS%\vsvars32.bat"
) ELSE (
  SET DIETEXT=VS 12 common tools not found, is Visual Studio 2013 installed?
  goto DIE
)

REM look for MSBuild.exe delivered with Visual Studio 2013
FOR /F "tokens=2,* delims= " %%A IN ('REG QUERY HKLM\SOFTWARE\Microsoft\MSBuild\ToolsVersions\12.0 /v MSBuildToolsRoot') DO SET MSBUILDROOT=%%B
SET NET="%MSBUILDROOT%12.0\bin\MSBuild.exe"

IF EXIST "!NET!" (
  set msbuildemitsolution=1
  set OPTS_EXE="%proj_dir%\XBMC for Windows.sln" /t:Build /p:Configuration="%buildconfig%" /property:VCTargetsPath="%MSBUILDROOT%Microsoft.Cpp\v4.0\V120" /m
  set CLEAN_EXE="%proj_dir%\XBMC for Windows.sln" /t:Clean /p:Configuration="%buildconfig%" /property:VCTargetsPath="%MSBUILDROOT%Microsoft.Cpp\v4.0\V120"
)

IF NOT EXIST %NET% (
  set DIETEXT=MSBuild was not found.
  goto DIE
)

set EXE= "%proj_dir%\XBMC\%buildconfig%\%APP_NAME%.exe"
set PDB= "%proj_dir%\XBMC\%buildconfig%\%APP_NAME%.pdb"

:: sets the BRANCH env var
call "%base_dir%\project\Win32BuildSetup\getbranch.bat"

rem  CONFIG END
rem -------------------------------------------------------------
goto COMPILE_MINGW
  

:COMPILE_MINGW
  ECHO Buildmode = %buildmode%
  IF %buildmingwlibs%==true (
    ECHO Compiling mingw libs
    ECHO bla>noprompt
    IF EXIST errormingw del errormingw > NUL
    IF %buildmode%==clean (
      ECHO bla>makeclean
    )
    rem only use sh to please jenkins
    IF %useshell%==sh (
      call "%base_dir%\tools\buildsteps\win32\make-mingwlibs.bat" sh noprompt
    ) ELSE (
      call "%base_dir%\tools\buildsteps\win32\make-mingwlibs.bat" noprompt
    )
    IF EXIST errormingw (
      set DIETEXT="failed to build mingw libs"
      goto DIE
    )
  )
  goto COMPILE_EXE
  
  
:COMPILE_EXE
  ECHO Wait while preparing the build.
  ECHO ------------------------------------------------------------
  ECHO Cleaning Solution...
  %NET% %CLEAN_EXE%
  ECHO Compiling %APP_NAME% branch %BRANCH%...
  %NET% %OPTS_EXE%
  IF %errorlevel%==1 (
    set DIETEXT="%APP_NAME%.EXE failed to build!  See %proj_dir%\XBMC\%buildconfig%\objs\XBMC.log"
    IF %promptlevel%==noprompt (
      type "%proj_dir%\XBMC\%buildconfig%\objs\XBMC.log"
    )
    goto DIE
  )
  ECHO Done!
  ECHO ------------------------------------------------------------
  set buildmode=clean
  GOTO MAKE_BUILD_EXE

:MAKE_BUILD_EXE
  SET build_path=%CD%
  IF %buildbinaryaddons%==true (
    ECHO ------------------------------------------------------------
    ECHO Building addons...
    cd "%base_dir%\tools\buildsteps\win32"
    IF %buildmode%==clean (
      call make-addons.bat clean
    )
    call make-addons.bat
    IF %errorlevel%==1 (
      set DIETEXT="failed to build addons"
      cd %build_path%
      goto DIE
    )
  
    cd %build_path%
    IF EXIST error.log del error.log > NUL
  )
  
  ECHO ------------------------------------------------------------
  ECHO Building Confluence Skin...
  cd "%base_dir%\addons\skin.confluence"
  call build.bat > NUL
  cd %build_path%
  
  IF EXIST  "%base_dir%\addons\skin.re-touched\build.bat" (
    ECHO Building Touch Skin...
    cd "%base_dir%\addons\skin.re-touched"
    call build.bat > NUL
    cd %build_path%
  )
  
  rem restore color and title, some scripts mess these up
  COLOR 1B
  TITLE %APP_NAME% for Windows Build Script
  
  ECHO ------------------------------------------------------------
  ECHO Build Succeeded!


:DIE
  ECHO ------------------------------------------------------------
  ECHO !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-
  ECHO    ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR
  ECHO !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-
  set DIETEXT=ERROR: %DIETEXT%
  echo %DIETEXT%
  SET exitcode=1
  ECHO ------------------------------------------------------------
  GOTO END

:VIEWLOG_EXE
  SET log="%proj_dir%\XBMC\%buildconfig%\objs\XBMC.log"
  IF NOT EXIST %log% goto END
  
  copy %log% ./buildlog.html > NUL

  IF %promptlevel%==noprompt (
    goto END
  )

  set /P APP_BUILD_ANSWER=View the build log in your HTML browser? [y/n]
  if /I %APP_BUILD_ANSWER% NEQ y goto END
  
  SET log="%proj_dir%\XBMC\%buildconfig%\objs\" XBMC.log
  
  start /D%log%
  goto END

:END
  IF %promptlevel% NEQ noprompt (
    ECHO Press any key to exit...
    pause > NUL
  )
  EXIT /B %exitcode%
