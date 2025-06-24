// ============================================================================
// Complete Alembic Bilateral Mesh Denoiser - C++ Implementation
// ============================================================================
// High-performance C++ implementation of bilateral temporal filtering for 4D mesh denoising.
// Processes Alembic (.abc) files directly without Maya dependency.
//
// ✔ Standalone C++ Implementation: No Maya, no Python dependencies
// ✔ High Performance: Native binary processing with multi-threading support
// ✔ Bilateral Temporal Filtering: Advanced noise reduction preserving detail
// ✔ Adaptive Windowing: Dynamic window sizing based on motion analysis
// ✔ Edge Preservation: Maintains sharp features and motion boundaries
// ✔ Production Ready: Handles large 4D scan datasets efficiently
// ✔ Default Strong Denoising: Optimized for heavy noise reduction out-of-the-box
//
// USAGE:
// ------
// Basic usage (uses default parameters):
//   AlembicBilateralDenoiser.exe input.abc output.abc
//
// With frame range:
//   AlembicBilateralDenoiser.exe input.abc output.abc 1 100
//
// With all parameters:
//   AlembicBilateralDenoiser.exe input.abc output.abc 1 100 5 1.0 0.05 0.02 0.05
//
// PARAMETERS:
// -----------
// window_size (default: 15) - STRONG denoising by default
//   - Temporal window size for filtering (must be odd number)
//   - Larger values = more smoothing but slower processing
//   - Recommended: 5-15 for most cases
//   - Use 5 for detail preservation, 9 for medium, 15 for strong denoising
//
// sigma_temporal (default: 4.0) - WIDE temporal range by default
//   - Controls temporal weight falloff in bilateral filter
//   - Larger values = more frames contribute to filtering
//   - Recommended: 1.0-5.0
//   - Use 1.0 for sharp temporal edges, 4.0 for strong smoothing
//
// sigma_spatial (default: 0.25) - STRONG spatial smoothing by default
//   - Controls spatial weight based on vertex displacement
//   - Larger values = more aggressive smoothing
//   - Recommended: 0.05-0.35
//   - Use 0.05 for detail preservation, 0.25 for strong denoising
//
// motion_threshold (default: 0.1) - HANDLE large motions by default
//   - Threshold for adaptive window sizing
//   - Frames with motion > threshold get smaller windows
//   - Recommended: 0.02-0.15
//   - Use 0.02 for sensitive motion, 0.1 for stability with large motions
//
// edge_threshold (default: 0.15) - RELAXED edge preservation by default
//   - Threshold for edge preservation
//   - Motion > threshold triggers edge-preserving mode
//   - Recommended: 0.05-0.2
//   - Use 0.05 for strict edge preservation, 0.15 for stronger smoothing
//
// WORKFLOW TIPS:
// --------------
// 1. Default parameters provide strong denoising suitable for heavy 4D scan noise
// 2. If too much smoothing: reduce window_size (to 9), sigma_spatial (to 0.15)
// 3. If not enough denoising: increase sigma_temporal (to 5.0), sigma_spatial (to 0.35)
// 4. If losing important details: reduce sigma_spatial (to 0.15), lower motion_threshold (to 0.05)
// 5. If motion artifacts appear: adjust edge_threshold (to 0.1 for more preservation)
// 6. For subtle touch-up: use --window 7 --sigma-temporal 1.5 --sigma-spatial 0.08
//
// PERFORMANCE OPTIMIZATION:
// -------------------------
// - Uses OpenMP for multi-threading (if available)
// - Memory-efficient frame loading
// - Optimized bilateral weight calculations
// - Adaptive processing based on motion analysis
//
// ============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <map>
#include <chrono>
#include <iomanip>
#include <memory>

#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreOgawa/All.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace Alembic::AbcCoreOgawa;

