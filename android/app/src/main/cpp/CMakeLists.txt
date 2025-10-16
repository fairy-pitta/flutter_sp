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

# Include directories from parent project
include_directories(${CMAKE_SOURCE_DIR}/native/include)
include_directories(${CMAKE_SOURCE_DIR}/native)

# Core source files
set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/native/src/mel_filter.cpp
    ${CMAKE_SOURCE_DIR}/native/src/mel_filter_bank.cpp
    ${CMAKE_SOURCE_DIR}/native/src/fft_processor.cpp
    ${CMAKE_SOURCE_DIR}/native/src/mel_spectrogram.cpp
    ${CMAKE_SOURCE_DIR}/native/src/audio_input.cpp
    ${CMAKE_SOURCE_DIR}/native/src/flutter_sp_native.cpp
    ${CMAKE_SOURCE_DIR}/native/src/kiss_fft.c
)

# Create shared library
add_library(flutter_sp_native SHARED ${CORE_SOURCES})

# Link libraries
target_link_libraries(flutter_sp_native
    ${log-lib}
    ${android-lib}
)

# Set properties
set_target_properties(flutter_sp_native PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    PREFIX ""
)

# Compiler flags
target_compile_options(flutter_sp_native PRIVATE
    -Wall
    -Wextra
    -Werror
    -fexceptions
    -frtti
)

# Android-specific definitions
target_compile_definitions(flutter_sp_native PRIVATE
    ANDROID
    MOBILE_PLATFORM
)