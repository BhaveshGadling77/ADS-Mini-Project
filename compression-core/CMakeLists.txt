cmake_minimum_required(VERSION 3.16)
project(adsc_compression_core VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O3)
endif()

include_directories(include)

# Core compression library
set(CORE_SOURCES
    src/io/BitStream.cpp
    src/algorithms/LZWCompressor.cpp
    src/algorithms/HuffmanCompressor.cpp
    src/algorithms/BWTCompressor.cpp
    src/algorithms/ImageCompressor.cpp
    src/algorithms/AudioCompressor.cpp
    src/algorithms/VideoCompressor.cpp
    src/core/Engine.cpp
)

add_library(adsc_core STATIC ${CORE_SOURCES})
target_include_directories(adsc_core PUBLIC include)

# CLI Executable
add_executable(compression-engine cli/main.cpp)
target_link_libraries(compression-engine PRIVATE adsc_core)

# Test Suite Executable
add_executable(test_runner tests/test_text_compression.cpp)
target_link_libraries(test_runner PRIVATE adsc_core)

# Output binaries to bin directory
set_target_properties(compression-engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(test_runner PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
