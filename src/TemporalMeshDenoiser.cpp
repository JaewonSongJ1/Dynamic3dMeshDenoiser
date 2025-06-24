// TemporalMeshDenoiser.cpp
// Simple temporal mesh denoising using sliding window with linear or gaussian weighting
// Author: Jaewon Song, Dexter Studios
// Collaborator: Minyeong Jeong

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

class TemporalMeshDenoiser {
public:
    struct DenoiseParams {
        int windowSize = 5;                    // Total window size (should be odd)
        std::string weightFunction = "linear"; // "linear" or "gaussian"
        double gaussianSigma = 1.0;           // Standard deviation for gaussian weighting
        int startFrame = -1;                  // -1 means use all frames
        int endFrame = -1;                    // -1 means use all frames
        bool mayaRange = false;               // Convert Maya 1-based to Alembic 0-based
        bool verbose = true;                  // Debug output
        bool userSetWindowSize = false;       // Track if user explicitly set window size
    };

private:
    // Core data structures (same as BilateralMeshDenoiser)
    std::map<int, std::vector<V3f>> allFrames;
    std::map<int, std::vector<V3f>> denoisedFrames;
    
    // Mesh information
    std::vector<int32_t> indices;
    std::vector<int32_t> counts;
    std::string meshName;
    size_t vertexCount;
    
    // Algorithm parameters
    DenoiseParams params;
    std::vector<double> weights;
    int pad; // Half window size
    
    // Performance tracking
    struct TimingData {
        double fileReading = 0.0;
        double temporalFiltering = 0.0;
        double fileWriting = 0.0;
        double total = 0.0;
    } timing;

public:
    TemporalMeshDenoiser() = default;
    
