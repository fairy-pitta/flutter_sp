#include <gtest/gtest.h>
#include "audio_input.h"
#include <chrono>
#include <thread>
#include <vector>
#include <numeric>

using namespace audio;

class AudioInputTest : public ::testing::Test {
protected:
    void SetUp() override {
        config.sampleRate = 32000;
        config.bufferSize = 1024;
        config.numChannels = 1;
        config.format = AudioFormat::S16_LE;
        config.platform = Platform::MOCK;
        
        audioInput = std::make_unique<AudioInput>(config);
    }
    
    void generateTestSignal(std::vector<int16_t>& signal, float frequency, float amplitude = 0.5f) {
        for (int i = 0; i < signal.size(); ++i) {
            float t = static_cast<float>(i) / config.sampleRate;
            signal[i] = static_cast<int16_t>(amplitude * 32767.0f * std::sin(2.0f * M_PI * frequency * t));
        }
    }
    
    AudioConfig config;
    std::unique_ptr<AudioInput> audioInput;
    bool callbackInvoked = false;
    std::vector<int16_t> capturedData;
};

// Test 1: Basic initialization
TEST_F(AudioInputTest, InitializationTest) {
    EXPECT_NE(audioInput, nullptr);
    EXPECT_EQ(audioInput->getSampleRate(), config.sampleRate);
    EXPECT_EQ(audioInput->getBufferSize(), config.bufferSize);
    EXPECT_EQ(audioInput->getNumChannels(), config.numChannels);
    EXPECT_FALSE(audioInput->isRecording());
}

// Test 2: Configuration validation
TEST_F(AudioInputTest, ConfigurationValidationTest) {
    // Test invalid sample rate
    AudioConfig invalidConfig = config;
    invalidConfig.sampleRate = 0;
    AudioInput invalidInput(invalidConfig);
    EXPECT_FALSE(invalidInput.initialize());
    
    // Test invalid buffer size
    invalidConfig = config;
    invalidConfig.bufferSize = 0;
    AudioInput invalidBufferInput(invalidConfig);
    EXPECT_FALSE(invalidBufferInput.initialize());
}

// Test 3: Permission handling (mock)
TEST_F(AudioInputTest, PermissionHandlingTest) {
    // In mock mode, should always return true
    EXPECT_TRUE(audioInput->requestPermission());
    
    // Test permission denied scenario
    audioInput->simulatePermissionDenied(true);
    EXPECT_FALSE(audioInput->requestPermission());
}

// Test 4: Device initialization
TEST_F(AudioInputTest, DeviceInitializationTest) {
    EXPECT_TRUE(audioInput->initialize());
    EXPECT_TRUE(audioInput->isInitialized());
    
    // Test double initialization
    EXPECT_TRUE(audioInput->initialize());
}

// Test 5: Start/stop recording
TEST_F(AudioInputTest, RecordingControlTest) {
    ASSERT_TRUE(audioInput->initialize());
    
    EXPECT_FALSE(audioInput->isRecording());
    EXPECT_TRUE(audioInput->startRecording());
    EXPECT_TRUE(audioInput->isRecording());
    
    // Test double start
    EXPECT_TRUE(audioInput->startRecording());
    
    EXPECT_TRUE(audioInput->stopRecording());
    EXPECT_FALSE(audioInput->isRecording());
    
    // Test stop when not recording
    EXPECT_TRUE(audioInput->stopRecording());
}

// Test 6: Audio callback functionality
TEST_F(AudioInputTest, AudioCallbackTest) {
    ASSERT_TRUE(audioInput->initialize());
    
    bool callbackInvoked = false;
    std::vector<int16_t> capturedData;
    
    audioInput->setAudioCallback([&](const int16_t* data, size_t size) {
        callbackInvoked = true;
        capturedData.assign(data, data + size);
    });
    
    EXPECT_TRUE(audioInput->startRecording());
    
    // Simulate some audio data in mock mode
    std::vector<int16_t> testData(config.bufferSize);
    generateTestSignal(testData, 1000.0f);
    audioInput->injectMockData(testData.data(), testData.size());
    
    // Give some time for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(capturedData.size(), config.bufferSize);
    
    audioInput->stopRecording();
}

