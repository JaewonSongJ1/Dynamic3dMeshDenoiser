@echo off
REM Development Build Script for Dynamic 3D Mesh Denoiser
REM Author: Jaewon Song, Dexter Studios
REM This builds with dynamic linking for faster development iteration

echo ============================================================================
echo DYNAMIC 3D MESH DENOISER - DEVELOPMENT BUILD
echo ============================================================================
echo Setting up development build environment...

REM Check if VCPKG_ROOT environment variable is set
if "%VCPKG_ROOT%"=="" (
    echo Error: VCPKG_ROOT environment variable is not set
    echo Please set VCPKG_ROOT to your vcpkg installation directory
    echo Example: set VCPKG_ROOT=C:\vcpkg
    pause
    exit /b 1
)

REM Check if vcpkg exists at the specified path
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo Error: vcpkg not found at %VCPKG_ROOT%\
    echo Please check your VCPKG_ROOT environment variable
    echo Current VCPKG_ROOT: %VCPKG_ROOT%
    pause
    exit /b 1
)

echo Using vcpkg from: %VCPKG_ROOT%

REM Create build directory
if not exist "build_dev" mkdir build_dev
cd build_dev

echo Configuring with CMake (Development - Dynamic Linking)...
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building project (Debug)...
cmake --build . --config Debug

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo ============================================================================
echo DEVELOPMENT BUILD COMPLETED SUCCESSFULLY!
echo ============================================================================
echo Executables location: build_dev\Debug\
echo - BilateralMeshDenoiser.exe
echo - TemporalMeshDenoiser.exe
echo.
echo Note: This is a development build with dynamic linking.
echo For deployment, use build_release.bat instead.
echo ============================================================================
pause