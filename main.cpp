/*
 * SonicBit - Real-Time Audio Capture & Buffering
 * * Dependencies:
 * 1. miniaudio.h (Download from: https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h)
 * 2. RingBuffer.h (Created in the previous step)
 *
 * Build Instructions:
 * Linux/macOS: g++ main.cpp -o sonicbit -lpthread -ldl -lm
 * Windows: Compile with Visual Studio (Console App)
 */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "RingBuffer.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath> 


const int SAMPLE_RATE = 44100;
const int BLOCK_SIZE = 4096; 
const int BUFFER_SECONDS = 1; 

LockFreeRingBuffer audioBuffer(SAMPLE_RATE * BUFFER_SECONDS);

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    const int16_t* samples = (const int16_t*)pInput;
    bool success = audioBuffer.push(samples, frameCount);
    
    if (!success) {
        // fprintf(stderr, "X"); 
    }

    (void)pOutput; 
}
int main()
{
    ma_result result;
    ma_device_config deviceConfig;
    ma_device device;
    deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format   = ma_format_s16; 
    deviceConfig.capture.channels = 1;             
    deviceConfig.sampleRate       = SAMPLE_RATE;
    deviceConfig.dataCallback     = data_callback;

    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize capture device." << std::endl;
        return -1;
    }

    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to start device." << std::endl;
        ma_device_uninit(&device);
        return -1;
    }

    std::cout << "========================================" << std::endl;
    std::cout << " SonicBit Engine Started" << std::endl;
    std::cout << " Sample Rate: " << SAMPLE_RATE << " Hz" << std::endl;
    std::cout << " Block Size:  " << BLOCK_SIZE << " samples" << std::endl;
    std::cout << "========================================" << std::endl;

    std::vector<int16_t> processingBlock;
    processingBlock.reserve(BLOCK_SIZE);

    bool running = true;
    while (running) {
        if (audioBuffer.available() >= BLOCK_SIZE) {
            audioBuffer.pop(processingBlock, BLOCK_SIZE);
            double sum = 0;
            for (int16_t sample : processingBlock) {
                sum += sample * sample;
            }
            double rms = sqrt(sum / BLOCK_SIZE);
            int bars = (int)(rms / 50.0); 
            if (bars > 60) bars = 60;
            std::cout << "\r[";
            for (int i=0; i<bars; i++) std::cout << "|";
            for (int i=bars; i<60; i++) std::cout << " ";
            std::cout << "] " << audioBuffer.available() << " buffered  " << std::flush;
            // std::vector<uint8_t> bytes = SonicEncoder.compress(processingBlock);
            // Network.send(bytes);
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    ma_device_uninit(&device);
    return 0;
}