@echo off
setlocal enabledelayedexpansion

if "%1" == "" (
  echo Please specify x86 ^(32-bit^) or x64 ^(64-bit^) architecture in cmdline
  goto :eof
)

set BOTH=0
if not "%1" == "x86" if not "%1" == "x64" set BOTH=1

if %BOTH% == 1  (
  echo Only x86 ^(32-bit^) or x64 ^(64-bit^) architectures are supported!
  goto :eof
)

set ARCH=%1

echo ==^> Checking build dependencies...

set MISSING=0

:: Windows version check (VS 2019 requires Windows 10 1703+)
for /f "tokens=4-6 delims=. " %%a in ('ver') do (
  set WIN_VER_MAJOR=%%a
  set WIN_VER_MINOR=%%b
  set WIN_VER_BUILD=%%c
)
rem Strip trailing ] from build number (present on Windows 8.1 and earlier)
set WIN_VER_BUILD=!WIN_VER_BUILD:]=!
set WIN_VER_OK=1
if !WIN_VER_MAJOR! lss 10 set WIN_VER_OK=0
if !WIN_VER_MAJOR! equ 10 if !WIN_VER_BUILD! lss 15063 set WIN_VER_OK=0
if !WIN_VER_OK! equ 0 (
  echo   [WARN] Windows !WIN_VER_MAJOR!.!WIN_VER_MINOR! ^(build !WIN_VER_BUILD!^) too old
  echo          Visual Studio requires a supported Windows version; VS 2019 needs Windows 10 1703+ ^(build 15063^)
  set /a MISSING+=1
) else (
  echo   [OK] Windows version !WIN_VER_MAJOR!.!WIN_VER_MINOR! build !WIN_VER_BUILD!
)

:: Visual Studio 2019/2022
set VS_FOUND=0
set "VCTOOLS_CRT=Microsoft.VC142.CRT"
if exist "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  set "VS_PATH=c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
  set "CMAKEGEN=Visual Studio 16 2019"
  set VS_FOUND=1
)
if exist "c:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
  set "VS_PATH=c:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
  set "CMAKEGEN=Visual Studio 16 2019"
  set VS_FOUND=1
)
if exist "c:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
  set "VS_PATH=c:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
  set "CMAKEGEN=Visual Studio 16 2019"
  set VS_FOUND=1
)
if !VS_FOUND! == 0 (
  if exist "c:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VS_PATH=c:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
    set "CMAKEGEN=Visual Studio 17 2022"
    set "VCTOOLS_CRT=Microsoft.VC143.CRT"
    set VS_FOUND=1
  )
)
if !VS_FOUND! == 0 (
  if exist "c:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VS_PATH=c:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
    set "CMAKEGEN=Visual Studio 17 2022"
    set "VCTOOLS_CRT=Microsoft.VC143.CRT"
    set VS_FOUND=1
  )
)
if !VS_FOUND! == 0 (
  if exist "c:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VS_PATH=c:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
    set "CMAKEGEN=Visual Studio 17 2022"
    set "VCTOOLS_CRT=Microsoft.VC143.CRT"
    set VS_FOUND=1
  )
)
if !VS_FOUND! == 1 (
  echo   [OK] Visual Studio ^(!VS_PATH!^)
) else (
  echo   [MISSING] Visual Studio 2019 or 2022 with C++ build tools
  echo            Install Visual Studio with Desktop development with C++
  set /a MISSING+=1
)

:: Qt 5.13.2
if "%ARCH%" == "x86" (
  set QT=C:\Qt\5.13.2\msvc2017\
  if exist "!QT!bin\qmake.exe" (
    echo   [OK] Qt 5.13.2 msvc2017 ^(32-bit^)
  ) else (
    echo   [MISSING] Qt 5.13.2 msvc2017 ^(32-bit^)
    echo            Install with: pip install aqtinstall
    echo            Then run: aqt install-qt windows desktop 5.13.2 win32_msvc2017 -O C:\Qt
    set /a MISSING+=1
  )
) else (
  set QT=C:\Qt\5.13.2\msvc2017_64\
  if exist "!QT!bin\qmake.exe" (
    echo   [OK] Qt 5.13.2 msvc2017_64 ^(64-bit^)
  ) else (
    echo   [MISSING] Qt 5.13.2 msvc2017_64 ^(64-bit^)
    echo            Install with: pip install aqtinstall
    echo            Then run: aqt install-qt windows desktop 5.13.2 win64_msvc2017_64 -O C:\Qt
    set /a MISSING+=1
  )
)

