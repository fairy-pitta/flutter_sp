#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

namespace audio {

enum class AudioFormat {
    S16_LE,      // 16-bit signed little-endian
    S24_LE,      // 24-bit signed little-endian
    S32_LE,      // 32-bit signed little-endian
    FLOAT32_LE   // 32-bit float little-endian
};

enum class Platform {
    IOS,
    ANDROID,
    MOCK   // For testing
};

struct AudioConfig {
    int sampleRate = 32000;
    int bufferSize = 1024;
    int numChannels = 1;
    AudioFormat format = AudioFormat::S16_LE;
    Platform platform = Platform::MOCK;
};

class AudioInput {
public:
    explicit AudioInput(const AudioConfig& config);
    ~AudioInput();
    
    // Initialization and control
    bool initialize();
    bool isInitialized() const { return initialized_; }
    
    // Permission handling
    bool requestPermission();
    bool hasPermission() const { return hasPermission_; }
    
    // Recording control
    bool startRecording();
    bool stopRecording();
    bool isRecording() const { return recording_; }
    
    // Configuration
    int getSampleRate() const { return config_.sampleRate; }
    int getBufferSize() const { return config_.bufferSize; }
    int getNumChannels() const { return config_.numChannels; }
    AudioFormat getFormat() const { return config_.format; }
    Platform getPlatform() const { return config_.platform; }
    
    // Audio callback
    using AudioCallback = std::function<void(const int16_t* data, size_t size)>;
    void setAudioCallback(AudioCallback callback);
    
    // Mock functionality for testing
    void injectMockData(const int16_t* data, size_t size);
    void simulatePermissionDenied(bool denied) { permissionDenied_ = denied; }
    
    // Statistics
    struct Stats {
        size_t framesProcessed = 0;
        size_t framesDropped = 0;
        float averageProcessingTimeMs = 0.0f;
        float currentFps = 0.0f;
    };
    Stats getStats() const;
    void resetStats();
    
private:
    // Core functionality
    bool validateConfiguration();
    void processingThread();
    void processAudioData(const int16_t* data, size_t size);
    
    // Platform-specific initialization
    bool initializeIOS();
    bool initializeAndroid();
    bool initializeMock();
    
    // Platform-specific cleanup
    void cleanupIOS();
    void cleanupAndroid();
    void cleanupMock();
    
    // Configuration
    AudioConfig config_;
    
    // State
    std::atomic<bool> initialized_{false};
    std::atomic<bool> recording_{false};
    std::atomic<bool> hasPermission_{false};
    std::atomic<bool> permissionDenied_{false};
    
    // Audio callback
    AudioCallback audioCallback_;
    std::mutex callbackMutex_;
    
    // Processing thread
    std::thread processingThread_;
    std::atomic<bool> shouldStop_{false};
    
    // Ring buffer for audio data
    std::vector<int16_t> ringBuffer_;
    size_t ringBufferSize_;
    size_t writeIndex_{0};
    size_t readIndex_{0};
    std::mutex bufferMutex_;
    std::condition_variable bufferCV_;
    
    // Mock data for testing
    std::vector<int16_t> mockData_;
    std::mutex mockDataMutex_;
    
    // Statistics
    mutable std::mutex statsMutex_;
    Stats stats_;
    std::chrono::steady_clock::time_point lastFrameTime_;
    
    // Platform-specific handles (opaque pointers)
    void* platformHandle_ = nullptr;
    void* platformAudioUnit_ = nullptr;
};

// Utility functions
const char* audioFormatToString(AudioFormat format);
const char* platformToString(Platform platform);
int getBytesPerSample(AudioFormat format);

} // namespace audio

#endif // AUDIO_INPUT_H