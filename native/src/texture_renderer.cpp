#include "texture_renderer.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GLES3/gl3.h>
#endif

namespace melspectrogram {

// Viridis color map data (normalized to 0-1 range)
const std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>> TextureRenderer::viridisColors_ = {
    {0.0f, 68, 1, 84},
    {0.13f, 71, 44, 122},
    {0.25f, 59, 81, 139},
    {0.38f, 44, 113, 142},
    {0.5f, 33, 144, 140},
    {0.63f, 39, 173, 129},
    {0.75f, 92, 200, 99},
    {0.88f, 170, 220, 50},
    {1.0f, 253, 231, 37}
};

// Inferno color map data
const std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>> TextureRenderer::infernoColors_ = {
    {0.0f, 0, 0, 4},
    {0.13f, 31, 12, 72},
    {0.25f, 85, 15, 109},
    {0.38f, 136, 19, 97},
    {0.5f, 186, 25, 51},
    {0.63f, 219, 51, 28},
    {0.75f, 232, 113, 32},
    {0.88f, 236, 173, 55},
    {1.0f, 252, 255, 164}
};

// Plasma color map data
const std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>> TextureRenderer::plasmaColors_ = {
    {0.0f, 13, 8, 135},
    {0.13f, 84, 2, 163},
    {0.25f, 139, 10, 165},
    {0.38f, 185, 50, 137},
    {0.5f, 219, 92, 104},
    {0.63f, 244, 136, 73},
    {0.75f, 254, 188, 43},
    {0.88f, 240, 249, 33},
    {1.0f, 240, 249, 33}
};

TextureRenderer::TextureRenderer(int width, int height, int numMelBands)
    : width_(width), height_(height), numMelBands_(numMelBands),
      initialized_(false), textureId_(0), currentColorMap_(ColorMapType::VIRIDIS),
      minValue_(0.0f), maxValue_(1.0f), currentColumn_(0), lastUpdateTimeMs_(0.0f),
      mockMode_(false) {
    
    textureData_.resize(width_ * height_ * 4, 0); // RGBA format
    ringBuffer_.resize(width_ * numMelBands_ * 4, 0); // RGBA format
    mockTextureData_.resize(width_ * height_ * 4, 0);
    
    // Initialize mock texture data with alpha = 255 for all pixels
    for (int i = 0; i < width_ * height_; ++i) {
        mockTextureData_[i * 4 + 3] = 255; // Alpha channel
    }
    
    createColorMaps();
    
    // Initialize in mock mode by default - actual OpenGL initialization
    // should be done explicitly when an OpenGL context is available
    mockMode_ = true;
    initialized_ = true;  // Allow operation in mock mode
    std::cout << "TextureRenderer initialized in mock mode (no OpenGL context)" << std::endl;
}

TextureRenderer::~TextureRenderer() {
    if (!mockMode_) {
        cleanupOpenGL();
    }
}

bool TextureRenderer::initialize() {
    if (initialized_) {
        return true;
    }
    
    if (!setupOpenGL()) {
        return false;
    }
    
    if (!createTexture()) {
        cleanupOpenGL();
        return false;
    }
    
    initialized_ = true;
    return true;
}

bool TextureRenderer::setupOpenGL() {
    // Check if OpenGL context is available by trying a simple operation
    GLenum error = glGetError(); // Clear any existing errors
    
    // Try to get a simple state - this will fail if no context
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    
    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "OpenGL context not available" << std::endl;
        return false;
    }
    
    // Enable blending for transparency (if needed)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return true;
}

bool TextureRenderer::createTexture() {
    glGenTextures(1, &textureId_);
    
    if (textureId_ == 0) {
        std::cerr << "Failed to generate OpenGL texture" << std::endl;
        return false;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId_);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Create texture with initial data (all black)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, textureData_.data());
    
    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "Failed to create OpenGL texture" << std::endl;
        glDeleteTextures(1, &textureId_);
        textureId_ = 0;
        return false;
    }
    
    return true;
}

