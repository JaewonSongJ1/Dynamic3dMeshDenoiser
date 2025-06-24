# Dynamic 3D Mesh Denoiser - Development Context

## Project Overview
- **Repository**: https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser
- **Purpose**: High-performance 4D mesh denoising toolkit for production pipelines
- **Target**: Process Alembic (.abc) files with temporal mesh sequences from 4D scanning
- **Author**: Jaewon Song, R&D Director at Dexter Studios
- **Collaborator**: Minyeong Jeong (Dynamic 3D mesh device development)

## Technical Stack
- **Language**: C++17
- **Build System**: CMake + vcpkg
- **Dependencies**: Alembic 1.8+, OpenMP (multi-threading)
- **Platform**: Windows (primary), Linux/macOS (supported)
- **Development Environment**: Visual Studio 2022, vcpkg for dependency management

## Current Implementation Status

### ✅ COMPLETED: Bilateral Temporal Filter
- **File**: `src/BilateralMeshDenoiser.cpp`
- **Algorithm**: Bilateral temporal filtering with adaptive windowing
- **Features**:
  - Motion-aware denoising with edge preservation
  - Multi-threaded processing (OpenMP)
  - Command-line interface with named parameters
  - Maya frame range conversion (1-based → 0-based)
  
### Algorithm Parameters (Current Defaults - Optimized for Strong Denoising)
```
--window 15              # Temporal window size (strong denoising)
--sigma-temporal 4.0     # Temporal weight falloff (wide range)  
--sigma-spatial 0.25     # Spatial smoothing strength (strong)
--motion-thresh 0.1      # Motion threshold for adaptive windowing
--edge-thresh 0.15       # Edge preservation threshold (relaxed)
```

### Performance Benchmarks
- **Test Case**: 24,049 vertices × 100 frames
- **Processing Time**: ~0.15 seconds (multi-threaded)
- **Memory Usage**: ~45 MB
- **Large Dataset**: 1620 frames in ~2.4 seconds

## Build & Usage Instructions

### Build Commands
```bash
# Windows (automated)
build.bat

# Manual build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Usage Examples
```bash
# Basic usage (strong denoising default)
BilateralMeshDenoiser.exe input.abc output.abc

# Maya frame range (1-100 becomes Alembic 0-99)  
BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100

# Custom parameters
BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100 --window 9 --sigma-spatial 0.15

# Help
BilateralMeshDenoiser.exe -h
```

## Development History & Context

### Original Problem
- 4D facial scanning data contained significant noise
- Maya-based keyframe denoising workflow was extremely slow (90%+ of processing time)
- Needed standalone solution for production pipeline integration

### Algorithm Evolution
1. **Started**: Python bilateral mesh denoiser (worked but slow)
2. **Developed**: C++ implementation with massive performance gains
3. **Optimized**: Default parameters tuned for heavy 4D scan noise
4. **Result**: Production-ready tool with excellent quality/speed balance

### Key Design Decisions
- **Strong defaults**: Default parameters provide aggressive denoising suitable for 4D scan noise
- **Parameter flexibility**: Can be tuned from subtle to extreme denoising
- **Frame indexing**: Clear distinction between Maya (1-based) and Alembic (0-based) frames
- **Extensible architecture**: Ready for additional algorithms

## Next Development Phase

### 🚧 IMMEDIATE NEXT: Linear Interpolation Filter
- **Goal**: Simple, fast alternative algorithm for light denoising
- **Implementation**: Create second algorithm option in same executable
- **Command**: `--algorithm linear` or similar parameter
- **Use Case**: Quick previews, light touch-ups, comparison baseline

### Technical Approach for Linear Filter
1. Add algorithm selection parameter to command line
2. Create `LinearInterpolationDenoiser` class
3. Implement simple temporal interpolation between keyframes
4. Integrate into main application with algorithm switching
5. Update help documentation

### Future Algorithms (Planned)
- Advanced edge-preserving filters
- Gaussian temporal smoothing
- Possible ML-based approaches (research phase)

## Development Environment Setup

### Prerequisites
- Visual Studio 2019+ with C++ development tools
- CMake 3.16+
- vcpkg package manager
- Git

### vcpkg Setup (Windows)
```bash
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat  
C:\vcpkg\vcpkg integrate install
C:\vcpkg\vcpkg install alembic:x64-windows
```

### File Structure
```
Dynamic3dMeshDenoiser/
├── README.md                    # Complete documentation
├── CMakeLists.txt              # Build configuration  
├── build.bat                   # Windows build script
├── .gitignore                  # Build artifacts excluded
├── src/
│   └── BilateralMeshDenoiser.cpp   # Main implementation
└── build/                      # Generated build files (gitignored)
```

## Test Data Information
- **Primary Test File**: `twf_nsk_cache.abc` (1620 frames, 24,049 vertices, facial animation)
- **Location**: `C:\Users\jaewon.song\Documents\maya\projects\DeepFaceSolver\cache\alembic\`
- **Test Range**: Typically use frames 1-100 for development testing

## Key Learnings & Notes
- Default parameters work well for heavy 4D scan noise
- Maya users should use `--maya-range` for intuitive frame specification  
- OpenMP provides significant performance boost on multi-core systems
- UTF-8 encoding required for source files to avoid MSVC warnings
- vcpkg greatly simplifies Alembic dependency management on Windows

## Communication Protocol for Future Development
**To continue development in new conversation:**
1. Provide this context file
2. Mention GitHub repository: https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser
3. Specify current development goal (e.g., "implement linear interpolation filter")
4. All build/usage information is preserved in README.md and code comments