class BilateralMeshDenoiser {
public:
    // Algorithm parameters (moved to public)
    struct DenoiseParams {
        int baseWindowSize = 15;       // Temporal window size (must be odd) - Strong denoising default
        float sigmaTemporalr = 4.0f;   // Temporal weight falloff - Wide temporal range
        float sigmaSpatial = 0.25f;    // Spatial weight based on displacement - Strong smoothing
        float motionThreshold = 0.1f;  // Threshold for adaptive windowing - Handle larger motions
        float edgeThreshold = 0.15f;   // Threshold for edge preservation - Relaxed edge preservation
        bool verbose = true;           // Debug output
    };

private:
    // Core data structures
    std::map<int, std::vector<V3f>> allFrames;
    std::map<int, std::vector<V3f>> denoisedFrames;
    std::map<int, float> motionMagnitudes;
    std::map<int, int> adaptiveWindows;
    
    // Mesh information
    std::vector<int32_t> indices;
    std::vector<int32_t> counts;
    std::string meshName;
    size_t vertexCount;
    
    // Algorithm parameters
    DenoiseParams params;
    
    // Performance tracking
    struct TimingData {
        double fileReading = 0.0;
        double motionAnalysis = 0.0;
        double bilateralFiltering = 0.0;
        double fileWriting = 0.0;
        double total = 0.0;
    } timing;

public:
    BilateralMeshDenoiser() = default;
    
