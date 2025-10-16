#ifndef FLUTTER_SP_NATIVE_H
#define FLUTTER_SP_NATIVE_H

#include "audio_input.h"
#include "mel_spectrogram.h"

#ifdef __cplusplus
extern "C" {
#endif

// Audio Input Functions
int init_audio_input(const audio::AudioConfig* config);
int start_recording();
int stop_recording();

// Mel Processor Functions
int init_mel_processor(const melspectrogram::AudioConfig* config);
int process_audio_frame(const int16_t* inputBuffer, int bufferSize, 
                       float* outputBuffer, int outputSize);

// Texture Renderer Functions
int init_texture_renderer(int width, int height, int numMelBands);
int update_texture_column(const float* melData, int dataSize);
unsigned int get_texture_id();
int get_texture_data(uint8_t* buffer, int bufferSize);
int set_texture_color_map(int colorMapType);
int set_texture_min_max(float minValue, float maxValue);

// Utility Functions
const char* get_error_message();
int get_texture_width();
int get_texture_height();
int get_texture_num_mel_bands();
int get_texture_current_column();
float get_texture_last_update_time_ms();
void cleanup();

#ifdef __cplusplus
}
#endif

#endif // FLUTTER_SP_NATIVE_H