#ifndef CANONICALGEN_H
#define CANONICALGEN_H

#include <cstdint>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <iostream>

class CanonicalGen {
public:
    uint32_t codes[65536];
    uint8_t lengths[65536];

    CanonicalGen() {
        clear();
    }

    void clear() {
        for(int i=0; i<65536; i++) {
            codes[i] = 0;
            lengths[i] = 0;
        }
    }

    void generate(const uint8_t* sourceLengths) {
        clear();
        int maxLen = 0;
        for (int i = 0; i < 65536; i++) {
            lengths[i] = sourceLengths[i];
            if (lengths[i] > maxLen) maxLen = lengths[i];
        }
        if (maxLen == 0) return;

        std::vector<int> bl_count(maxLen + 1, 0);
        for (int i = 0; i < 65536; i++) {
            if (lengths[i] > 0) {
                bl_count[lengths[i]]++;
            }
        }

        std::vector<uint32_t> next_code(maxLen + 1, 0);
        uint32_t code = 0;
        for (int bits = 1; bits <= maxLen; bits++) {
            code = (code + bl_count[bits - 1]) << 1;
            next_code[bits] = code;
        }
        for (int i = 0; i < 65536; i++) {
            int len = lengths[i];
            if (len != 0) {
                codes[i] = next_code[len];
                next_code[len]++; 
            }
        }
    }

    static void printBinary(uint32_t code, int len) {
        for (int i = len - 1; i >= 0; i--) {
            std::cout << ((code >> i) & 1);
        }
    }
};

#endif