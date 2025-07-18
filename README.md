# Dynamic 3D Mesh Denoiser

**High-Performance 4D Mesh Denoising Toolkit for Production Pipelines**

*Professional-grade temporal mesh denoising for standalone processing*

Dynamic 3D Mesh Denoiser is a high-performance C++ toolkit designed for denoising temporal mesh sequences from 4D scanning, motion capture, and simulation data. This standalone solution processes Alembic (.abc) files directly, providing professional-grade denoising capabilities for production pipelines.

## 🌟 Key Features

- **🚀 High Performance**: Optimized C++ implementation with multi-threading support
- **🎛️ Multiple Algorithms**: Bilateral filtering and temporal smoothing with extensible architecture
- **📁 Standalone Processing**: Direct Alembic file processing, no external dependencies
- **🔧 Production Ready**: Handles large-scale 4D datasets (24K+ vertices, 1600+ frames)
- **⚡ Multi-threaded**: OpenMP support for parallel processing
- **🎯 Adaptive Processing**: Motion-aware denoising with edge preservation
- **💾 Memory Efficient**: Optimized for large temporal datasets
- **🛠️ Command-Line Interface**: Easy integration into production pipelines

## 🎯 Use Cases

- **4D Face Scanning**: Remove noise from high-resolution facial capture data
- **Motion Capture Cleanup**: Smooth temporal artifacts in mocap sequences
- **Simulation Post-Processing**: Clean up cloth, fluid, and soft-body simulations
- **Animation Polishing**: Remove jitter from keyframe animation
- **Research & Development**: Prototype new denoising algorithms

## 📁 Project Structure

```
dynamic_3dmesh_denoiser/
│
├── 📂 src/                           # Core engine source code
│   ├── BilateralMeshDenoiser.cpp     # Bilateral temporal filtering implementation ⭐
│   └── TemporalMeshDenoiser.cpp      # Temporal smoothing implementation ⭐ NEW
│
├── 📂 build/                         # Build directory (generated)
│   └── Release/
│       ├── BilateralMeshDenoiser.exe # Bilateral filtering executable
│       └── TemporalMeshDenoiser.exe  # Temporal smoothing executable
│
├── 📂 examples/                      # Example files and test data
│   ├── sample_input.abc              # Sample input mesh sequence
│   └── sample_output.abc             # Expected output after denoising
│
├── 📄 CMakeLists.txt                 # CMake build configuration
├── 📄 build.bat                      # Windows build script
└── 📄 README.md                      # This file
```

## 🔬 Denoising Algorithms

| Algorithm | Status | Performance | Quality | Use Case |
|-----------|--------|-------------|---------|----------|
| **Bilateral Temporal Filter** | ✅ Production Ready | Medium | Excellent | General-purpose, production workflows |
| **Temporal Smoothing Filter** | ✅ Production Ready | Fast | Good | Quick previews, light denoising |
| Advanced Edge-Preserving | 📋 Planned | Medium | Superior | High-detail preservation scenarios |

### 1. Bilateral Temporal Filter (`BilateralMeshDenoiser.exe`)
- **Advanced algorithm** with motion-aware denoising and edge preservation
- **Adaptive windowing** based on motion analysis
- **Best for**: Heavy noise reduction, final production quality
- **Performance**: 24K vertices × 1620 frames in ~2.4 seconds

### 2. Temporal Smoothing Filter (`TemporalMeshDenoiser.exe`) ⭐ **NEW**
- **Simple temporal averaging** with linear or gaussian weighting
- **FPS-aware auto window sizing**: Automatically detects frame rate and adjusts window size
- **Smart defaults**: 24fps→window 3, 60fps→window 5, 120fps+→window 7
- **Best for**: Quick denoising, preview workflows, light touch-ups
- **Performance**: 24K vertices × 1620 frames in ~0.8 seconds

🚀 Installation & Build
Prerequisites

Windows 10/11 (64-bit) or Linux/macOS
Visual Studio 2022 (Windows) or GCC 7+/Clang 8+ (Linux/macOS)
CMake 3.16+
vcpkg (for dependency management)
Git

Environment Setup
Important: Set VCPKG_ROOT environment variable before building.
Method 1: System Environment Variable (Recommended)

Windows + R → sysdm.cpl → Enter
"Advanced" Tab → "Environment Variables"
Add System Variable: VCPKG_ROOT=C:\vcpkg