:: cmake
where /q cmake.exe
if errorlevel 1 (
  echo   [MISSING] cmake
  echo            Install from https://cmake.org/download/ or run: winget install cmake
  set /a MISSING+=1
) else (
  for /f "tokens=*" %%c in ('where cmake.exe 2^>nul ^| findstr /r /c:"cmake"') do (
    echo   [OK] cmake ^(%%c^)
  )
)

:: git
where /q git.exe
if errorlevel 1 (
  echo   [MISSING] git
  echo            Install from https://git-scm.com/download/win or run: winget install Git.Git
  set /a MISSING+=1
) else (
  echo   [OK] git
)

:: 7-Zip
if exist "c:\Program Files\7-Zip\7z.exe" (
  echo   [OK] 7-Zip
) else (
  echo   [MISSING] 7-Zip
  echo            Install from https://www.7-zip.org/ or run: winget install 7zip.7zip
  set /a MISSING+=1
)

:: Inno Setup 6
if exist "c:\Program Files (x86)\Inno Setup 6\iscc.exe" (
  echo   [OK] Inno Setup 6
) else (
  echo   [MISSING] Inno Setup 6
  echo            Install from https://jrsoftware.org/isinfo.php or run: winget install JRSoftware.InnoSetup
  set /a MISSING+=1
)

