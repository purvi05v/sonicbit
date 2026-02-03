#ifndef DELTAENCODER_H
#define DELTAENCODER_H

#include <vector>
#include <cstdint>
#include <cmath>

class DeltaEncoder {
public:
    static void encode(const std::vector<int16_t>& input, std::vector<uint16_t>& output) {
        if (input.empty()) return;
        if (output.size() != input.size()) output.resize(input.size());

        int16_t previous = 0; 
        for (size_t i = 0; i < input.size(); i++) {
            int16_t delta = input[i] - previous;
            uint16_t zigzag = (delta << 1) ^ (delta >> 15);

            output[i] = zigzag;
            previous = input[i];
        }
    }
    static void decode(const std::vector<uint16_t>& input, std::vector<int16_t>& output) {
        if (input.empty()) return;
        if (output.size() != input.size()) output.resize(input.size());

        int16_t previous = 0;

        for (size_t i = 0; i < input.size(); i++) {
            uint16_t zigzag = input[i];
            int16_t delta = (zigzag >> 1) ^ -(zigzag & 1);
            int16_t sample = previous + delta;
            
            output[i] = sample;
            previous = sample;
        }
    }
};

#endif