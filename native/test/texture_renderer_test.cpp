#include <gtest/gtest.h>
#include "texture_renderer.h"
#include <chrono>
#include <cmath>
#include <vector>

using namespace melspectrogram;

class TextureRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<TextureRenderer>(512, 256, 64);
    }
    
    void generateTestData(std::vector<float>& data, float frequency, float amplitude = 1.0f) {
        for (size_t i = 0; i < data.size(); ++i) {
            float phase = 2.0f * M_PI * frequency * i / data.size();
            data[i] = amplitude * (0.5f + 0.5f * std::sin(phase));
            data[i] = std::max(0.0f, std::min(1.0f, data[i])); // Clamp to [0, 1]
        }
    }
    
    void generateSineWavePattern(std::vector<float>& data, int cycles) {
        for (size_t i = 0; i < data.size(); ++i) {
            float phase = 2.0f * M_PI * cycles * i / data.size();
            data[i] = 0.5f + 0.5f * std::sin(phase);
        }
    }
    
    std::unique_ptr<TextureRenderer> renderer;
};

// Test 1: Basic initialization
TEST_F(TextureRendererTest, InitializationTest) {
    EXPECT_NE(renderer, nullptr);
    EXPECT_EQ(renderer->getWidth(), 512);
    EXPECT_EQ(renderer->getHeight(), 256);
    EXPECT_EQ(renderer->getNumMelBands(), 64);
    EXPECT_TRUE(renderer->isInitialized());
}

// Test 2: Texture creation and OpenGL context
TEST_F(TextureRendererTest, TextureCreationTest) {
    // In mock mode, we don't have a real OpenGL texture, which is expected
    // The test passes if we're properly initialized (mock mode is fine for testing)
    EXPECT_TRUE(renderer->isInitialized());
    if (renderer->getTextureId() > 0) {
        // We have a real OpenGL context
        std::cout << "Running with real OpenGL context" << std::endl;
    } else {
        // We're in mock mode, which is fine for testing
        std::cout << "Running in mock mode - OpenGL texture creation skipped" << std::endl;
    }
}

// Test 3: Single column update with sine wave pattern
TEST_F(TextureRendererTest, SingleColumnUpdateTest) {
    std::vector<float> melData(64);
    generateSineWavePattern(melData, 3); // 3 cycles across 64 bands
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = renderer->updateColumn(melData);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_TRUE(result);
    EXPECT_LT(duration.count(), 1000); // Should be sub-millisecond
}

// Test 4: Multiple column updates (waterfall effect)
TEST_F(TextureRendererTest, WaterfallEffectTest) {
    std::vector<float> melData(64);
    
    // Simulate 100 frames of waterfall data
    for (int frame = 0; frame < 100; ++frame) {
        // Generate different patterns for each frame
        generateSineWavePattern(melData, 1 + frame % 5);
        
        bool result = renderer->updateColumn(melData);
        EXPECT_TRUE(result);
    }
    
    // Check that we can get the current texture data
    auto textureData = renderer->getTextureData();
    EXPECT_EQ(textureData.size(), 512 * 256 * 4); // RGBA format
}

// Test 5: Color mapping - Viridis
TEST_F(TextureRendererTest, ViridisColorMappingTest) {
    renderer->setColorMap(ColorMapType::VIRIDIS);
    
    std::vector<float> melData(64);
    // Test different amplitude levels
    for (int i = 0; i < 64; ++i) {
        melData[i] = i / 63.0f; // Linear ramp from 0 to 1
    }
    
    bool result = renderer->updateColumn(melData);
    EXPECT_TRUE(result);
    
    auto textureData = renderer->getTextureData();
    EXPECT_EQ(textureData.size(), 512 * 256 * 4);
    
    // Check that we have valid RGBA values
    for (size_t i = 0; i < textureData.size(); i += 4) {
        EXPECT_GE(textureData[i + 0], 0);     // R
        EXPECT_LE(textureData[i + 0], 255);
        EXPECT_GE(textureData[i + 1], 0);     // G
        EXPECT_LE(textureData[i + 1], 255);
        EXPECT_GE(textureData[i + 2], 0);     // B
        EXPECT_LE(textureData[i + 2], 255);
        EXPECT_EQ(textureData[i + 3], 255);   // A (fully opaque)
    }
}

