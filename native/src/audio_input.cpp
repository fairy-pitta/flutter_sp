#include "audio_input.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>

namespace audio {

AudioInput::AudioInput(const AudioConfig& config) 
    : config_(config), ringBufferSize_(config.bufferSize * 8) {
    ringBuffer_.resize(ringBufferSize_);
    lastFrameTime_ = std::chrono::steady_clock::now();
}

AudioInput::~AudioInput() {
    stopRecording();
    
    if (initialized_) {
        switch (config_.platform) {
            case Platform::IOS:
                cleanupIOS();
                break;
            case Platform::Android:
                cleanupAndroid();
                break;
            case Platform::MOCK:
                cleanupMock();
                break;
        }
    }
}

bool AudioInput::initialize() {
    if (initialized_) {
        return true;
    }
    
    if (!validateConfiguration()) {
        return false;
    }
    
    bool success = false;
    switch (config_.platform) {
        case Platform::IOS:
            success = initializeIOS();
            break;
        case Platform::Android:
            success = initializeAndroid();
            break;
        case Platform::MOCK:
            success = initializeMock();
            break;
    }
    
    if (success) {
        initialized_ = true;
        hasPermission_ = true; // Assume permission granted after successful init
    }
    
    return success;
}

bool AudioInput::validateConfiguration() {
    if (config_.sampleRate <= 0) {
        return false;
    }
    
    if (config_.bufferSize <= 0) {
        return false;
    }
    
    if (config_.numChannels <= 0 || config_.numChannels > 8) {
        return false;
    }
    
    return true;
}

bool AudioInput::requestPermission() {
    if (permissionDenied_) {
        hasPermission_ = false;
        return false;
    }
    
    // In real implementation, this would request platform-specific permissions
    hasPermission_ = true;
    return true;
}

bool AudioInput::startRecording() {
    if (!initialized_ || !hasPermission_) {
        return false;
    }
    
    if (recording_) {
        return true; // Already recording
    }
    
    recording_ = true;
    shouldStop_ = false;
    
    // Start processing thread for mock mode
    if (config_.platform == Platform::MOCK) {
        processingThread_ = std::thread(&AudioInput::processingThread, this);
    }
    
    return true;
}

bool AudioInput::stopRecording() {
    if (!initialized_) {
        return false;
    }
    
    if (!recording_) {
        return true; // Already stopped
    }
    
    recording_ = false;
    shouldStop_ = true;
    bufferCV_.notify_all();
    
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
    
    return true;
}

void AudioInput::setAudioCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    audioCallback_ = callback;
}

void AudioInput::injectMockData(const int16_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(mockDataMutex_);
    mockData_.assign(data, data + size);
    bufferCV_.notify_one();
}

AudioInput::Stats AudioInput::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

void AudioInput::resetStats() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    stats_ = Stats{};
    lastFrameTime_ = std::chrono::steady_clock::now();
}

void AudioInput::processingThread() {
    std::vector<int16_t> buffer(config_.bufferSize);
    
    while (!shouldStop_) {
        std::unique_lock<std::mutex> lock(mockDataMutex_);
        bufferCV_.wait_for(lock, std::chrono::milliseconds(10), 
                          [this] { return !mockData_.empty() || shouldStop_; });
        
        if (shouldStop_) {
            break;
        }
        
        if (!mockData_.empty()) {
            // Process available mock data
            size_t dataToProcess = std::min(mockData_.size(), buffer.size());
            std::memcpy(buffer.data(), mockData_.data(), dataToProcess * sizeof(int16_t));
            
            // Pad with zeros if needed
            if (dataToProcess < buffer.size()) {
                std::memset(buffer.data() + dataToProcess, 0, 
                           (buffer.size() - dataToProcess) * sizeof(int16_t));
            }
            
            // Remove processed data
            mockData_.erase(mockData_.begin(), mockData_.begin() + dataToProcess);
            
            lock.unlock();
            
            // Process the audio data
            processAudioData(buffer.data(), buffer.size());
        } else {
            // Generate silence if no data available
            lock.unlock();
            std::memset(buffer.data(), 0, buffer.size() * sizeof(int16_t));
            processAudioData(buffer.data(), buffer.size());
        }
        
        // Simulate real-time audio timing
        std::this_thread::sleep_for(std::chrono::microseconds(
            static_cast<int>(config_.bufferSize * 1000000.0 / config_.sampleRate)));
    }
}

void AudioInput::processAudioData(const int16_t* data, size_t size) {
    auto startTime = std::chrono::steady_clock::now();
    
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (audioCallback_) {
            audioCallback_(data, size);
        }
    }
    
    // Update statistics
    auto endTime = std::chrono::steady_clock::now();
    auto processingTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    {
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats_.framesProcessed++;
        
        // Update average processing time
        float currentTimeMs = processingTime.count() / 1000.0f;
        stats_.averageProcessingTimeMs = 
            (stats_.averageProcessingTimeMs * (stats_.framesProcessed - 1) + currentTimeMs) / 
            stats_.framesProcessed;
        
        // Calculate FPS
        auto timeSinceLastFrame = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - lastFrameTime_).count();
        if (timeSinceLastFrame > 0) {
            stats_.currentFps = 1000.0f / timeSinceLastFrame;
        }
        
        lastFrameTime_ = endTime;
    }
}

// Platform-specific implementations
bool AudioInput::initializeIOS() {
    // iOS-specific initialization would go here
    // For now, just return success in mock mode
    return true;
}

bool AudioInput::initializeAndroid() {
    // Android-specific initialization would go here
    // For now, just return success in mock mode
    return true;
}

bool AudioInput::initializeMock() {
    // Mock initialization - always succeeds
    return true;
}

void AudioInput::cleanupIOS() {
    // iOS-specific cleanup would go here
}

void AudioInput::cleanupAndroid() {
    // Android-specific cleanup would go here
}

void AudioInput::cleanupMock() {
    // Mock cleanup - nothing to do
}

// Utility functions
const char* audioFormatToString(AudioFormat format) {
    switch (format) {
        case AudioFormat::S16_LE: return "S16_LE";
        case AudioFormat::S24_LE: return "S24_LE";
        case AudioFormat::S32_LE: return "S32_LE";
        case AudioFormat::FLOAT32_LE: return "FLOAT32_LE";
        default: return "UNKNOWN";
    }
}

const char* platformToString(Platform platform) {
    switch (platform) {
        case Platform::IOS: return "iOS";
        case Platform::Android: return "Android";
        case Platform::MOCK: return "Mock";
        default: return "Unknown";
    }
}

int getBytesPerSample(AudioFormat format) {
    switch (format) {
        case AudioFormat::S16_LE: return 2;
        case AudioFormat::S24_LE: return 3;
        case AudioFormat::S32_LE: return 4;
        case AudioFormat::FLOAT32_LE: return 4;
        default: return 2;
    }
}

} // namespace audio