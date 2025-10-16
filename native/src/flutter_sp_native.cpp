#include "audio_input.h"
#include "mel_spectrogram.h"
#include "texture_renderer.h"
#include <memory>
#include <cstring>
#include <cstdlib>

// Global instances
static std::unique_ptr<AudioInput> g_audioInput;
static std::unique_ptr<MelSpectrogramProcessor> g_melProcessor;
static std::unique_ptr<TextureRenderer> g_textureRenderer;

// Error handling
static char g_lastError[256] = {0};

extern "C" {

// Audio Input Functions
int init_audio_input(const AudioConfig* config) {
    try {
        g_audioInput = std::make_unique<AudioInput>(*config);
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
int init_mel_processor(const MelConfig* config) {
    try {
        g_melProcessor = std::make_unique<MelSpectrogramProcessor>(*config);
        return g_melProcessor->initialize() ? 0 : -1;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

int process_audio_frame(const float* inputBuffer, int bufferSize, 
                       float* outputBuffer, int outputSize) {
    if (!g_melProcessor) {
        strncpy(g_lastError, "Mel processor not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        std::vector<float> input(inputBuffer, inputBuffer + bufferSize);
        auto result = g_melProcessor->processFrame(input);
        
        if (result.size() != static_cast<size_t>(outputSize)) {
            strncpy(g_lastError, "Output size mismatch", sizeof(g_lastError) - 1);
            return -1;
        }
        
        std::copy(result.begin(), result.end(), outputBuffer);
        return 0;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

// Texture Renderer Functions
int init_texture_renderer(const TextureConfig* config) {
    try {
        g_textureRenderer = std::make_unique<TextureRenderer>(*config);
        return g_textureRenderer->initialize() ? 0 : -1;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

int update_texture(int textureId, const float* melData, int dataSize) {
    if (!g_textureRenderer) {
        strncpy(g_lastError, "Texture renderer not initialized", sizeof(g_lastError) - 1);
        return -1;
    }
    
    try {
        std::vector<float> melDataVec(melData, melData + dataSize);
        return g_textureRenderer->updateTexture(textureId, melDataVec) ? 0 : -1;
    } catch (const std::exception& e) {
        strncpy(g_lastError, e.what(), sizeof(g_lastError) - 1);
        return -1;
    }
}

// Utility Functions
const char* get_error_message() {
    return g_lastError;
}

void cleanup() {
    g_audioInput.reset();
    g_melProcessor.reset();
    g_textureRenderer.reset();
    g_lastError[0] = '\0';
}

} // extern "C"