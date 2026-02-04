#ifndef BITWRITER_H
#define BITWRITER_H

#include <vector>
#include <cstdint>
#include <iostream> 

class BitWriter {
public:
    std::vector<uint8_t> bytes;
    uint32_t bitBuffer;
    int bitCount;

    BitWriter() {
        clear();
    }

    void clear() {
        bytes.clear();
        bitBuffer = 0;
        bitCount = 0;
    }

    void write(uint32_t value, int length) {
        if (length == 0) return;
        uint32_t mask = (length == 32) ? 0xFFFFFFFF : ((1 << length) - 1);
        value &= mask;
        bitBuffer = (bitBuffer << length) | value;
        bitCount += length;

        while (bitCount >= 8) {
            uint8_t byteVal = (bitBuffer >> (bitCount - 8)) & 0xFF;
            bytes.push_back(byteVal);
            bitCount -= 8;
        }
    }
    void flush() {
        if (bitCount > 0) {
            uint8_t byteVal = (bitBuffer << (8 - bitCount)) & 0xFF;
            bytes.push_back(byteVal);
            bitCount = 0;
            bitBuffer = 0;
        }
    }
    size_t getSize() const {
        return bytes.size();
    }
};

#endif