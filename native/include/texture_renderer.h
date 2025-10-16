#ifndef TEXTURE_RENDERER_H
#define TEXTURE_RENDERER_H

#include <vector>
#include <cstdint>
#include <memory>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GLES3/gl3.h>
#endif

namespace melspectrogram {

enum class ColorMapType {
    VIRIDIS,
    INFERNO,
    PLASMA
};

class TextureRenderer {
public:
    TextureRenderer(int width, int height, int numMelBands);
    ~TextureRenderer();

    // Main functionality
    bool initialize();
    bool updateColumn(const std::vector<float>& melData);
    
    // Configuration
    void setColorMap(ColorMapType type);
    void setMinMaxValues(float minValue, float maxValue);
    
    // Data access
    std::vector<uint8_t> getTextureData() const;
    unsigned int getTextureId() const { return textureId_; }
    
    // Status
    bool isInitialized() const { return initialized_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getNumMelBands() const { return numMelBands_; }
    
    // Performance metrics
    float getLastUpdateTimeMs() const { return lastUpdateTimeMs_; }
    int getCurrentColumn() const { return currentColumn_; }

private:
    // OpenGL setup
    bool setupOpenGL();
    void cleanupOpenGL();
    
    // Color mapping
    void applyColorMap(float value, uint8_t* r, uint8_t* g, uint8_t* b);
    void createColorMaps();
    std::tuple<uint8_t, uint8_t, uint8_t> interpolateColor(float value, 
        const std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>>& colorMap);
    
    // Texture management
    bool createTexture();
    void updateTextureSubImage();
    
    // Ring buffer management
    void advanceColumn();
    
    // Member variables
    int width_;
    int height_;
    int numMelBands_;
    bool initialized_;
    
    // OpenGL resources
    unsigned int textureId_;
    std::vector<uint8_t> textureData_;
    
    // Color mapping
    ColorMapType currentColorMap_;
    std::vector<std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>>> colorMaps_;
    float minValue_;
    float maxValue_;
    
    // Ring buffer for waterfall effect
    int currentColumn_;
    std::vector<uint8_t> ringBuffer_;
    
    // Performance metrics
    mutable float lastUpdateTimeMs_;
    bool mockMode_;  // For testing without OpenGL context
    std::vector<uint8_t> mockTextureData_;  // For testing
    
    // Color map data
    static const std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>> viridisColors_;
    static const std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>> infernoColors_;
    static const std::vector<std::tuple<float, uint8_t, uint8_t, uint8_t>> plasmaColors_;
};

} // namespace melspectrogram

#endif // TEXTURE_RENDERER_H