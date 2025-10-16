cmake_minimum_required(VERSION 3.10)
project(MelSpectrogramDSP)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz
)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/mel_filter.cpp
    src/mel_filter_bank.cpp
)

# Test executables
add_executable(mel_filter_test
    test/mel_filter_test.cpp
    ${SOURCES}
)

add_executable(mel_filter_bank_test
    test/mel_filter_bank_test.cpp
    ${SOURCES}
)

# Link Google Test
target_link_libraries(mel_filter_test gtest gtest_main)
target_link_libraries(mel_filter_bank_test gtest gtest_main)

# Enable testing
enable_testing()
add_test(NAME mel_filter_test COMMAND mel_filter_test)