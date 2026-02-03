/*
 * SonicBit - Real-Time Audio Capture & Delta Encoding
 */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "RingBuffer.h"
#include "DeltaEncoder.h"
#include "FrequencyTable.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>

const int SAMPLE_RATE = 44100;
const int BLOCK_SIZE = 4096;  
const int BUFFER_SECONDS = 1; 

LockFreeRingBuffer audioBuffer(SAMPLE_RATE * BUFFER_SECONDS);
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    const int16_t* samples = (const int16_t*)pInput;
    if (!audioBuffer.push(samples, frameCount)) {
        // fprintf(stderr, "X"); 
    }

    (void)pOutput; 
}

int main()
{
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format   = ma_format_s16;
    deviceConfig.capture.channels = 1;
    deviceConfig.sampleRate       = SAMPLE_RATE;
    deviceConfig.dataCallback     = data_callback;

    ma_device device;
    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize capture device." << std::endl;
        return -1;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        std::cerr << "Failed to start device." << std::endl;
        ma_device_uninit(&device);
        return -1;
    }

    std::cout << "==========================================" << std::endl;
    std::cout << " SonicBit Engine: Delta Encoding Active" << std::endl;
    std::cout << "==========================================" << std::endl;

    std::vector<int16_t> rawBlock;
    rawBlock.reserve(BLOCK_SIZE);

    std::vector<uint16_t> compressedSymbols;
    compressedSymbols.reserve(BLOCK_SIZE);

    std::vector<int16_t> restoredBlock; 
    restoredBlock.reserve(BLOCK_SIZE);

    bool running = true;
    FrequencyTable freqTable;

    while (running) {
        if (audioBuffer.available() >= BLOCK_SIZE) {
            audioBuffer.pop(rawBlock, BLOCK_SIZE);
            DeltaEncoder::encode(rawBlock, compressedSymbols);
            freqTable.scan(compressedSymbols);
            std::cout << "\r[Block Processed]";
            freqTable.printTopStats(); 
            std::cout << "      " << std::flush;
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    ma_device_uninit(&device);
    return 0;
}