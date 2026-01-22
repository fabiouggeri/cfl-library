@echo off

set COMPILER=%1

if "%COMPILER%"=="" (
    echo Usage: %0 [mingw^|msvc] [install]
    exit /b 1
)

if "%COMPILER%"=="mingw" (
    set GENERATOR="MinGW Makefiles"
    set BUILD_DIR=build-mingw
    set CONAN_PROFILE=./.conan/mingw_profile
) else if "%COMPILER%"=="msvc" (
    set GENERATOR="Visual Studio 16 2019"
    set BUILD_DIR=build-msvc
    set CONAN_PROFILE=./.conan/msvc19_profile
) else (
    echo Unsupported compiler: %COMPILER%. Use 'mingw' or 'msvc'.
    exit /b 1
)

echo Building with %COMPILER% in directory %BUILD_DIR%...

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

:: Conan package creation
if "%2"=="install" (
    echo Creating Conan package...
    conan create . --build=missing --profile=%CONAN_PROFILE%
    if %ERRORLEVEL% neq 0 (
        echo Conan package creation failed
        exit /b %ERRORLEVEL%
    )
    echo Package creation for %COMPILER% completed successfully.
    exit /b 0
)

:: Conan install for local build
conan install . --output-folder=%BUILD_DIR% --build=missing --profile=%CONAN_PROFILE%
if %ERRORLEVEL% neq 0 (
    echo Conan install failed
    exit /b %ERRORLEVEL%
)

cd %BUILD_DIR%

:: Configure
cmake .. -G %GENERATOR%
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed
    cd ..
    exit /b %ERRORLEVEL%
)

:: Build
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed
    cd ..
    exit /b %ERRORLEVEL%
)

:: Package
echo Generating ZIP package...
cpack -G ZIP
if %ERRORLEVEL% neq 0 (
    echo Packaging failed
    cd ..
    exit /b %ERRORLEVEL%
)

cd ..

echo Build and packaging for %COMPILER% completed successfully.
