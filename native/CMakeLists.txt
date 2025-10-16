cmake_minimum_required(VERSION 3.10)
project(MelSpectrogram VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find OpenGL libraries
find_package(OpenGL REQUIRED)
find_library(GLES3_LIB NAMES GLESv3 GLESv2)

# Core source files (no OpenGL dependencies)
set(CORE_SOURCES
    src/mel_filter.cpp
    src/mel_filter_bank.cpp
    src/fft_processor.cpp
    src/mel_spectrogram.cpp
    src/audio_input.cpp
    src/kiss_fft.c
)

# Full source files (including OpenGL)
set(ALL_SOURCES ${CORE_SOURCES} src/texture_renderer.cpp)

# Test executables
add_executable(mel_filter_test test/mel_filter_test.cpp ${CORE_SOURCES})
add_executable(mel_filter_bank_test test/mel_filter_bank_test.cpp ${CORE_SOURCES})
add_executable(fft_processor_test test/fft_processor_test.cpp ${CORE_SOURCES})
add_executable(mel_spectrogram_test test/mel_spectrogram_test.cpp ${CORE_SOURCES})
add_executable(texture_renderer_test test/texture_renderer_test.cpp ${ALL_SOURCES})
add_executable(audio_input_test test/audio_input_test.cpp ${CORE_SOURCES})

# Link Google Test
target_link_libraries(mel_filter_test gtest gtest_main)
target_link_libraries(mel_filter_bank_test gtest gtest_main)
target_link_libraries(fft_processor_test gtest gtest_main)
target_link_libraries(mel_spectrogram_test gtest gtest_main)
target_link_libraries(texture_renderer_test gtest gtest_main)
target_link_libraries(audio_input_test gtest gtest_main)

# Link OpenGL libraries for texture renderer
target_link_libraries(texture_renderer_test ${OPENGL_LIBRARIES})
if(GLES3_LIB)
    target_link_libraries(texture_renderer_test ${GLES3_LIB})
endif()

# Silence OpenGL deprecation warnings on macOS
if(APPLE)
    target_compile_definitions(texture_renderer_test PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Enable testing
enable_testing()
add_test(NAME mel_filter_test COMMAND mel_filter_test)
add_test(NAME mel_filter_bank_test COMMAND mel_filter_bank_test)
add_test(NAME fft_processor_test COMMAND fft_processor_test)
add_test(NAME mel_spectrogram_test COMMAND mel_spectrogram_test)
add_test(NAME texture_renderer_test COMMAND texture_renderer_test)
add_test(NAME audio_input_test COMMAND audio_input_test)