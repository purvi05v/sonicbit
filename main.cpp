/*
 * SonicBit - The Real-Time Lossless Audio Codec
 */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "RingBuffer.h"
#include "SonicBit.h" 
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

const int SAMPLE_RATE = 44100;
const int BLOCK_SIZE = 4096;
const int BUFFER_SECONDS = 1;
LockFreeRingBuffer audioBuffer(SAMPLE_RATE * BUFFER_SECONDS);

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    if (!audioBuffer.push((const int16_t*)pInput, frameCount)) {
    }
}

int main() {
    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_s16;
    config.capture.channels = 1; 
    config.sampleRate = SAMPLE_RATE;
    config.dataCallback = data_callback;

    ma_device device;
    ma_device_init(NULL, &config, &device);
    ma_device_start(&device);
    std::cout << "SonicBit Engine Started..." << std::endl;
    SonicBit codec;
    std::vector<int16_t> rawBlock;
    rawBlock.reserve(BLOCK_SIZE);

    bool running = true;
    while (running) {
        if (audioBuffer.available() >= BLOCK_SIZE) {
            
            audioBuffer.pop(rawBlock, BLOCK_SIZE);
            const std::vector<uint8_t>& compressedData = codec.compress(rawBlock);
            size_t orgSize = BLOCK_SIZE * sizeof(int16_t);
            size_t compSize = compressedData.size();
            float ratio = codec.getLastRatio(orgSize);
            float savings = 100.0f * (1.0f - ((float)compSize / (float)orgSize));

            std::cout << "\r[SonicBit] " 
                      << compSize << " bytes "
                      << "| Ratio: " << std::fixed << std::setprecision(1) << ratio << ":1 "
                      << "| Saved: " << (int)savings << "%     " << std::flush;
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    ma_device_uninit(&device);
    return 0;
}