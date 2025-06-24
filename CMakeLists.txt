cmake_minimum_required(VERSION 3.16)
project(AlembicDenoiser)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg 툴체인 사용
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Alembic 패키지 찾기
find_package(Alembic CONFIG REQUIRED)

# 실행 파일 생성
add_executable(BilateralMeshDenoiser src/BilateralMeshDenoiser.cpp)

# Alembic 라이브러리 링크
target_link_libraries(BilateralMeshDenoiser PRIVATE Alembic::Alembic)

# Windows에서 필요한 추가 라이브러리
if(WIN32)
    target_link_libraries(BilateralMeshDenoiser PRIVATE ws2_32)
endif()

# OpenMP 지원 (선택사항)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(BilateralMeshDenoiser PRIVATE OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found - multi-threading enabled")
else()
    message(STATUS "OpenMP not found - single-threaded processing")
endif()

# 디버그 정보 포함
if(MSVC)
    target_compile_options(BilateralMeshDenoiser PRIVATE /W3)
    set_property(TARGET BilateralMeshDenoiser PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# 새로운 실행 파일: TemporalMeshDenoiser
add_executable(TemporalMeshDenoiser src/TemporalMeshDenoiser.cpp)

# Alembic 라이브러리 링크
target_link_libraries(TemporalMeshDenoiser PRIVATE Alembic::Alembic)

# Windows에서 필요한 추가 라이브러리
if(WIN32)
    target_link_libraries(TemporalMeshDenoiser PRIVATE ws2_32)
endif()

# OpenMP 지원
if(OpenMP_CXX_FOUND)
    target_link_libraries(TemporalMeshDenoiser PRIVATE OpenMP::OpenMP_CXX)
endif()

# 디버그 정보 포함
if(MSVC)
    target_compile_options(TemporalMeshDenoiser PRIVATE /W3)
    set_property(TARGET TemporalMeshDenoiser PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()