Method 2: Current Session
bashset VCPKG_ROOT=C:\vcpkg
Build Options
Development Build (Fast, Dynamic Linking)
bash# Quick build for development/debugging
# Creates smaller exe but requires DLL files
build_dev.bat
Output: build/dev/bin/Debug/BilateralMeshDenoiser.exe + DLL files
Use case: Development, debugging, algorithm testing
Release Build (Standalone, Static Linking)
bash# Standalone executable for deployment
# Takes longer to build but creates portable exe
build_release.bat
Output: deploy/BilateralMeshDenoiser.exe (standalone, ~30-50MB)
Use case: Production deployment, distribution to artists
Quick Start (Windows)
bash# 1. Clone repository
git clone https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser.git
cd Dynamic3dMeshDenoiser

# 2. Setup vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
set VCPKG_ROOT=C:\vcpkg

# 3. Development build (fast)
build_dev.bat
Production Deployment
bash# 1. Clone repository (on build machine)
git clone https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser.git
cd Dynamic3dMeshDenoiser

# 2. Release build (standalone)
build_release.bat

# 3. Distribute the deploy/ folder to all workstations
# No additional setup required on target machines
Manual Installation (Advanced)
Windows
bash# 1. Install vcpkg dependencies
%VCPKG_ROOT%\vcpkg install alembic:x64-windows        # For development
%VCPKG_ROOT%\vcpkg install alembic:x64-windows-static # For deployment

# 2. Configure build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
         -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build . --config Release
Linux/macOS
bash# 1. Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install cmake build-essential libalembic-dev

# 2. Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
Troubleshooting
Common Issues
❌ "VCPKG_ROOT environment variable is not set"
bash# Solution: Set environment variable
set VCPKG_ROOT=C:\vcpkg
❌ "Alembic not found"
bash# Solution: Install dependencies
%VCPKG_ROOT%\vcpkg install alembic:x64-windows
%VCPKG_ROOT%\vcpkg install alembic:x64-windows-static
❌ "Build failed - Runtime library mismatch"
bash# Solution: Clean build
build_release.bat clean
build_release.bat
❌ "Visual Studio not found"

Install Visual Studio 2022 with C++ development tools
Or use Visual Studio Build Tools 2022

Build Verification
bash# Test development build
build\dev\bin\Debug\BilateralMeshDenoiser.exe -h

# Test release build  
deploy\BilateralMeshDenoiser.exe -h
Build Performance
Build TypeTimeOutput SizeDependenciesDevelopment~30 seconds~2MB + DLLsRequires vcpkgRelease~5 minutes~45MBStandalone

## 🚀 Usage Examples

### Basic Usage
```bash
# Bilateral filtering (production quality)
BilateralMeshDenoiser.exe input.abc output.abc

# Temporal smoothing (fast preview)
TemporalMeshDenoiser.exe input.abc output.abc
```

### Frame Range Processing
```bash
# Process specific frame range (1-based Maya frames)
BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100
TemporalMeshDenoiser.exe input.abc output.abc --maya-range 1 100

# Process specific Alembic frame range (0-based)
BilateralMeshDenoiser.exe input.abc output.abc --sf 0 --ef 99
TemporalMeshDenoiser.exe input.abc output.abc --start-frame 0 --end-frame 99
```

### Temporal Denoiser Specific Options
```bash
# Auto window size based on detected FPS (default)
TemporalMeshDenoiser.exe input.abc output.abc --maya-range 1 100

# Manual window size (overrides auto-detection)
TemporalMeshDenoiser.exe input.abc output.abc --window 7

# Gaussian weighting (smoother for larger windows)
TemporalMeshDenoiser.exe input.abc output.abc --weight gaussian --sigma 1.5

# Linear weighting (faster, good for small windows)
TemporalMeshDenoiser.exe input.abc output.abc --weight linear
```

### Advanced Bilateral Denoiser Options
```bash
# Ultra-strong denoising (maximum smoothing)
BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100 \
    --window 15 --sigma-temporal 5.0 --sigma-spatial 0.35

# Medium denoising (balanced)
BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100 \
    --window 9 --sigma-temporal 2.5 --sigma-spatial 0.15

# Subtle denoising (detail preservation)
BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100 \
    --window 7 --sigma-temporal 1.5 --sigma-spatial 0.08
```