    bool processFile(const std::string& inputFile, const std::string& outputFile,
                    int startFrame = -1, int endFrame = -1,
                    const DenoiseParams& userParams = DenoiseParams()) {
        
        params = userParams;
        
        auto totalStart = std::chrono::high_resolution_clock::now();
        
        if (params.verbose) {
            printHeader(inputFile, outputFile, startFrame, endFrame);
        }
        
        try {
            // Step 1: Read Alembic file
            if (!readAlembicFile(inputFile, startFrame, endFrame)) {
                std::cerr << "Failed to read input file" << std::endl;
                return false;
            }
            
            // Step 2: Analyze motion patterns
            calculateMotionMagnitudes();
            
            // Step 3: Calculate adaptive windows
            calculateAdaptiveWindows();
            
            // Step 4: Apply bilateral filtering
            applyBilateralFiltering();
            
            // Step 5: Write output file
            if (!writeAlembicFile(outputFile)) {
                std::cerr << "Failed to write output file" << std::endl;
                return false;
            }
            
            auto totalEnd = std::chrono::high_resolution_clock::now();
            timing.total = std::chrono::duration<double>(totalEnd - totalStart).count();
            
            if (params.verbose) {
                printPerformanceReport();
            }
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error during processing: " << e.what() << std::endl;
            return false;
        }
    }

private:
    bool readAlembicFile(const std::string& filename, int startFrame, int endFrame) {
        if (params.verbose) {
            std::cout << "Reading Alembic file..." << std::endl;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            IArchive archive(ReadArchive(), filename);
            IObject topObj = archive.getTop();
            
            // Find PolyMesh
            IPolyMesh meshObj;
            if (!findFirstPolyMesh(topObj, meshObj)) {
                std::cerr << "No PolyMesh found in file" << std::endl;
                return false;
            }
            
            meshName = meshObj.getName();
            IPolyMeshSchema& mesh = meshObj.getSchema();
            size_t totalFrames = mesh.getNumSamples();
            
            if (params.verbose) {
                std::cout << "Found mesh: " << meshName << std::endl;
                std::cout << "Total frames: " << totalFrames << std::endl;
            }
            
            // Determine frame range
            if (startFrame < 0) startFrame = 0;
            if (endFrame < 0) endFrame = static_cast<int>(totalFrames) - 1;
            
            startFrame = std::max(0, startFrame);
            endFrame = std::min(static_cast<int>(totalFrames) - 1, endFrame);
            
            if (startFrame > endFrame) {
                std::cerr << "Invalid frame range" << std::endl;
                return false;
            }
            
            // Read topology from first frame
            IPolyMeshSchema::Sample sample0;
            mesh.get(sample0, ISampleSelector(static_cast<Alembic::AbcCoreAbstract::index_t>(startFrame)));
            
            vertexCount = sample0.getPositions()->size();
            
            Int32ArraySamplePtr indicesPtr = sample0.getFaceIndices();
            Int32ArraySamplePtr countsPtr = sample0.getFaceCounts();
            
            if (indicesPtr && countsPtr) {
                indices.assign(indicesPtr->get(), indicesPtr->get() + indicesPtr->size());
                counts.assign(countsPtr->get(), countsPtr->get() + countsPtr->size());
            }
            
            if (params.verbose) {
                std::cout << "Vertex count: " << vertexCount << std::endl;
                std::cout << "Face count: " << counts.size() << std::endl;
                std::cout << "Processing frames " << startFrame << "-" << endFrame << std::endl;
            }
            
            // Read all frames
            for (int frame = startFrame; frame <= endFrame; ++frame) {
                IPolyMeshSchema::Sample sample;
                mesh.get(sample, ISampleSelector(static_cast<Alembic::AbcCoreAbstract::index_t>(frame)));
                
                P3fArraySamplePtr positions = sample.getPositions();
                std::vector<V3f> frameVertices(positions->get(), positions->get() + positions->size());
                
                allFrames[frame] = std::move(frameVertices);
                
                if (params.verbose && ((frame - startFrame) % std::max(1, (endFrame - startFrame) / 10) == 0)) {
                    std::cout << "Loaded frame " << frame << std::endl;
                }
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            timing.fileReading = std::chrono::duration<double>(end - start).count();
            
            if (params.verbose) {
                std::cout << "Successfully loaded " << allFrames.size() << " frames in " 
                         << timing.fileReading << "s" << std::endl;
            }
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error reading file: " << e.what() << std::endl;
            return false;
        }
    }
    
    void calculateMotionMagnitudes() {
        if (params.verbose) {
            std::cout << "Analyzing motion patterns..." << std::endl;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<int> frameKeys;
        for (const auto& pair : allFrames) {
            frameKeys.push_back(pair.first);
        }
        std::sort(frameKeys.begin(), frameKeys.end());
        
        for (size_t i = 0; i < frameKeys.size(); ++i) {
            int frame = frameKeys[i];
            float totalMotion = 0.0f;
            
            if (i == 0 && frameKeys.size() > 1) {
                // First frame: compare with next
                int nextFrame = frameKeys[1];
                for (size_t v = 0; v < vertexCount; ++v) {
                    V3f motion = allFrames[nextFrame][v] - allFrames[frame][v];
                    totalMotion += motion.length();
                }
            } else if (i == frameKeys.size() - 1) {
                // Last frame: compare with previous
                int prevFrame = frameKeys[i - 1];
                for (size_t v = 0; v < vertexCount; ++v) {
                    V3f motion = allFrames[frame][v] - allFrames[prevFrame][v];
                    totalMotion += motion.length();
                }
            } else {
                // Middle frame: average of both directions
                int prevFrame = frameKeys[i - 1];
                int nextFrame = frameKeys[i + 1];
                for (size_t v = 0; v < vertexCount; ++v) {
                    V3f motionPrev = allFrames[frame][v] - allFrames[prevFrame][v];
                    V3f motionNext = allFrames[nextFrame][v] - allFrames[frame][v];
                    totalMotion += (motionPrev.length() + motionNext.length()) * 0.5f;
                }
            }
            
            motionMagnitudes[frame] = totalMotion / static_cast<float>(vertexCount);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        timing.motionAnalysis = std::chrono::duration<double>(end - start).count();
        
        if (params.verbose) {
            float avgMotion = 0.0f;
            for (const auto& pair : motionMagnitudes) {
                avgMotion += pair.second;
            }
            avgMotion /= motionMagnitudes.size();
            
            std::cout << "Motion analysis completed in " << timing.motionAnalysis << "s" << std::endl;
            std::cout << "Average motion magnitude: " << std::fixed << std::setprecision(6) 
                     << avgMotion << std::endl;
        }
    }
    
    void calculateAdaptiveWindows() {
        if (params.verbose) {
            std::cout << "Calculating adaptive window sizes..." << std::endl;
        }
        
        for (const auto& pair : motionMagnitudes) {
            int frame = pair.first;
            float motionMag = pair.second;
            
            float scaleFactor;
            if (motionMag > params.motionThreshold) {
                // High motion: smaller window
                scaleFactor = std::max(0.3f, 1.0f - (motionMag / params.motionThreshold));
            } else {
                // Low motion: larger window
                scaleFactor = std::min(2.0f, 1.0f + (params.motionThreshold / std::max(motionMag, 0.001f)));
            }
            
            int adaptiveSize = static_cast<int>(params.baseWindowSize * scaleFactor);
            if (adaptiveSize % 2 == 0) adaptiveSize += 1; // Ensure odd number
            
            adaptiveSize = std::max(3, std::min(adaptiveSize, 15)); // Clamp to reasonable range
            adaptiveWindows[frame] = adaptiveSize;
        }
        
        if (params.verbose) {
            int minWindow = 15, maxWindow = 3;
            for (const auto& pair : adaptiveWindows) {
                minWindow = std::min(minWindow, pair.second);
                maxWindow = std::max(maxWindow, pair.second);
            }
            std::cout << "Window size range: " << minWindow << "-" << maxWindow << std::endl;
        }
    }
    
    std::vector<V3f> bilateralTemporalFilter(int centerFrame) {
        if (allFrames.find(centerFrame) == allFrames.end()) {
            return std::vector<V3f>();
        }
        
        int windowSize = adaptiveWindows[centerFrame];
        int halfWindow = windowSize / 2;
        
        std::vector<int> frameKeys;
        for (const auto& pair : allFrames) {
            frameKeys.push_back(pair.first);
        }
        std::sort(frameKeys.begin(), frameKeys.end());
        
        auto centerIt = std::find(frameKeys.begin(), frameKeys.end(), centerFrame);
        if (centerIt == frameKeys.end()) {
            return std::vector<V3f>();
        }
        
        int centerIdx = static_cast<int>(centerIt - frameKeys.begin());
        int startIdx = std::max(0, centerIdx - halfWindow);
        int endIdx = std::min(static_cast<int>(frameKeys.size()) - 1, centerIdx + halfWindow);
        
        const std::vector<V3f>& centerPositions = allFrames[centerFrame];
        std::vector<V3f> filteredPositions(vertexCount, V3f(0.0f, 0.0f, 0.0f));
        std::vector<float> totalWeights(vertexCount, 0.0f);
        
        for (int i = startIdx; i <= endIdx; ++i) {
            int frame = frameKeys[i];
            const std::vector<V3f>& framePositions = allFrames[frame];
            
            // Temporal weight
            float temporalDist = static_cast<float>(std::abs(i - centerIdx));
            float temporalWeight = std::exp(-(temporalDist * temporalDist) / (2.0f * params.sigmaTemporalr * params.sigmaTemporalr));
            
            // Process vertices (with OpenMP if available)
            #ifdef _OPENMP
            #pragma omp parallel for
            #endif
            for (int v = 0; v < static_cast<int>(vertexCount); ++v) {
                // Spatial weight
                V3f spatialDiff = framePositions[v] - centerPositions[v];
                float spatialDist = spatialDiff.length();
                float spatialWeight = std::exp(-(spatialDist * spatialDist) / (2.0f * params.sigmaSpatial * params.sigmaSpatial));
                
                // Edge preservation
                float motionMag = motionMagnitudes.at(centerFrame);
                if (motionMag > params.edgeThreshold) {
                    float edgeFactor = std::min(2.0f, motionMag / params.edgeThreshold);
                    spatialWeight = std::pow(spatialWeight, edgeFactor);
                }
                
                // Final weight
                float finalWeight = temporalWeight * spatialWeight;
                
                // Accumulate weighted average
                #ifdef _OPENMP
                #pragma omp atomic
                #endif
                filteredPositions[v].x += finalWeight * framePositions[v].x;
                #ifdef _OPENMP
                #pragma omp atomic
                #endif
                filteredPositions[v].y += finalWeight * framePositions[v].y;
                #ifdef _OPENMP
                #pragma omp atomic
                #endif
                filteredPositions[v].z += finalWeight * framePositions[v].z;
                #ifdef _OPENMP
                #pragma omp atomic
                #endif
                totalWeights[v] += finalWeight;
            }
        }
        
        // Normalize
        for (size_t v = 0; v < vertexCount; ++v) {
            if (totalWeights[v] > 1e-8f) {
                filteredPositions[v] /= totalWeights[v];
            } else {
                filteredPositions[v] = centerPositions[v];
            }
        }
        
        return filteredPositions;
    }
    
    void applyBilateralFiltering() {
        if (params.verbose) {
            std::cout << "Applying bilateral temporal filtering..." << std::endl;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<int> frameKeys;
        for (const auto& pair : allFrames) {
            frameKeys.push_back(pair.first);
        }
        std::sort(frameKeys.begin(), frameKeys.end());
        
        int processedFrames = 0;
        for (int frame : frameKeys) {
            std::vector<V3f> smoothedPositions = bilateralTemporalFilter(frame);
            if (!smoothedPositions.empty()) {
                denoisedFrames[frame] = std::move(smoothedPositions);
            }
            
            processedFrames++;
            if (params.verbose && (processedFrames % std::max(1, static_cast<int>(frameKeys.size()) / 10) == 0)) {
                std::cout << "Processed frame " << frame << " (" << processedFrames << "/" << frameKeys.size() << ")" << std::endl;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        timing.bilateralFiltering = std::chrono::duration<double>(end - start).count();
        
        if (params.verbose) {
            std::cout << "Bilateral filtering completed in " << timing.bilateralFiltering << "s" << std::endl;
        }
    }
    
    bool writeAlembicFile(const std::string& filename) {
        if (params.verbose) {
            std::cout << "Writing denoised Alembic file..." << std::endl;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            OArchive archive(WriteArchive(), filename);
            OObject topObj(archive, "ABC");
            
            OPolyMesh meshObj(topObj, meshName.empty() ? "denoised_mesh" : meshName);
            OPolyMeshSchema& mesh = meshObj.getSchema();
            
            // Set time sampling (24fps default)
            TimeSampling ts(1.0/24.0, 0.0);
            uint32_t tsidx = archive.addTimeSampling(ts);
            mesh.setTimeSampling(tsidx);
            
            // Write all frames
            std::vector<int> frameKeys;
            for (const auto& pair : denoisedFrames) {
                frameKeys.push_back(pair.first);
            }
            std::sort(frameKeys.begin(), frameKeys.end());
            
            for (size_t i = 0; i < frameKeys.size(); ++i) {
                int frame = frameKeys[i];
                const std::vector<V3f>& positions = denoisedFrames[frame];
                
                OPolyMeshSchema::Sample sample;
                sample.setPositions(P3fArraySample(positions));
                
                // Set topology only on first frame
                if (i == 0 && !indices.empty() && !counts.empty()) {
                    sample.setFaceIndices(Int32ArraySample(indices));
                    sample.setFaceCounts(Int32ArraySample(counts));
                }
                
                mesh.set(sample);
                
                if (params.verbose && (i % std::max(size_t(1), frameKeys.size() / 10) == 0)) {
                    std::cout << "Wrote frame " << frame << " (" << (i + 1) << "/" << frameKeys.size() << ")" << std::endl;
                }
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            timing.fileWriting = std::chrono::duration<double>(end - start).count();
            
            if (params.verbose) {
                std::cout << "Successfully wrote " << frameKeys.size() << " frames in " 
                         << timing.fileWriting << "s" << std::endl;
            }
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error writing file: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool findFirstPolyMesh(IObject obj, IPolyMesh& meshObj) {
        if (IPolyMesh::matches(obj.getMetaData())) {
            meshObj = IPolyMesh(obj, kWrapExisting);
            return true;
        }
        
        for (size_t i = 0; i < obj.getNumChildren(); ++i) {
            IObject child(obj, obj.getChildHeader(i).getName());
            if (findFirstPolyMesh(child, meshObj)) {
                return true;
            }
        }
        return false;
    }
    
    void printHeader(const std::string& inputFile, const std::string& outputFile, 
                    int startFrame, int endFrame) {
        std::cout << "============================================================================" << std::endl;
        std::cout << "ALEMBIC BILATERAL MESH DENOISER - C++ IMPLEMENTATION" << std::endl;
        std::cout << "============================================================================" << std::endl;
        std::cout << "Input:  " << inputFile << std::endl;
        std::cout << "Output: " << outputFile << std::endl;
        if (startFrame >= 0 && endFrame >= 0) {
            std::cout << "Frame range: " << startFrame << "-" << endFrame << std::endl;
        }
        std::cout << "Parameters:" << std::endl;
        std::cout << "  Window size: " << params.baseWindowSize << std::endl;
        std::cout << "  Sigma temporal: " << params.sigmaTemporalr << std::endl;
        std::cout << "  Sigma spatial: " << params.sigmaSpatial << std::endl;
        std::cout << "  Motion threshold: " << params.motionThreshold << std::endl;
        std::cout << "  Edge threshold: " << params.edgeThreshold << std::endl;
        std::cout << "============================================================================" << std::endl;
    }
    
    void printPerformanceReport() {
        std::cout << "============================================================================" << std::endl;
        std::cout << "PROCESSING COMPLETED SUCCESSFULLY!" << std::endl;
        std::cout << "============================================================================" << std::endl;
        std::cout << "Vertex count: " << vertexCount << std::endl;
        std::cout << "Frames processed: " << denoisedFrames.size() << std::endl;
        std::cout << "Total processing time: " << std::fixed << std::setprecision(2) 
                 << timing.total << " seconds" << std::endl;
        
        if (!motionMagnitudes.empty()) {
            float avgMotion = 0.0f;
            for (const auto& pair : motionMagnitudes) {
                avgMotion += pair.second;
            }
            avgMotion /= motionMagnitudes.size();
            std::cout << "Average motion magnitude: " << std::fixed << std::setprecision(6) 
                     << avgMotion << std::endl;
        }
        
        if (!adaptiveWindows.empty()) {
            int minWindow = 15, maxWindow = 3;
            for (const auto& pair : adaptiveWindows) {
                minWindow = std::min(minWindow, pair.second);
                maxWindow = std::max(maxWindow, pair.second);
            }
            std::cout << "Window size range: " << minWindow << "-" << maxWindow << std::endl;
        }
        
        std::cout << "\nPERFORMANCE BREAKDOWN:" << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;
        
        auto printTiming = [this](const std::string& name, double time) {
            if (time > 0.0) {
                double percentage = (time / timing.total) * 100.0;
                std::cout << std::left << std::setw(25) << (name + ":") 
                         << std::right << std::setw(8) << std::fixed << std::setprecision(2) 
                         << time << "s (" << std::setw(5) << std::setprecision(1) 
                         << percentage << "%)" << std::endl;
            }
        };
        
        printTiming("File Reading", timing.fileReading);
        printTiming("Motion Analysis", timing.motionAnalysis);
        printTiming("Bilateral Filtering", timing.bilateralFiltering);
        printTiming("File Writing", timing.fileWriting);
        
        std::cout << "------------------------------------------------------------" << std::endl;
        if (vertexCount > 0 && !denoisedFrames.empty()) {
            double perVertex = (timing.total / vertexCount) * 1000.0;
            double perFrame = timing.total / denoisedFrames.size();
            std::cout << "Performance per vertex: " << std::fixed << std::setprecision(3) 
                     << perVertex << " ms/vertex" << std::endl;
            std::cout << "Performance per frame:  " << std::fixed << std::setprecision(3) 
                     << perFrame << " s/frame" << std::endl;
        }
        std::cout << "============================================================================" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    // Default parameters
    std::string inputFile, outputFile;
    int startFrame = -1, endFrame = -1;
    BilateralMeshDenoiser::DenoiseParams params;
    
    // Function to show help
    auto showHelp = [&]() {
        std::cout << "ALEMBIC BILATERAL MESH DENOISER - C++ IMPLEMENTATION" << std::endl;
        std::cout << "====================================================" << std::endl;
        std::cout << "Usage: " << argv[0] << " input.abc output.abc [options]" << std::endl;
        std::cout << "\nRequired:" << std::endl;
        std::cout << "  input.abc                Input Alembic file" << std::endl;
        std::cout << "  output.abc               Output Alembic file" << std::endl;
        std::cout << "\nOptional Frame Range:" << std::endl;
        std::cout << "  --sf N, --start-frame N  Start frame (0-based Alembic index, default: 0)" << std::endl;
        std::cout << "  --ef N, --end-frame N    End frame (0-based Alembic index, default: last)" << std::endl;
        std::cout << "  --maya-range M1 M2       Maya frame range (1-based, converts to 0-based)" << std::endl;
        std::cout << "\nOptional Denoising Parameters:" << std::endl;
        std::cout << "  --window N               Temporal window size (default: " << params.baseWindowSize << ")" << std::endl;
        std::cout << "  --sigma-temporal F       Temporal weight falloff (default: " << params.sigmaTemporalr << ")" << std::endl;
        std::cout << "  --sigma-spatial F        Spatial weight threshold (default: " << params.sigmaSpatial << ")" << std::endl;
        std::cout << "  --motion-thresh F        Motion threshold for adaptive windowing (default: " << params.motionThreshold << ")" << std::endl;
        std::cout << "  --edge-thresh F          Edge preservation threshold (default: " << params.edgeThreshold << ")" << std::endl;
        std::cout << "  --quiet                  Disable verbose output" << std::endl;
        std::cout << "  -h, --help               Show this help message" << std::endl;
        std::cout << "\nFrame Index Examples:" << std::endl;
        std::cout << "  Maya frames 1-100  = Alembic frames 0-99" << std::endl;
        std::cout << "  Maya frames 5-50   = Alembic frames 4-49" << std::endl;
        std::cout << "\nUsage Examples:" << std::endl;
        std::cout << "  # Basic usage (strong denoising, all frames)" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  # Process Maya frames 1-100 with default strong denoising" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --maya-range 1 100" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  # Ultra-strong denoising (maximum smoothing)" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --maya-range 1 100 --window 15 --sigma-temporal 5.0 --sigma-spatial 0.35" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  # Medium denoising (reduced from default)" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --maya-range 1 100 --window 9 --sigma-temporal 2.5 --sigma-spatial 0.15" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  # Subtle denoising (preserve more detail)" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --maya-range 1 100 --window 7 --sigma-temporal 1.5 --sigma-spatial 0.08" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  # Detail-preserving denoising (minimal smoothing)" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --maya-range 1 100 --window 5 --sigma-temporal 1.0 --sigma-spatial 0.05" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  # Custom edge-sensitive denoising" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --maya-range 1 100 --edge-thresh 0.08 --motion-thresh 0.05" << std::endl;
        std::cout << "\nParameter Guidelines:" << std::endl;
        std::cout << "  window:         5=minimal, 9=medium, 15=strong(default), 15=maximum" << std::endl;
        std::cout << "  sigma-temporal: 1.0=sharp motion, 2.5=medium, 4.0=strong(default), 5.0=ultra-smooth" << std::endl;
        std::cout << "  sigma-spatial:  0.05=preserve detail, 0.15=medium, 0.25=strong(default), 0.35=maximum smoothing" << std::endl;
        std::cout << "  motion-thresh:  0.02=very sensitive, 0.05=medium, 0.1=strong(default), 0.15=relaxed" << std::endl;
        std::cout << "  edge-thresh:    0.05=preserve edges, 0.1=medium, 0.15=strong(default), 0.2=smooth edges" << std::endl;
    };
    
    // Check for help or insufficient arguments
    if (argc < 3) {
        showHelp();
        return 1;
    }
    
    // Check for help flag early
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            showHelp();
            return 0;
        }
    }
    
    inputFile = argv[1];
    outputFile = argv[2];
    
    // Parse named arguments
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        
        if ((arg == "--sf" || arg == "--start-frame") && i + 1 < argc) {
            startFrame = std::stoi(argv[++i]);
        }
        else if ((arg == "--ef" || arg == "--end-frame") && i + 1 < argc) {
            endFrame = std::stoi(argv[++i]);
        }
        else if (arg == "--maya-range" && i + 2 < argc) {
            int mayaStart = std::stoi(argv[++i]);
            int mayaEnd = std::stoi(argv[++i]);
            startFrame = mayaStart - 1;  // Convert 1-based to 0-based
            endFrame = mayaEnd - 1;
            std::cout << "Maya range " << mayaStart << "-" << mayaEnd 
                     << " converted to Alembic range " << startFrame << "-" << endFrame << std::endl;
        }
        else if (arg == "--window" && i + 1 < argc) {
            params.baseWindowSize = std::stoi(argv[++i]);
        }
        else if (arg == "--sigma-temporal" && i + 1 < argc) {
            params.sigmaTemporalr = std::stof(argv[++i]);
        }
        else if (arg == "--sigma-spatial" && i + 1 < argc) {
            params.sigmaSpatial = std::stof(argv[++i]);
        }
        else if (arg == "--motion-thresh" && i + 1 < argc) {
            params.motionThreshold = std::stof(argv[++i]);
        }
        else if (arg == "--edge-thresh" && i + 1 < argc) {
            params.edgeThreshold = std::stof(argv[++i]);
        }
        else if (arg == "--quiet") {
            params.verbose = false;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    // Validate parameters
    if (params.baseWindowSize % 2 == 0) {
        params.baseWindowSize += 1; // Ensure odd number
    }
    params.baseWindowSize = std::max(3, std::min(params.baseWindowSize, 15));
    
    // Create denoiser and process
    BilateralMeshDenoiser denoiser;
    
    bool success = denoiser.processFile(inputFile, outputFile, startFrame, endFrame, params);
    
    if (success) {
        std::cout << "\n✅ PROCESSING COMPLETED SUCCESSFULLY!" << std::endl;
        std::cout << "📁 Output file: " << outputFile << std::endl;
        if (startFrame >= 0 && endFrame >= 0) {
            std::cout << "📊 Processed Alembic frames: " << startFrame << "-" << endFrame 
                     << " (Maya equivalent: " << (startFrame + 1) << "-" << (endFrame + 1) << ")" << std::endl;
        }
        std::cout << "\nTo test in Maya:" << std::endl;
        std::cout << "1. Open Maya" << std::endl;
        std::cout << "2. Cache → Alembic Cache → Import Alembic" << std::endl;
        std::cout << "3. Select: " << outputFile << std::endl;
        std::cout << "4. Compare with original for noise reduction quality" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ PROCESSING FAILED!" << std::endl;
        return 1;
    }
}