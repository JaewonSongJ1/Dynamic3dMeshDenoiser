# CMakeLists.txt for Dynamic 3D Mesh Denoiser
# Author: Jaewon Song, Dexter Studios
# Collaborator: Minyeong Jeong

cmake_minimum_required(VERSION 3.16)
project(Dynamic3DMeshDenoiser VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Static linking configuration for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring for RELEASE build with static linking")
    
    # Force static runtime on Windows
    if(WIN32)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        
        # Additional static linking flags
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
        
        # Compiler optimizations
        add_compile_options(/O2 /Ob2 /Ot /GL)
    endif()
    
    # Prefer static libraries
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
    
else()
    message(STATUS "Configuring for DEVELOPMENT build with dynamic linking")
endif()

# Find required packages
find_package(Alembic REQUIRED)

# Find OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found - multi-threading enabled")
else()
    message(WARNING "OpenMP not found - single-threaded build")
endif()

# Common compile features and definitions
add_compile_definitions(
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0601>
    $<$<PLATFORM_ID:Windows>:NOMINMAX>
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
)

# Function to create executable with common settings
function(create_mesh_denoiser_executable target_name source_file)
    add_executable(${target_name} ${source_file})
    
    # Set UTF-8 encoding for source files (fixes MSVC warnings)
    if(WIN32)
        target_compile_options(${target_name} PRIVATE /utf-8)
    endif()
    
    # Link libraries
    target_link_libraries(${target_name} 
        PRIVATE 
            Alembic::Alembic
    )
    
    # Link OpenMP if available
    if(OpenMP_CXX_FOUND)
        target_link_libraries(${target_name} PRIVATE OpenMP::OpenMP_CXX)
        target_compile_definitions(${target_name} PRIVATE _OPENMP)
    endif()
    
    # Windows-specific libraries
    if(WIN32)
        target_link_libraries(${target_name} PRIVATE ws2_32)
    endif()
    
    # Static linking for release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release" AND WIN32)
        # Force static linking of runtime
        set_property(TARGET ${target_name} PROPERTY 
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
            
        # Additional static linking options
        target_link_options(${target_name} PRIVATE 
            $<$<CONFIG:Release>:/LTCG>
            /INCREMENTAL:NO
        )
    endif()
    
    # Set output directory
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
    )
    
    message(STATUS "Configured executable: ${target_name}")
endfunction()

# Create executables
create_mesh_denoiser_executable(BilateralMeshDenoiser "src/BilateralMeshDenoiser.cpp")
create_mesh_denoiser_executable(TemporalMeshDenoiser "src/TemporalMeshDenoiser.cpp")

# Print build information
message(STATUS "=== Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Linking: STATIC (standalone deployment)")
    message(STATUS "Runtime: Static CRT")
else()
    message(STATUS "Linking: DYNAMIC (fast development)")
endif()

message(STATUS "OpenMP: ${OpenMP_CXX_FOUND}")
message(STATUS "Output Directory: ${CMAKE_BINARY_DIR}/$<CONFIG>")
message(STATUS "============================")