    bool processFile(const std::string& inputFile, const std::string& outputFile,
                    int startFrame = -1, int endFrame = -1,
                    const DenoiseParams& userParams = DenoiseParams()) {
        
        params = userParams;
        
        // Ensure window size is odd
        if (params.windowSize % 2 == 0) {
            params.windowSize++;
            if (params.verbose) {
                std::cout << "Window size adjusted to " << params.windowSize << " (must be odd)" << std::endl;
            }
        }
        pad = (params.windowSize - 1) / 2;
        
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
            
            // Step 2: Compute weights
            computeWeights();
            
            // Step 3: Apply temporal filtering
            applyTemporalFiltering();
            
            // Step 4: Write output file
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
    // Calculate optimal window size based on FPS
    int calculateOptimalWindowSize(double fps) const {
        /*
        FPS-based Window Size Guidelines:
        ================================
        24 fps: window 3 (~0.125s temporal coverage)
        30 fps: window 3 (~0.100s temporal coverage)
        60 fps: window 5 (~0.083s temporal coverage)
        120fps+: window 7 (~0.058s temporal coverage)
        */
        
        int suggestedWindow;
        
        if (fps <= 25.0) {
            suggestedWindow = 3;  // 24fps
        } else if (fps <= 35.0) {
            suggestedWindow = 3;  // 30fps
        } else if (fps <= 65.0) {
            suggestedWindow = 5;  // 60fps
        } else {
            suggestedWindow = 7;  // 120fps+
        }
        
        // Ensure odd number
        if (suggestedWindow % 2 == 0) {
            suggestedWindow++;
        }
        
        return suggestedWindow;
    }

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
            
            // Try to extract FPS from TimeSampling
            TimeSamplingPtr timeSampling = mesh.getTimeSampling();
            double fps = 24.0; // Default fallback
            bool fpsDetected = false;
            
            if (timeSampling) {
                TimeSamplingType tsType = timeSampling->getTimeSamplingType();
                
                if (tsType.isUniform()) {
                    double timePerSample = tsType.getTimePerCycle();
                    if (timePerSample > 0.0) {
                        fps = 1.0 / timePerSample;
                        fpsDetected = true;
                    }
                } else if (tsType.isCyclic()) {
                    double timePerCycle = tsType.getTimePerCycle();
                    size_t samplesPerCycle = tsType.getNumSamplesPerCycle();
                    if (timePerCycle > 0.0 && samplesPerCycle > 0) {
                        double timePerSample = timePerCycle / samplesPerCycle;
                        fps = 1.0 / timePerSample;
                        fpsDetected = true;
                    }
                }
            }
            
            if (params.verbose && fpsDetected) {
                std::cout << "Detected FPS: " << std::fixed << std::setprecision(1) << fps << std::endl;
            }
            
            // Auto-adjust window size based on FPS if not explicitly set by user
            if (!params.userSetWindowSize) {
                int suggestedWindow = calculateOptimalWindowSize(fps);
                if (suggestedWindow != params.windowSize) {
                    params.windowSize = suggestedWindow;
                    pad = (params.windowSize - 1) / 2; // Update pad
                    if (params.verbose) {
                        std::cout << "Auto-adjusted window size to " << params.windowSize 
                                 << " based on " << std::fixed << std::setprecision(1) << fps << " fps" << std::endl;
                    }
                }
            } else if (params.verbose) {
                std::cout << "Using user-specified window size: " << params.windowSize << std::endl;
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

    void computeWeights() {
        weights.clear();
        weights.resize(params.windowSize);
        
        if (params.weightFunction == "linear") {
            // Linear weighting: higher weight at center, linearly decreasing to edges
            for (int i = 0; i < params.windowSize; ++i) {
                int distance = std::abs(i - pad);
                weights[i] = 1.0 - (double)distance / (double)(pad + 1);
            }
        } else if (params.weightFunction == "gaussian") {
            // Gaussian weighting
            double sigma = params.gaussianSigma;
            
            for (int i = 0; i < params.windowSize; ++i) {
                int distance = i - pad;
                weights[i] = std::exp(-(distance * distance) / (2.0 * sigma * sigma));
            }
        } else {
            std::cerr << "Error: Unknown weight function '" << params.weightFunction << "'" << std::endl;
            std::cerr << "Available options: 'linear', 'gaussian'" << std::endl;
            // Default to linear
            for (int i = 0; i < params.windowSize; ++i) {
                int distance = std::abs(i - pad);
                weights[i] = 1.0 - (double)distance / (double)(pad + 1);
            }
        }
        
        // Always normalize weights to sum to 1.0
        double sum = 0.0;
        for (double w : weights) {
            sum += w;
        }
        
        if (sum > 1e-8) {
            for (double& w : weights) {
                w /= sum;
            }
        }
        
        if (params.verbose) {
            std::cout << "Using " << params.weightFunction << " weighting with window size " << params.windowSize << std::endl;
            std::cout << "Weights (normalized): ";
            double checkSum = 0.0;
            for (size_t i = 0; i < weights.size(); ++i) {
                std::cout << std::fixed << std::setprecision(4) << weights[i];
                checkSum += weights[i];
                if (i < weights.size() - 1) std::cout << ", ";
            }
            std::cout << " (sum=" << std::fixed << std::setprecision(6) << checkSum << ")" << std::endl;
        }
    }

    std::vector<V3f> temporalFilter(int centerFrame) {
        if (allFrames.find(centerFrame) == allFrames.end()) {
            return std::vector<V3f>();
        }
        
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
        int startIdx = std::max(0, centerIdx - pad);
        int endIdx = std::min(static_cast<int>(frameKeys.size()) - 1, centerIdx + pad);
        
        std::vector<V3f> smoothedVertices(vertexCount, V3f(0.0f, 0.0f, 0.0f));
        
        // Apply temporal smoothing with sliding window
        #ifdef _OPENMP
        #pragma omp parallel for
        #endif
        for (int v = 0; v < static_cast<int>(vertexCount); ++v) {
            V3f weightedSum(0.0f, 0.0f, 0.0f);
            double totalWeight = 0.0;
            
            for (int i = startIdx; i <= endIdx; ++i) {
                int frame = frameKeys[i];
                int weightIdx = i - centerIdx + pad;
                
                // Clamp weight index to valid range
                weightIdx = std::max(0, std::min(params.windowSize - 1, weightIdx));
                
                double currentWeight = weights[weightIdx];
                weightedSum += static_cast<float>(currentWeight) * allFrames[frame][v];
                totalWeight += currentWeight;
            }
            
            // Normalize by total weight
            if (totalWeight > 1e-8) {
                smoothedVertices[v] = weightedSum / static_cast<float>(totalWeight);
            } else {
                smoothedVertices[v] = allFrames[centerFrame][v]; // Fallback to original
            }
        }
        
        return smoothedVertices;
    }

    void applyTemporalFiltering() {
        if (params.verbose) {
            std::cout << "Applying temporal filtering..." << std::endl;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<int> frameKeys;
        for (const auto& pair : allFrames) {
            frameKeys.push_back(pair.first);
        }
        std::sort(frameKeys.begin(), frameKeys.end());
        
        int processedFrames = 0;
        for (int frame : frameKeys) {
            std::vector<V3f> smoothedPositions = temporalFilter(frame);
            if (!smoothedPositions.empty()) {
                denoisedFrames[frame] = std::move(smoothedPositions);
            }
            
            processedFrames++;
            if (params.verbose && (processedFrames % std::max(1, static_cast<int>(frameKeys.size()) / 10) == 0)) {
                std::cout << "Processed frame " << frame << " (" << processedFrames << "/" << frameKeys.size() << ")" << std::endl;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        timing.temporalFiltering = std::chrono::duration<double>(end - start).count();
        
        if (params.verbose) {
            std::cout << "Temporal filtering completed in " << timing.temporalFiltering << "s" << std::endl;
        }
    }

    bool writeAlembicFile(const std::string& filename) {
        if (params.verbose) {
            std::cout << "Writing denoised Alembic file..." << std::endl;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            OArchive archive(WriteArchive(), filename);
            
            // Create denoised transform node name
            std::string denoisedTransformName;
            if (meshName.empty()) {
                denoisedTransformName = "mesh_denoised";
            } else {
                // Remove "Shape" suffix if it exists and add "_denoised"
                std::string baseName = meshName;
                if (baseName.length() > 5 && baseName.substr(baseName.length() - 5) == "Shape") {
                    baseName = baseName.substr(0, baseName.length() - 5);
                }
                denoisedTransformName = baseName + "_denoised";
            }
            
            OObject topObj(archive, denoisedTransformName);
            
            // Create denoised shape node name
            std::string denoisedShapeName;
            if (meshName.empty()) {
                denoisedShapeName = "meshShape_denoised";
            } else {
                denoisedShapeName = meshName + "_denoised";
            }
            
            OPolyMesh meshObj(topObj, denoisedShapeName);
            OPolyMeshSchema& mesh = meshObj.getSchema();
            
            if (params.verbose) {
                std::cout << "Creating transform: " << denoisedTransformName << std::endl;
                std::cout << "Creating shape: " << denoisedShapeName << std::endl;
            }
            
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
        std::cout << "TEMPORAL MESH DENOISER - C++ IMPLEMENTATION" << std::endl;
        std::cout << "============================================================================" << std::endl;
        std::cout << "Input:  " << inputFile << std::endl;
        std::cout << "Output: " << outputFile << std::endl;
        if (startFrame >= 0 && endFrame >= 0) {
            std::cout << "Frame range: " << startFrame << "-" << endFrame << std::endl;
        }
        std::cout << "Parameters:" << std::endl;
        std::cout << "  Window size: " << params.windowSize << std::endl;
        std::cout << "  Weight function: " << params.weightFunction << std::endl;
        if (params.weightFunction == "gaussian") {
            std::cout << "  Gaussian sigma: " << params.gaussianSigma << std::endl;
        }
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
        printTiming("Temporal Filtering", timing.temporalFiltering);
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
    TemporalMeshDenoiser::DenoiseParams params;
    
    // Function to show help
    auto showHelp = [&]() {
        std::cout << "TEMPORAL MESH DENOISER - C++ IMPLEMENTATION" << std::endl;
        std::cout << "=============================================" << std::endl;
        std::cout << "Usage: " << argv[0] << " input.abc output.abc [options]" << std::endl;
        std::cout << "\nRequired:" << std::endl;
        std::cout << "  input.abc                Input Alembic file" << std::endl;
        std::cout << "  output.abc               Output Alembic file" << std::endl;
        std::cout << "\nOptional Parameters:" << std::endl;
        std::cout << "  --window <size>          Temporal window size (auto-detected from FPS, must be odd)" << std::endl;
        std::cout << "  --weight <function>      Weight function: 'linear' or 'gaussian' (default: " << params.weightFunction << ")" << std::endl;
        std::cout << "  --sigma <value>          Gaussian standard deviation (default: " << params.gaussianSigma << ", used with --weight gaussian)" << std::endl;
        std::cout << "  --start-frame <frame>    Start frame (default: all frames)" << std::endl;
        std::cout << "  --end-frame <frame>      End frame (default: all frames)" << std::endl;
        std::cout << "  --maya-range <start> <end>  Maya frame range (1-based, converts to 0-based)" << std::endl;
        std::cout << "  --quiet                  Disable verbose output" << std::endl;
        std::cout << "  -h, --help               Show this help" << std::endl;
        std::cout << "\nFPS-Based Window Size Guidelines:" << std::endl;
        std::cout << "  24 fps (cinema):     window 3  (~0.125s)" << std::endl;
        std::cout << "  30 fps (video):      window 3  (~0.100s)" << std::endl;
        std::cout << "  60 fps (high-fps):   window 5  (~0.083s)" << std::endl;
        std::cout << "  120fps+ (ultra-fps): window 7  (~0.058s)" << std::endl;
        std::cout << "\nWeight Function Guidelines:" << std::endl;
        std::cout << "  linear:   Simple, fast, good for most cases" << std::endl;
        std::cout << "  gaussian: Smoother falloff, better for larger windows (5+)" << std::endl;
        std::cout << "  Note: For window=3, linear vs gaussian shows minimal difference" << std::endl;
        std::cout << "\nExamples:" << std::endl;
        std::cout << "  # Auto window size based on detected FPS" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --maya-range 1 100" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  # Force specific window size (overrides auto-detection)" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --window 7 --weight gaussian --sigma 1.5" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  # Minimal denoising for fast motion preservation" << std::endl;
        std::cout << "  " << argv[0] << " input.abc output.abc --window 3 --weight linear" << std::endl;
        std::cout << std::endl;
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
        
        if (arg == "--window" && i + 1 < argc) {
            params.windowSize = std::stoi(argv[++i]);
            params.userSetWindowSize = true;  // Mark as user-specified
        } else if (arg == "--weight" && i + 1 < argc) {
            params.weightFunction = argv[++i];
        } else if (arg == "--sigma" && i + 1 < argc) {
            params.gaussianSigma = std::stod(argv[++i]);
        } else if (arg == "--start-frame" && i + 1 < argc) {
            startFrame = std::stoi(argv[++i]);
        } else if (arg == "--end-frame" && i + 1 < argc) {
            endFrame = std::stoi(argv[++i]);
        } else if (arg == "--maya-range" && i + 2 < argc) {
            int mayaStart = std::stoi(argv[++i]);
            int mayaEnd = std::stoi(argv[++i]);
            startFrame = mayaStart - 1;  // Convert 1-based to 0-based
            endFrame = mayaEnd - 1;
            params.mayaRange = true;
            std::cout << "Maya range " << mayaStart << "-" << mayaEnd 
                     << " converted to Alembic range " << startFrame << "-" << endFrame << std::endl;
        } else if (arg == "--quiet") {
            params.verbose = false;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    // Create denoiser and process
    TemporalMeshDenoiser denoiser;
    
    bool success = denoiser.processFile(inputFile, outputFile, startFrame, endFrame, params);
    
    if (success) {
        std::cout << "\n✅ PROCESSING COMPLETED SUCCESSFULLY!" << std::endl;
        std::cout << "📁 Output file: " << outputFile << std::endl;
        if (startFrame >= 0 && endFrame >= 0) {
            std::cout << "📊 Processed Alembic frames: " << startFrame << "-" << endFrame 
                     << " (Maya equivalent: " << (startFrame + 1) << "-" << (endFrame + 1) << ")" << std::endl;
        }
        return 0;
    } else {
        std::cout << "\n❌ PROCESSING FAILED!" << std::endl;
        return 1;
    }
}