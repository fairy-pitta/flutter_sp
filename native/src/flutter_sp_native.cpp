#include "audio_input.h"
#include "mel_spectrogram.h"
#include "texture_renderer.h"
#include <memory>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <mutex>

// Global instances
static std::unique_ptr<audio::AudioInput> g_audioInput;
static std::unique_ptr<melspectrogram::MelSpectrogramProcessor> g_melProcessor;
static std::unique_ptr<melspectrogram::TextureRenderer> g_textureRenderer;

// Error handling
static char g_lastError[256] = {0};
static std::mutex g_mutex;

extern "C" {

// Audio Input Functions
int init_audio_input(const audio::AudioConfig* config) {
    try {
        g_audioInput = std::make_unique<audio::AudioInput>(*config);
        return g_audioInput->initialize() ? 0 : -1;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

int start_recording() {
    if (!g_audioInput) {
        strncpy(g_lastError, "Audio input not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        return g_audioInput->startRecording() ? 0 : -1;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

int stop_recording() {
    if (!g_audioInput) {
        strncpy(g_lastError, "Audio input not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        return g_audioInput->stopRecording() ? 0 : -1;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

// Mel Processor Functions
int init_mel_processor(const melspectrogram::AudioConfig* config) {
    try {
        g_melProcessor = std::make_unique<melspectrogram::MelSpectrogramProcessor>(*config);
        return g_melProcessor->processAudioFrame(nullptr, 0) ? 0 : -1;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

int process_audio_frame(const int16_t* inputBuffer, int bufferSize, 
                       float* outputBuffer, int outputSize) {
    if (!g_melProcessor) {
        strncpy(g_lastError, "Mel processor not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        if (!g_melProcessor->processAudioFrame(inputBuffer, bufferSize)) {
            strncpy(g_lastError, "Failed to process audio frame", sizeof(g_lastError) - 1);
            return -1;
        }
        
        auto melSpectrum = g_melProcessor->getMelSpectrum();
        
        if (static_cast<int>(melSpectrum.size()) > outputSize) {
            strncpy(g_lastError, "Output buffer too small", sizeof(g_lastError) - 1);
            return -1;
        }
        
        std::copy(melSpectrum.begin(), melSpectrum.end(), outputBuffer);
        return static_cast<int>(melSpectrum.size());
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

// Texture Renderer Functions
int init_texture_renderer(int width, int height, int numMelBands) {
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_textureRenderer = std::make_unique<melspectrogram::TextureRenderer>(width, height, numMelBands);
        
        const bool ok = g_textureRenderer->initialize();
        if (!ok) {
#ifdef __APPLE__
            strncpy(g_lastError, "OpenGL context not available on macOS. Flutter owns the GL context; using software rendering (fallback).", sizeof(g_lastError) - 1);
#else
            strncpy(g_lastError, "OpenGL context not available or texture creation failed.", sizeof(g_lastError) - 1);
#endif
            g_textureRenderer.reset();
            return -1;
        }
        return 0;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

int update_texture_column(const float* melData, int dataSize) {
    if (!g_textureRenderer) {
        strncpy(g_lastError, "Texture renderer not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        std::vector<float> melDataVec(melData, melData + dataSize);
        const bool ok = g_textureRenderer->updateColumn(melDataVec);
        if (!ok) {
            strncpy(g_lastError, "Failed to update texture column (invalid data size or not initialized).", sizeof(g_lastError) - 1);
            return -1;
        }
        return 0;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

unsigned int get_texture_id() {
    if (!g_textureRenderer) {
        strncpy(g_lastError, "Texture renderer not initialized", sizeof(g_lastError) - 1);
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_textureRenderer->getTextureId();
}

int get_texture_data(uint8_t* buffer, int bufferSize) {
    if (!g_textureRenderer) {
        strncpy(g_lastError, "Texture renderer not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto data = g_textureRenderer->getTextureData();
        
        if (static_cast<int>(data.size()) > bufferSize) {
            strncpy(g_lastError, "Buffer too small", sizeof(g_lastError) - 1);
            return -1;
        }
        
        std::copy(data.begin(), data.end(), buffer);
        return static_cast<int>(data.size());
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

int set_texture_color_map(int colorMapType) {
    if (!g_textureRenderer) {
        strncpy(g_lastError, "Texture renderer not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_textureRenderer->setColorMap(static_cast<melspectrogram::ColorMapType>(colorMapType));
        return 0;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

int set_texture_min_max(float minValue, float maxValue) {
    if (!g_textureRenderer) {
        strncpy(g_lastError, "Texture renderer not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_textureRenderer->setMinMaxValues(minValue, maxValue);
        return 0;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

// Utility Functions
const char* get_error_message() {
    return g_lastError;
}

int get_texture_width() {
    if (!g_textureRenderer) return 0;
    return g_textureRenderer->getWidth();
}

int get_texture_height() {
    if (!g_textureRenderer) return 0;
    return g_textureRenderer->getHeight();
}

int get_texture_num_mel_bands() {
    if (!g_textureRenderer) return 0;
    return g_textureRenderer->getNumMelBands();
}

int get_texture_current_column() {
    if (!g_textureRenderer) return 0;
    return g_textureRenderer->getCurrentColumn();
}

float get_texture_last_update_time_ms() {
    if (!g_textureRenderer) return 0.0f;
    return g_textureRenderer->getLastUpdateTimeMs();
}

void cleanup() {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_audioInput.reset();
    g_melProcessor.reset();
    g_textureRenderer.reset();
    g_lastError[0] = '\0';
}

} // extern "C"