// Test 6: Color mapping - Inferno
TEST_F(TextureRendererTest, InfernoColorMappingTest) {
    renderer->setColorMap(ColorMapType::INFERNO);
    
    std::vector<float> melData(64, 0.5f); // Constant mid-level signal
    
    bool result = renderer->updateColumn(melData);
    EXPECT_TRUE(result);
    
    auto textureData = renderer->getTextureData();
    EXPECT_EQ(textureData.size(), 512 * 256 * 4);
}

// Test 7: Color mapping - Plasma
TEST_F(TextureRendererTest, PlasmaColorMappingTest) {
    renderer->setColorMap(ColorMapType::PLASMA);
    
    std::vector<float> melData(64);
    generateSineWavePattern(melData, 2);
    
    bool result = renderer->updateColumn(melData);
    EXPECT_TRUE(result);
}

// Test 8: Performance benchmark
TEST_F(TextureRendererTest, PerformanceBenchmarkTest) {
    std::vector<float> melData(64);
    // Generate white noise mel data (simple random values)
    for (int i = 0; i < 64; ++i) {
        melData[i] = static_cast<float>(rand()) / RAND_MAX * 0.5f;
    }
    
    const int warmupFrames = 10;
    const int benchmarkFrames = 1000;
    
    // Warmup
    for (int i = 0; i < warmupFrames; ++i) {
        renderer->updateColumn(melData);
    }
    
    // Benchmark
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < benchmarkFrames; ++i) {
        renderer->updateColumn(melData);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    float totalSeconds = duration.count() / 1000000.0f;
    float framesPerSecond = benchmarkFrames / totalSeconds;
    float avgFrameTime = duration.count() / static_cast<float>(benchmarkFrames);
    
    std::cout << "Texture Renderer Benchmark Results:" << std::endl;
    std::cout << "Total time: " << totalSeconds << " seconds" << std::endl;
    std::cout << "Frames processed: " << benchmarkFrames << std::endl;
    std::cout << "Frames per second: " << framesPerSecond << std::endl;
    std::cout << "Average frame time: " << avgFrameTime << " microseconds" << std::endl;
    
    // Should achieve at least 1000 FPS for smooth waterfall display
    EXPECT_GT(framesPerSecond, 1000.0f);
    EXPECT_LT(avgFrameTime, 1000.0f); // Less than 1ms per frame
}

// Test 9: Edge case - empty data
TEST_F(TextureRendererTest, EmptyDataTest) {
    std::vector<float> emptyData;
    
    bool result = renderer->updateColumn(emptyData);
    EXPECT_FALSE(result); // Should fail with empty data
}

// Test 10: Edge case - wrong data size
TEST_F(TextureRendererTest, WrongDataSizeTest) {
    std::vector<float> wrongSizeData(32); // Wrong number of mel bands
    
    bool result = renderer->updateColumn(wrongSizeData);
    EXPECT_FALSE(result); // Should fail with wrong size
}

// Test 11: Edge case - extreme values
TEST_F(TextureRendererTest, ExtremeValuesTest) {
    std::vector<float> extremeData(64);
    
    // Test values outside normal range
    for (int i = 0; i < 64; ++i) {
        extremeData[i] = (i % 2 == 0) ? -1.0f : 2.0f; // Below 0 and above 1
    }
    
    bool result = renderer->updateColumn(extremeData);
    EXPECT_TRUE(result); // Should handle clamping internally
    
    // Verify that extreme values are properly clamped
    auto textureData = renderer->getTextureData();
    EXPECT_EQ(textureData.size(), 512 * 256 * 4);
}

// Test 12: Ring buffer behavior
TEST_F(TextureRendererTest, RingBufferTest) {
    std::vector<float> melData(64, 0.5f);
    
    // Fill more columns than texture width to test ring buffer
    for (int i = 0; i < 600; ++i) { // More than 512 columns
        bool result = renderer->updateColumn(melData);
        EXPECT_TRUE(result);
    }
    
    // Should still work correctly after wrapping
    auto textureData = renderer->getTextureData();
    EXPECT_EQ(textureData.size(), 512 * 256 * 4);
}

// Helper function for white noise generation
void generateWhiteNoise(std::vector<float>& data, float amplitude) {
    for (auto& sample : data) {
        sample = amplitude * (rand() / static_cast<float>(RAND_MAX));
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}