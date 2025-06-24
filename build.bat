@echo off
echo Setting up build environment...

set VCPKG_ROOT=C:\Users\jaewon.song\source\repos\vcpkg

if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 17 2022" -A x64

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ✅ Build completed successfully!
echo 📁 Executable: build\Release\BilateralMeshDenoiser.exe
echo.
echo For help: BilateralMeshDenoiser.exe -h

pause