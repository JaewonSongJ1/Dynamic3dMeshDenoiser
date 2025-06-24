# Dynamic 3D Mesh Denoiser 🎬

High-performance C++ toolkit for denoising temporal mesh sequences from 4D scanning and motion capture data. Processes Alembic (.abc) files directly for production pipelines.

## ✨ Key Features

- **🚀 High Performance**: Multi-threaded C++ with OpenMP support
- **🎛️ Multiple Algorithms**: Bilateral filtering + Temporal smoothing
- **📁 Standalone**: No Maya/Python dependencies, direct Alembic processing
- **🔧 Production Ready**: Handles 24K+ vertices, 1600+ frames efficiently
- **🎯 Smart Processing**: FPS-aware auto-adjustment, motion detection

## 🛠️ Quick Start

### Build (Windows)
```bash
git clone https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser.git
cd Dynamic3dMeshDenoiser
build.bat
```

### Usage Examples
```bash
# Basic denoising (auto window size based on FPS)
BilateralMeshDenoiser.exe input.abc output.abc --maya-range 1 100

# Simple temporal smoothing (faster, lighter)
TemporalMeshDenoiser.exe input.abc output.abc --maya-range 1 100

# Custom settings
TemporalMeshDenoiser.exe input.abc output.abc --maya-range 1 100 --window 7 --weight gaussian
```

## 🔬 Algorithms

### 1. Bilateral Temporal Filter (`BilateralMeshDenoiser.exe`)
- **Best for**: Heavy noise reduction, production workflows
- **Features**: Adaptive windowing, edge preservation, motion analysis
- **Performance**: 24K vertices × 1620 frames in ~2.4 seconds

### 2. Temporal Smoothing Filter (`TemporalMeshDenoiser.exe`) ⭐ *New*
- **Best for**: Quick denoising, preview workflows
- **Features**: FPS-aware auto window sizing, linear/gaussian weighting
- **Performance**: Same dataset in ~0.8 seconds
- **Smart defaults**: 24fps→window 3, 60fps→window 5, 120fps→window 7

## 📊 Algorithm Comparison

| Algorithm | Speed | Quality | Use Case |
|-----------|-------|---------|----------|
| **Bilateral** | Medium | Excellent | Production, heavy noise |
| **Temporal** | Fast | Good | Previews, light touch-ups |

## ⚙️ Key Parameters

### Temporal Denoiser Parameters
```bash
--window <size>          # Auto-detected from FPS (or 3,5,7,9)
--weight linear|gaussian # Linear faster, gaussian smoother for large windows
--sigma <value>          # Gaussian standard deviation (default: 1.0)
--maya-range <s> <e>     # Maya frame range (1-based)
```

### FPS-Based Auto Window Sizing
- **24fps** (cinema): window 3 (~0.125s)
- **30fps** (video): window 3 (~0.100s) 
- **60fps** (high-fps): window 5 (~0.083s)
- **120fps+** (ultra): window 7 (~0.058s)

*Manual `--window` setting overrides auto-detection*

## 🎯 When to Use What

### Use **Temporal Denoiser** for:
- Quick preview denoising
- Light noise reduction
- Fast iteration workflows
- When you need speed over maximum quality

### Use **Bilateral Denoiser** for:
- Final production rendering
- Heavy noise in 4D scans
- Maximum quality requirements
- Complex motion with detail preservation

## 🔧 Integration Examples

### Pipeline Batch Processing
```bash
# Light denoising for previews
for file in *.abc; do
    TemporalMeshDenoiser.exe "$file" "${file%.abc}_preview.abc" --quiet
done

# Production denoising
for file in *.abc; do  
    BilateralMeshDenoiser.exe "$file" "${file%.abc}_final.abc" --maya-range 1 100
done
```

### Maya Integration Result
- **Input mesh**: `head_lod2_meshShape`
- **Output transform**: `head_lod2_mesh_denoised`  
- **Output shape**: `head_lod2_meshShape_denoised`

## 📈 Performance Benchmark
*Test: 24,049 vertices × 1,620 frames*

| Algorithm | Time | Memory | Throughput |
|-----------|------|--------|------------|
| Temporal | 0.81s | ~45MB | 0.034ms/vertex |
| Bilateral | 2.4s | ~680MB | Production grade |

## 🏗️ Build Requirements

- **Visual Studio 2019+** (Windows) / **GCC 7+** (Linux)
- **CMake 3.16+**
- **vcpkg** (dependency management)
- **Alembic 1.8+** (auto-installed via vcpkg)

## 💡 Pro Tips

1. **Start with Temporal** for quick tests, switch to Bilateral for final
2. **FPS matters**: Higher FPS allows larger windows without motion blur
3. **Linear vs Gaussian**: Minimal difference for window ≤3
4. **Maya users**: Always use `--maya-range` for intuitive frame numbers

## 🤝 Contributing

Built by **Jaewon Song** (R&D Director, Dexter Studios) & **Minyeong Jeong**

- 📧 **Contact**: jaewon.song@dexterstudios.com
- 🐛 **Issues**: [GitHub Issues](https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/JaewonSongJ1/Dynamic3dMeshDenoiser/discussions)

## 📄 License

MIT License - See [LICENSE](LICENSE) for details.

---

**Professional 4D Mesh Denoising Made Simple** 🎬✨