bool TextureRenderer::updateColumn(const std::vector<float>& melData) {
    if (!initialized_) {
        return false;
    }
    
    if (melData.size() != static_cast<size_t>(numMelBands_)) {
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Convert mel data to colors and update ring buffer
    for (int band = 0; band < numMelBands_; ++band) {
        float value = melData[band];
        
        // Clamp value to valid range
        value = std::max(minValue_, std::min(maxValue_, value));
        
        // Normalize to 0-1 range
        float normalized = (value - minValue_) / (maxValue_ - minValue_);
        
        uint8_t r, g, b;
        applyColorMap(normalized, &r, &g, &b);
        
        // Store in ring buffer (column-major order for efficient updates)
        int bufferIndex = (currentColumn_ * numMelBands_ + band) * 4;
        ringBuffer_[bufferIndex + 0] = r;
        ringBuffer_[bufferIndex + 1] = g;
        ringBuffer_[bufferIndex + 2] = b;
        ringBuffer_[bufferIndex + 3] = 255; // Full opacity
    }
    
    // Update the texture with the new column data
    if (!mockMode_) {
        updateTextureSubImage();
    } else {
        // In mock mode, just update the mock texture data
        for (int y = 0; y < height_; ++y) {
            int band = (y * numMelBands_) / height_;
            if (band >= numMelBands_) band = numMelBands_ - 1;
            
            int bufferIndex = (currentColumn_ * numMelBands_ + band) * 4;
            int textureIndex = ((height_ - 1 - y) * width_ + currentColumn_) * 4;
            
            mockTextureData_[textureIndex + 0] = ringBuffer_[bufferIndex + 0];
            mockTextureData_[textureIndex + 1] = ringBuffer_[bufferIndex + 1];
            mockTextureData_[textureIndex + 2] = ringBuffer_[bufferIndex + 2];
            mockTextureData_[textureIndex + 3] = ringBuffer_[bufferIndex + 3];
        }
    }
    
    // Advance to next column (ring buffer)
    advanceColumn();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    lastUpdateTimeMs_ = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    return true;
}

void TextureRenderer::applyColorMap(float value, uint8_t* r, uint8_t* g, uint8_t* b) {
    const auto& colorMap = colorMaps_[static_cast<int>(currentColorMap_)];
    auto color = interpolateColor(value, colorMap);
    
    *r = std::get<0>(color);
    *g = std::get<1>(color);
    *b = std::get<2>(color);
}

std::tuple<uint8_t, uint8_t, uint8_t> TextureRenderer::interpolateColor(
    float value, const std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>>& colorMap) {
    
    // Clamp value to [0, 1]
    value = std::max(0.0f, std::min(1.0f, value));
    
    // Find the two colors to interpolate between
    size_t i = 0;
    for (; i < colorMap.size() - 1; ++i) {
        if (value <= std::get<0>(colorMap[i + 1])) {
            break;
        }
    }
    
    if (i >= colorMap.size() - 1) {
        // Use the last color
    return std::make_tuple(std::get<1>(colorMap.back()),
                          std::get<2>(colorMap.back()),
                          std::get<3>(colorMap.back()));
    }
    
    // Interpolate between colorMap[i] and colorMap[i+1]
    float t1 = std::get<0>(colorMap[i]);
    float t2 = std::get<0>(colorMap[i + 1]);
    float factor = (value - t1) / (t2 - t1);
    
    uint8_t r = static_cast<uint8_t>(std::get<1>(colorMap[i]) * (1.0f - factor) +
                                     std::get<1>(colorMap[i + 1]) * factor + 0.5f);
    uint8_t g = static_cast<uint8_t>(std::get<2>(colorMap[i]) * (1.0f - factor) +
                                     std::get<2>(colorMap[i + 1]) * factor + 0.5f);
    uint8_t b = static_cast<uint8_t>(std::get<3>(colorMap[i]) * (1.0f - factor) +
                                     std::get<3>(colorMap[i + 1]) * factor + 0.5f);
    
    return std::make_tuple(r, g, b);
}

void TextureRenderer::updateTextureSubImage() {
    glBindTexture(GL_TEXTURE_2D, textureId_);
    
    // Update the entire texture from the ring buffer
    // We need to reorganize the ring buffer data to match texture layout
    std::vector<uint8_t> organizedData(width_ * height_ * 4);
    
    for (int x = 0; x < width_; ++x) {
        // Calculate the column index in the ring buffer
        int ringCol = (currentColumn_ - width_ + x + width_) % width_;
        
        for (int y = 0; y < height_; ++y) {
            // Map y coordinate to mel band (with interpolation if needed)
            int band = (y * numMelBands_) / height_;
            if (band >= numMelBands_) band = numMelBands_ - 1;
            
            int srcIndex = (ringCol * numMelBands_ + band) * 4;
            int dstIndex = ((height_ - 1 - y) * width_ + x) * 4; // Flip vertically
            
            organizedData[dstIndex + 0] = ringBuffer_[srcIndex + 0];
            organizedData[dstIndex + 1] = ringBuffer_[srcIndex + 1];
            organizedData[dstIndex + 2] = ringBuffer_[srcIndex + 2];
            organizedData[dstIndex + 3] = ringBuffer_[srcIndex + 3];
        }
    }
    
    // Update the entire texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_,
                    GL_RGBA, GL_UNSIGNED_BYTE, organizedData.data());
}

void TextureRenderer::advanceColumn() {
    currentColumn_ = (currentColumn_ + 1) % width_;
}

void TextureRenderer::setColorMap(ColorMapType type) {
    currentColorMap_ = type;
}

void TextureRenderer::setMinMaxValues(float minValue, float maxValue) {
    minValue_ = minValue;
    maxValue_ = maxValue;
}

std::vector<uint8_t> TextureRenderer::getTextureData() const {
    if (!initialized_) {
        return {};
    }
    
    std::vector<uint8_t> data(width_ * height_ * 4);
    
    if (!mockMode_) {
        glBindTexture(GL_TEXTURE_2D, textureId_);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    } else {
        // In mock mode, return the mock texture data
        data = mockTextureData_;
    }
    
    return data;
}

void TextureRenderer::createColorMaps() {
    colorMaps_.clear();
    colorMaps_.push_back(viridisColors_);
    colorMaps_.push_back(infernoColors_);
    colorMaps_.push_back(plasmaColors_);
}

void TextureRenderer::cleanupOpenGL() {
    if (textureId_ != 0) {
        glDeleteTextures(1, &textureId_);
        textureId_ = 0;
    }
    initialized_ = false;
}

} // namespace melspectrogram