// Test 7: Buffer management
TEST_F(AudioInputTest, BufferManagementTest) {
    ASSERT_TRUE(audioInput->initialize());
    
    // Test ring buffer functionality
    std::vector<int16_t> largeData(config.bufferSize * 3);
    generateTestSignal(largeData, 440.0f);
    
    // Inject data larger than buffer
    audioInput->injectMockData(largeData.data(), largeData.size());
    
    // Should handle without crashing
    EXPECT_TRUE(audioInput->startRecording());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    audioInput->stopRecording();
}

// Test 8: Performance requirements
TEST_F(AudioInputTest, PerformanceTest) {
    ASSERT_TRUE(audioInput->initialize());
    
    std::vector<int16_t> testData(config.bufferSize);
    generateTestSignal(testData, 880.0f);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Simulate 100 callbacks
    for (int i = 0; i < 100; ++i) {
        audioInput->injectMockData(testData.data(), testData.size());
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    // Should process 100 frames in reasonable time
    EXPECT_LT(duration.count(), 50000); // 50ms total
}

// Test 9: Error handling
TEST_F(AudioInputTest, ErrorHandlingTest) {
    // Test operations without initialization
    AudioInput uninitializedInput(config);
    EXPECT_FALSE(uninitializedInput.startRecording());
    EXPECT_FALSE(uninitializedInput.stopRecording());
    
    // Test with invalid configuration (zero buffer size)
    AudioConfig badConfig = config;
    badConfig.bufferSize = 0;
    AudioInput badInput(badConfig);
    EXPECT_FALSE(badInput.initialize());
}

// Test 10: Thread safety
TEST_F(AudioInputTest, ThreadSafetyTest) {
    ASSERT_TRUE(audioInput->initialize());
    
    std::atomic<int> callbackCount{0};
    std::mutex dataMutex;
    std::vector<int16_t> sharedData;
    
    audioInput->setAudioCallback([&](const int16_t* data, size_t size) {
        std::lock_guard<std::mutex> lock(dataMutex);
        callbackCount++;
        sharedData.assign(data, data + size);
    });
    
    EXPECT_TRUE(audioInput->startRecording());
    
    // Inject data from multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&, i]() {
            std::vector<int16_t> threadData(config.bufferSize);
            generateTestSignal(threadData, 200.0f + i * 100.0f);
            audioInput->injectMockData(threadData.data(), threadData.size());
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    audioInput->stopRecording();
    
    // Should have processed all data without crashes
    EXPECT_GT(callbackCount.load(), 0);
}

// Test 11: Different audio formats
TEST_F(AudioInputTest, AudioFormatTest) {
    // Test S16_LE format (default)
    EXPECT_TRUE(audioInput->initialize());
    EXPECT_EQ(audioInput->getFormat(), AudioFormat::S16_LE);
    
    // Test other formats
    AudioConfig floatConfig = config;
    floatConfig.format = AudioFormat::FLOAT32_LE;
    AudioInput floatInput(floatConfig);
    EXPECT_TRUE(floatInput.initialize());
    EXPECT_EQ(floatInput.getFormat(), AudioFormat::FLOAT32_LE);
}

// Test 12: Platform-specific behavior
TEST_F(AudioInputTest, PlatformSpecificTest) {
    // Test iOS platform
    AudioConfig iosConfig = config;
    iosConfig.platform = Platform::IOS;
    AudioInput iosInput(iosConfig);
    EXPECT_TRUE(iosInput.initialize());
    
    // Test Android platform
    AudioConfig androidConfig = config;
    androidConfig.platform = Platform::Android;
    AudioInput androidInput(androidConfig);
    EXPECT_TRUE(androidInput.initialize());
    
    // Should work with different platforms
    EXPECT_EQ(iosInput.getPlatform(), Platform::IOS);
    EXPECT_EQ(androidInput.getPlatform(), Platform::Android);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}