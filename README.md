# Dynamic 3D Mesh Denoiser 🎬

<div align="center">

![C++](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=cplusplus)
![Alembic](https://img.shields.io/badge/Alembic-1.8+-green?style=for-the-badge)
![OpenMP](https://img.shields.io/badge/OpenMP-Multi--threading-orange?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey?style=for-the-badge)

**High-Performance 4D Mesh Denoising Toolkit for Production Pipelines**

*Professional-grade temporal mesh denoising for standalone processing*

</div>

---

## 🎯 Introduction

**Dynamic 3D Mesh Denoiser** is a high-performance C++ toolkit designed for denoising temporal mesh sequences from 4D scanning, motion capture, and simulation data. This standalone solution processes Alembic (.abc) files directly, providing professional-grade denoising capabilities for production pipelines.

### ✨ Key Features

- 🚀 **High Performance**: Optimized C++ implementation with multi-threading support
- 🎛️ **Multiple Algorithms**: Bilateral filtering with extensible architecture for future algorithms
- 📁 **Standalone Processing**: Direct Alembic file processing, no external dependencies
- 🔧 **Production Ready**: Handles large-scale 4D datasets (24K+ vertices, 1600+ frames)
- ⚡ **Multi-threaded**: OpenMP support for parallel processing
- 🎯 **Adaptive Processing**: Motion-aware denoising with edge preservation
- 💾 **Memory Efficient**: Optimized for large temporal datasets
- 🛠️ **Command-Line Interface**: Easy integration into production pipelines

### 🎬 Use Cases

- **4D Face Scanning**: Remove noise from high-resolution facial capture data
- **Motion Capture Cleanup**: Smooth temporal artifacts in mocap sequences  
- **Simulation Post-Processing**: Clean up cloth, fluid, and soft-body simulations
- **Animation Polishing**: Remove jitter from keyframe animation
- **Research & Development**: Prototype new denoising algorithms

---

## 📁 Project Structure

```
dynamic_3dmesh_denoiser/
│
├── 📂 src/                          # Core engine source code
│   └── BilateralMeshDenoiser.cpp        # Bilateral temporal filtering implementation ⭐
│
├── 📂 build/                        # Build directory (generated)
│   └── Release/
│       └── BilateralMeshDenoiser.exe   # Compiled executable
│
├── 📂 examples/                     # Example files and test data
│   ├── sample_input.abc                # Sample input mesh sequence
│   └── sample_output.abc               # Expected output after denoising
│
├── 📂 docs/                         # Documentation
│   ├── algorithm_guide.md              # Algorithm theory and parameters
│   ├── integration_guide.md            # Pipeline integration guide
│   └── performance_benchmarks.md       # Performance analysis
│
├── 📄 CMakeLists.txt                # CMake build configuration
├── 📄 build.bat                     # Windows build script
├── 📄 requirements.txt              # Dependencies (vcpkg packages)
└── 📄 README.md                     # This file
```

### 🧮 Current Algorithms

| Algorithm | Status | Performance | Quality | Use Case |
|-----------|--------|-------------|---------|----------|
| **Bilateral Temporal Filter** | ✅ Production Ready | Fast | Excellent | General-purpose, production workflows |
| **Linear Interpolation Filter** | 🚧 In Development | Very Fast | Good | Quick previews, light denoising |
| **Advanced Edge-Preserving** | 📋 Planned | Medium | Superior | High-detail preservation scenarios |

---

## 🚀 Installation

### 📋 Prerequisites

- **Visual Studio 2019+** (Windows) or **GCC 7+** / **Clang 8+** (Linux/macOS)
- **CMake 3.16+**
- **vcpkg** (for dependency management)
- **Git**

### 💾 Quick Installation (Windows)

```bash
# 1. Clone the repository
git clone https://github.com/JaewonSongJ1/dynamic_3dmesh_denoiser.git
cd dynamic_3dmesh_denoiser

# 2. Setup vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg integrate install

# 3. Install dependencies
C:\vcpkg\vcpkg install alembic:x64-windows

# 4. Build the project
build.bat

# 5. Test installation
cd build\Release
BilateralMeshDenoiser.exe -h
```

### 🐧 Linux/macOS Installation

```bash
# 1. Clone repository
git clone https://github.com/JaewonSongJ1/dynamic_3dmesh_denoiser.git
cd dynamic_3dmesh_denoiser

# 2. Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install cmake build-essential libalembic-dev

# 3. Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 4. Test
./BilateralMeshDenoiser -h
```

---

## 🎮 Usage

### Basic Denoising

```bash
# Default strong denoising (recommended for 4D scans)
BilateralMeshDenoiser input.abc output.abc

# Process specific Maya frame range (1-100 → Alembic 0-99)
BilateralMeshDenoiser input.abc output.abc --maya-range 1 100

# Process specific Alembic frame range
BilateralMeshDenoiser input.abc output.abc --sf 0 --ef 99
```

### 🎛️ Parameter Tuning

```bash
# Ultra-strong denoising (maximum smoothing)
BilateralMeshDenoiser input.abc output.abc --maya-range 1 100 \
  --window 15 --sigma-temporal 5.0 --sigma-spatial 0.35

# Medium denoising (balanced)
BilateralMeshDenoiser input.abc output.abc --maya-range 1 100 \
  --window 9 --sigma-temporal 2.5 --sigma-spatial 0.15

# Subtle denoising (detail preservation)
BilateralMeshDenoiser input.abc output.abc --maya-range 1 100 \
  --window 7 --sigma-temporal 1.5 --sigma-spatial 0.08

# Custom edge-sensitive denoising
BilateralMeshDenoiser input.abc output.abc --maya-range 1 100 \
  --edge-thresh 0.08 --motion-thresh 0.05
```

### 📊 Performance Monitoring

```bash
# Quiet mode (pipeline integration)
BilateralMeshDenoiser input.abc output.abc --maya-range 1 100 --quiet

# Verbose mode (development/debugging)
BilateralMeshDenoiser input.abc output.abc --maya-range 1 100
```

### 🎯 Parameter Guidelines

| Parameter | Range | Effect | Recommendation |
|-----------|-------|--------|----------------|
| `--window` | 5-15 | Temporal smoothing strength | 15 (default), 9 (medium), 5 (subtle) |
| `--sigma-temporal` | 1.0-5.0 | Temporal weight falloff | 4.0 (default), 2.5 (medium), 1.0 (sharp) |
| `--sigma-spatial` | 0.05-0.35 | Spatial smoothing strength | 0.25 (default), 0.15 (medium), 0.05 (preserve detail) |
| `--motion-thresh` | 0.02-0.15 | Motion sensitivity | 0.1 (default), 0.05 (sensitive), 0.15 (relaxed) |
| `--edge-thresh` | 0.05-0.2 | Edge preservation | 0.15 (default), 0.1 (preserve), 0.2 (smooth) |

---

## 🔬 Algorithm Details

### Bilateral Temporal Filtering

Our bilateral temporal filter combines spatial and temporal information to achieve superior denoising while preserving important details:

- **Adaptive Windowing**: Dynamic window sizing based on motion analysis
- **Edge Preservation**: Motion-aware edge detection prevents over-smoothing
- **Multi-threaded Processing**: OpenMP parallelization for vertex-level operations
- **Memory Optimization**: Efficient frame loading and processing

```cpp
// Core algorithm pseudo-code
for each frame:
    analyze_motion_magnitude()
    calculate_adaptive_window_size()
    
    for each vertex:
        spatial_weight = exp(-spatial_distance² / (2 * sigma_spatial²))
        temporal_weight = exp(-temporal_distance² / (2 * sigma_temporal²))
        edge_factor = motion_aware_edge_detection()
        
        final_weight = spatial_weight * temporal_weight * edge_factor
        filtered_position = weighted_average(neighboring_frames)
```

### Performance Benchmarks

**Test Case**: 24,049 vertices × 100 frames (4D facial scan sample)

| Configuration | Processing Time | Memory Usage |
|---------------|----------------|--------------|
| **Single Thread** | 0.25 seconds | ~45 MB |
| **Multi-threaded (OpenMP)** | 0.15 seconds | ~45 MB |
| **Large Dataset (1620 frames)** | ~2.4 seconds | ~680 MB |

---

## 🛠️ Development & Extension

### Adding New Algorithms

1. **Create Algorithm Class**
   ```cpp
   class NewDenoiserAlgorithm : public BaseDenoiser {
   public:
       bool processFile(const std::string& input, const std::string& output) override;
   private:
       // Algorithm-specific implementation
   };
   ```

2. **Integrate into Main**
   ```cpp
   // Add to main.cpp parameter parsing
   if (algorithm == "new_algorithm") {
       NewDenoiserAlgorithm denoiser;
       return denoiser.processFile(input, output);
   }
   ```

3. **Test & Benchmark**
   ```bash
   # Build and test new algorithm
   mkdir build && cd build
   cmake .. && make
   ./YourNewDenoiser input.abc output.abc --algorithm new_algorithm
   ```

### 🔧 Build Customization

```cmake
# CMakeLists.txt - Add custom options
option(ENABLE_CUDA "Enable CUDA acceleration" OFF)
option(ENABLE_PROFILING "Enable performance profiling" OFF)

if(ENABLE_CUDA)
    find_package(CUDA REQUIRED)
    target_link_libraries(BilateralMeshDenoiser ${CUDA_LIBRARIES})
endif()
```

---

## 📈 Roadmap

### 🚀 Next Release (v1.1)
- [ ] **Linear Interpolation Filter**: Simple temporal interpolation-based denoising
- [ ] **Algorithm Comparison Tools**: Side-by-side quality and performance analysis
- [ ] **Batch Processing**: Multi-file processing capabilities

### 🔬 Future Development
- [ ] **Advanced Edge-Preserving Filters**: Improved detail preservation algorithms
- [ ] **Performance Optimization**: Further speed and memory improvements
- [ ] **Extended File Format Support**: Support for additional mesh formats

---

## 👨‍💼 Author

**Jaewon Song**  
*R&D Director, Dexter Studios*

- 🏢 **Company**: [Dexter Studios](http://www.dexterstudios.com/)
- 🎓 **Expertise**: Computer Graphics, 3D Pipeline Development, High-Performance Computing
- 🔬 **Research Focus**: Digital human, facial performance capture & retargeting, dynamic 3D scanning, AI-driven content creation
- 📧 **Contact**: [jaewon.song@dexterstudios.com](mailto:jaewon.song@dexterstudios.com)
- 💼 **LinkedIn**: [linkedin.com/in/jaewonsongj1](https://www.linkedin.com/in/jaewonsongj1/)

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🤝 Contributing

We welcome contributions from the community! Whether you're from a VFX studio, research institution, or independent developer:

### 🐛 Bug Reports & Feature Requests
- **Issues**: Report bugs or request features via [GitHub Issues](https://github.com/JaewonSongJ1/dynamic_3dmesh_denoiser/issues)
- **Discussions**: Join technical discussions in [GitHub Discussions](https://github.com/JaewonSongJ1/dynamic_3dmesh_denoiser/discussions)

### 💻 Code Contributions
1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-algorithm`)
3. **Commit** your changes (`git commit -m 'Add amazing denoising algorithm'`)
4. **Push** to the branch (`git push origin feature/amazing-algorithm`)
5. **Open** a Pull Request

### 📚 Documentation & Testing
- **Documentation**: Improve README, add tutorials, write algorithm guides
- **Testing**: Contribute test cases, benchmark datasets, performance analysis
- **Examples**: Share real-world usage examples and case studies

### 🏢 Industry Collaboration
Interested in collaboration or integration into your pipeline? Reach out to discuss:
- Algorithm development and optimization
- Production pipeline integration
- Performance analysis and benchmarking
- Technical consulting and support

---

## 🙏 Acknowledgments

- **Minyeong Jeong**: Dynamic 3D mesh device development and hardware integration

---

<div align="center">

**Professional 4D Mesh Denoising Made Simple** 🎬✨

*Empowering creators with production-grade tools*

[![GitHub stars](https://img.shields.io/github/stars/JaewonSongJ1/dynamic_3dmesh_denoiser?style=social)](https://github.com/JaewonSongJ1/dynamic_3dmesh_denoiser)
[![GitHub forks](https://img.shields.io/github/forks/JaewonSongJ1/dynamic_3dmesh_denoiser?style=social)](https://github.com/JaewonSongJ1/dynamic_3dmesh_denoiser)
[![GitHub issues](https://img.shields.io/github/issues/JaewonSongJ1/dynamic_3dmesh_denoiser?style=social)](https://github.com/JaewonSongJ1/dynamic_3dmesh_denoiser/issues)

*Built with ❤️ by [Dexter Studios](https://www.dexterstudios.com/) R&D Team*

</div>