## ⚙️ Parameter Guidelines

### Temporal Denoiser Parameters
| Parameter | Range | Default | Effect |
|-----------|-------|---------|--------|
| `--window` | 3-9 | Auto (FPS-based) | Temporal smoothing strength |
| `--weight` | linear/gaussian | linear | Weighting function |
| `--sigma` | 0.5-3.0 | 1.0 | Gaussian standard deviation |

### FPS-Based Auto Window Sizing
The Temporal Denoiser automatically detects FPS and adjusts window size:
- **24fps** (cinema): window 3 (~0.125s temporal coverage)
- **30fps** (video): window 3 (~0.100s temporal coverage)  
- **60fps** (high-fps): window 5 (~0.083s temporal coverage)
- **120fps+** (ultra): window 7 (~0.058s temporal coverage)

*Manual `--window` setting overrides auto-detection*

### Bilateral Denoiser Parameters
| Parameter | Range | Default | Recommendation |
|-----------|-------|---------|----------------|
| `--window` | 5-15 | 15 | 15 (default), 9 (medium), 5 (subtle) |
| `--sigma-temporal` | 1.0-5.0 | 4.0 | 4.0 (default), 2.5 (medium), 1.0 (sharp) |
| `--sigma-spatial` | 0.05-0.35 | 0.25 | 0.25 (default), 0.15 (medium), 0.05 (preserve detail) |
| `--motion-thresh` | 0.02-0.15 | 0.1 | 0.1 (default), 0.05 (sensitive), 0.15 (relaxed) |
| `--edge-thresh` | 0.05-0.2 | 0.15 | 0.15 (default), 0.1 (preserve), 0.2 (smooth) |

## 📊 Performance Benchmarks

**Test Case**: 24,049 vertices × 1,620 frames (4D facial scan)

| Algorithm | Processing Time | Memory Usage | Throughput |
|-----------|----------------|--------------|------------|
| **Temporal Denoiser** | ~0.8 seconds | ~45 MB | Very Fast |
| **Bilateral Denoiser** | ~2.4 seconds | ~680 MB | Production Grade |

## 🔬 Algorithm Details

### Temporal Smoothing Algorithm
Simple and effective temporal averaging with configurable weighting:
- **Linear weighting**: Higher weight at center, linearly decreasing to edges
- **Gaussian weighting**: Smooth falloff based on temporal distance
- **Normalized weights**: Ensures mesh scale preservation
- **FPS awareness**: Automatically adjusts temporal window based on frame rate

### Bilateral Temporal Algorithm
Advanced denoising combining spatial and temporal information:
- **Adaptive Windowing**: Dynamic window sizing based on motion analysis
- **Edge Preservation**: Motion-aware edge detection prevents over-smoothing
- **Multi-threaded Processing**: OpenMP parallelization for vertex-level operations
- **Memory Optimization**: Efficient frame loading and processing

## 🤝 Contributing

**Jaewon Song**  
R&D Director, Dexter Studios
- 🏢 Company: [Dexter Studios](http://www.dexterstudios.com/)
- 🎓 Expertise: Computer Graphics, 3D Pipeline Development, High-Performance Computing
- 🔬 Research Focus: Digital human, facial performance capture & retargeting, dynamic 3D scanning, AI-driven content creation
- 📧 Contact: [jaewon.song@dexterstudios.com](mailto:jaewon.song@dexterstudios.com)
- 💼 LinkedIn: [linkedin.com/in/jaewonsongj1](https://www.linkedin.com/in/jaewonsongj1/)

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

We welcome contributions from the community! Whether you're from a VFX studio, research institution, or independent developer:

- **Issues**: Report bugs or request features via [GitHub Issues](https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser/issues)
- **Discussions**: Join technical discussions in [GitHub Discussions](https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser/discussions)

### Contributing Guidelines
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-algorithm`)
3. Commit your changes (`git commit -m 'Add amazing denoising algorithm'`)
4. Push to the branch (`git push origin feature/amazing-algorithm`)
5. Open a Pull Request

## 🙏 Acknowledgments

- **Minyeong Jeong**: Dynamic 3D mesh device development and hardware integration

---

**Professional 4D Mesh Denoising Made Simple** 🎬✨

*Empowering creators with production-grade tools*

Built with ❤️ by [Dexter Studios](https://www.dexterstudios.com/) R&D Team