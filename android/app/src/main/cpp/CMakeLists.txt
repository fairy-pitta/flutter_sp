cmake_minimum_required(VERSION 3.31)
project(flutter_sp_native LANGUAGES C CXX)

# Flutter Mel Spectrogram Plugin

# This file is used by Flutter tools to build the native library for Android

# Set minimum Android API level
set(ANDROID_MIN_SDK_VERSION 21)
set(ANDROID_TARGET_SDK_VERSION 33)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required libraries
find_library(log-lib log)
find_library(android-lib android)
find_library(gles-lib GLESv2)
find_library(egl-lib EGL)
find_library(opensles-lib OpenSLES)

# Resolve project root and native paths relative to this CMake file
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../" ABSOLUTE)
set(NATIVE_DIR "${PROJECT_ROOT}/native")

# Include directories from parent project
include_directories(${NATIVE_DIR}/include)

# Core source files
set(CORE_SOURCES
    ${NATIVE_DIR}/src/mel_filter.cpp
    ${NATIVE_DIR}/src/mel_filter_bank.cpp
    ${NATIVE_DIR}/src/fft_processor.cpp
    ${NATIVE_DIR}/src/mel_spectrogram.cpp
    ${NATIVE_DIR}/src/audio_input.cpp
    ${NATIVE_DIR}/src/flutter_sp_native.cpp
    ${NATIVE_DIR}/src/texture_renderer.cpp
    ${NATIVE_DIR}/src/kiss_fft.c
)

# Create shared library
add_library(flutter_sp_native SHARED ${CORE_SOURCES})

# Link libraries
target_link_libraries(flutter_sp_native
    ${log-lib}
    ${android-lib}
    ${gles-lib}
    ${egl-lib}
    ${opensles-lib}
)

# Set properties
set_target_properties(flutter_sp_native PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
)

# Compiler flags
target_compile_options(flutter_sp_native PRIVATE
    -Wall
    -Wextra
    -fexceptions
    -frtti
)

# Android-specific definitions
target_compile_definitions(flutter_sp_native PRIVATE
    MOBILE_PLATFORM
)