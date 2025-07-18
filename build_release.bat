@echo off
REM Release Build Script for Dynamic 3D Mesh Denoiser
REM Author: Jaewon Song, Dexter Studios
REM This builds with static linking for standalone deployment

echo ============================================================================
echo DYNAMIC 3D MESH DENOISER - RELEASE BUILD
echo ============================================================================
echo Setting up release build environment for deployment...

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

REM Install static libraries if not already installed
echo Checking and installing static dependencies...
"%VCPKG_ROOT%\vcpkg.exe" install alembic:x64-windows-static
if %ERRORLEVEL% neq 0 (
    echo Failed to install static dependencies!
    pause
    exit /b 1
)

REM Create build directory
if not exist "build_release" mkdir build_release
cd build_release

echo Configuring with CMake (Release - Static Linking)...
cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building project (Release - Static)...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo ============================================================================
echo CREATING DEPLOYMENT PACKAGE...
echo ============================================================================

REM Create deploy directory
cd ..
if exist "deploy" rmdir /s /q deploy
mkdir deploy

REM Copy executables
echo Copying executables...
copy "build_release\Release\BilateralMeshDenoiser.exe" "deploy\" >nul
copy "build_release\Release\TemporalMeshDenoiser.exe" "deploy\" >nul

REM Create simple usage guide
echo Creating usage guide...
echo Dynamic 3D Mesh Denoiser - Usage Guide > deploy\how_to_use.txt
echo ============================================== >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo BASIC USAGE: >> deploy\how_to_use.txt
echo ------------ >> deploy\how_to_use.txt
echo BilateralMeshDenoiser.exe input.abc output.abc >> deploy\how_to_use.txt
echo TemporalMeshDenoiser.exe input.abc output.abc >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo WITH FRAME RANGE: >> deploy\how_to_use.txt
echo ----------------- >> deploy\how_to_use.txt
echo BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100 >> deploy\how_to_use.txt
echo TemporalMeshDenoiser.exe input.abc output.abc --maya-range 1 100 >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo ALGORITHMS: >> deploy\how_to_use.txt
echo ----------- >> deploy\how_to_use.txt
echo - BilateralMeshDenoiser: Advanced denoising with edge preservation >> deploy\how_to_use.txt
echo   Best for: Production quality, heavy noise reduction >> deploy\how_to_use.txt
echo   Speed: Medium (~2.4s for 1620 frames) >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo - TemporalMeshDenoiser: Fast temporal smoothing >> deploy\how_to_use.txt
echo   Best for: Quick previews, light denoising >> deploy\how_to_use.txt
echo   Speed: Fast (~0.8s for 1620 frames) >> deploy\how_to_use.txt
echo   Features: Auto window sizing based on FPS >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo COMMON OPTIONS: >> deploy\how_to_use.txt
echo --------------- >> deploy\how_to_use.txt
echo --maya-range ^<start^> ^<end^>     Maya frame range (1-based) >> deploy\how_to_use.txt
echo --start-frame ^<frame^>          Start frame (0-based) >> deploy\how_to_use.txt
echo --end-frame ^<frame^>            End frame (0-based) >> deploy\how_to_use.txt
echo --quiet                       Disable verbose output >> deploy\how_to_use.txt
echo --help                        Show detailed help >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo TEMPORAL DENOISER OPTIONS: >> deploy\how_to_use.txt
echo -------------------------- >> deploy\how_to_use.txt
echo --window ^<size^>               Window size (auto-detected from FPS) >> deploy\how_to_use.txt
echo --weight linear^|gaussian      Weight function (default: linear) >> deploy\how_to_use.txt
echo --sigma ^<value^>               Gaussian standard deviation (default: 1.0) >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo EXAMPLES: >> deploy\how_to_use.txt
echo --------- >> deploy\how_to_use.txt
echo # Quick temporal denoising (auto FPS detection) >> deploy\how_to_use.txt
echo TemporalMeshDenoiser.exe scan.abc clean.abc --maya-range 1 100 >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo # Production quality bilateral denoising >> deploy\how_to_use.txt
echo BilateralMeshDenoiser.exe scan.abc final.abc --maya-range 1 100 >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo # Custom temporal denoising >> deploy\how_to_use.txt
echo TemporalMeshDenoiser.exe scan.abc smooth.abc --window 7 --weight gaussian >> deploy\how_to_use.txt
echo. >> deploy\how_to_use.txt
echo Author: Jaewon Song (Dexter Studios) >> deploy\how_to_use.txt
echo GitHub: https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser >> deploy\how_to_use.txt
echo Build Date: %date% %time% >> deploy\how_to_use.txt

echo ============================================================================
echo RELEASE BUILD COMPLETED SUCCESSFULLY!
echo ============================================================================
echo.
echo 📦 DEPLOYMENT PACKAGE READY: deploy\
echo.
echo 📁 Package Contents:
echo   deploy\
echo   ├── BilateralMeshDenoiser.exe     (Standalone executable)
echo   ├── TemporalMeshDenoiser.exe      (Standalone executable)
echo   └── how_to_use.txt                (Usage guide)
echo.
echo 🚀 Ready for Distribution:
echo   - Zip the 'deploy' folder for sharing
echo   - No additional DLLs or dependencies required
echo   - Works on any Windows 10+ system
echo.
echo 💡 Quick Test:
echo   cd deploy
echo   BilateralMeshDenoiser.exe --help
echo   TemporalMeshDenoiser.exe --help
echo.
echo ============================================================================
pause