:: OpenSSL 1.1.1d (32-bit only; x64 uses Qt's bundled SSL)
if "%ARCH%" == "x86" (
  if exist "c:\Program Files (x86)\openssl-1.1.1d-win32\libssl-1_1.dll" (
    echo   [OK] OpenSSL 1.1.1d win32
  ) else (
    echo   [MISSING] OpenSSL 1.1.1d win32
    echo            Download from https://slproweb.com/products/Win32OpenSSL.html
    set /a MISSING+=1
  )
) else (
  echo   [OK] OpenSSL ^(x64 uses Qt-bundled SSL^)
)

echo.
if !MISSING! gtr 0 (
  echo ==^> WARNING: !MISSING! dependencies are missing!
  echo See install instructions above for each missing dependency.
  echo.
  choice /c YN /m "Continue anyway?"
  if errorlevel 2 goto :eof
) else (
  echo ==^> All dependencies found.
)
echo.

rem Set up Visual Studio build environment
if defined VS_PATH (
  call "!VS_PATH!" %ARCH%
  if errorlevel 1 goto :fail
) else (
  echo WARNING: Visual Studio environment not set up. Build will likely fail.
)

if not defined VCToolsRedistDir (
  echo Visual Studio redistributable directory not found.
  goto :fail
)

set PATH=%QT%\bin;%PATH%

set "ROOT=%~dp0.."
set "BUILD=%~dp0..\build\build\release"
set /p VERSION=<"%ROOT%\VERSION"
set "VERSION_COMMIT=%VERSION%"

where /q git.exe
if "%ERRORLEVEL%" equ "0" (
  for /f "tokens=*" %%t in ('git.exe rev-parse --short HEAD') do (
    set COMMIT=%%t
  )
  if defined COMMIT set "VERSION_COMMIT=%VERSION%-!COMMIT!"
)

if "%ARCH%" == "x86" (
  set "TARGET=%~dp0\..\release\rclone-browser-%VERSION_COMMIT%-windows-32-bit"
  set "TARGET_EXE=%~dp0\..\release\rclone-browser-%VERSION_COMMIT%-windows-32-bit"
) else (
  set "TARGET=%~dp0\..\release\rclone-browser-%VERSION_COMMIT%-windows-64-bit"
  set "TARGET_EXE=%~dp0\..\release\rclone-browser-%VERSION_COMMIT%-windows-64-bit"
)

pushd "%ROOT%"
if errorlevel 1 goto :fail

if not exist release mkdir release

if exist "%TARGET%" rd /s /q "%TARGET%"
if exist "%TARGET%.zip" del "%TARGET%.zip"
if exist "%TARGET_EXE%.exe" del "%TARGET_EXE%.exe"

if exist build rd /s /q build
mkdir build
cd build

if "%ARCH%" == "x86" (
cmake -G "%CMAKEGEN%" -A Win32 -DCMAKE_CONFIGURATION_TYPES="Release" -DCMAKE_PREFIX_PATH="%QT%" ..
) else (
cmake -G "%CMAKEGEN%" -A x64 -DCMAKE_CONFIGURATION_TYPES="Release" -DCMAKE_PREFIX_PATH="%QT%" ..
)
if errorlevel 1 goto :fail_popd

cmake --build . --config Release
if errorlevel 1 goto :fail_popd
popd

mkdir "%TARGET%" 2>nul
if errorlevel 1 goto :fail

copy "%ROOT%\README.md" "%TARGET%\Readme.md"
if errorlevel 1 goto :fail
copy "%ROOT%\CHANGELOG.md" "%TARGET%\Changelog.md"
if errorlevel 1 goto :fail
copy "%ROOT%\LICENSE" "%TARGET%\License.txt"
if errorlevel 1 goto :fail
copy "%BUILD%\RcloneBrowser.exe" "%TARGET%"
if errorlevel 1 goto :fail

windeployqt.exe --no-translations --no-angle --no-compiler-runtime --no-svg "%TARGET%\RcloneBrowser.exe"
if errorlevel 1 goto :fail
if exist "%TARGET%\imageformats" rd /s /q "%TARGET%\imageformats"

rem include all MSVCruntime dlls
copy "%VCToolsRedistDir%\%ARCH%\%VCTOOLS_CRT%\msvcp140.dll" "%TARGET%\"
if errorlevel 1 goto :fail
copy "%VCToolsRedistDir%\%ARCH%\%VCTOOLS_CRT%\vcruntime140*.dll" "%TARGET%\"
if errorlevel 1 goto :fail

rem for Windows 32 bits build include relevant openssl libraries
if "%ARCH%" == "x86" (
copy "c:\Program Files (x86)\openssl-1.1.1d-win32\libssl-1_1.dll" "%TARGET%\"
if errorlevel 1 goto :fail
copy "c:\Program Files (x86)\openssl-1.1.1d-win32\libcrypto-1_1.dll" "%TARGET%\"
if errorlevel 1 goto :fail
)

(
echo [Paths]
echo Prefix = .
echo LibraryExecutables = .
echo Plugins = .
)>"%TARGET%\qt.conf"

rem https://www.7-zip.org/
rem create zip archive of all files
"c:\Program Files\7-Zip\7z.exe" a -mx=9 -r -tzip "%TARGET%.zip" "%TARGET%"
if errorlevel 1 goto :fail

rem create proper installer
rem Inno Setup installer by https://github.com/jrsoftware/issrc
rem in case user wants to install both 32bits and 64bits versions we need two AppId
rem 64bits ;AppId={{0AF9BF43-8D44-4AFF-AE60-6CECF1BF0D31}
rem 32bits ;AppId={{5644ED3A-6028-47C0-9796-29548EF7CEA3}
if "%ARCH%" == "x86" (
"c:\Program Files (x86)\Inno Setup 6"\iscc "/dMyAppVersion=%VERSION%" "/dMyAppId={{5644ED3A-6028-47C0-9796-29548EF7CEA3}" "/dMyAppDir=rclone-browser-%VERSION_COMMIT%-windows-32-bit" "/dMyAppArch=x86" /O"../release" /F"rclone-browser-%VERSION_COMMIT%-windows-32-bit" rclone-browser-win-installer.iss
) else (
"c:\Program Files (x86)\Inno Setup 6"\iscc "/dMyAppVersion=%VERSION%" "/dMyAppId={{0AF9BF43-8D44-4AFF-AE60-6CECF1BF0D31}" "/dMyAppDir=rclone-browser-%VERSION_COMMIT%-windows-64-bit" "/dMyAppArch=x64" /O"../release" /F"rclone-browser-%VERSION_COMMIT%-windows-64-bit" rclone-browser-win-installer.iss
)
if errorlevel 1 goto :fail

goto :eof

:fail_popd
popd

:fail
echo Build failed.
exit /b 1
