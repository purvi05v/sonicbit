#ifndef FREQUENCYTABLE_H
#define FREQUENCYTABLE_H

#include <vector>
#include <cstdint>
#include <cstring> 

class FrequencyTable {
public:
    static const int MAX_SYMBOL = 65536;
    uint32_t counts[MAX_SYMBOL];

    FrequencyTable() {
        clear();
    }

    void clear() {
        std::memset(counts, 0, sizeof(counts));
    }

    void scan(const std::vector<uint16_t>& symbols) {
        clear();
        for (uint16_t sym : symbols) {
            counts[sym]++;
        }
    }

    void printTopStats() const {
        uint32_t maxCount = 0;
        int mostFrequentSymbol = -1;

        for (int i = 0; i < MAX_SYMBOL; i++) {
            if (counts[i] > maxCount) {
                maxCount = counts[i];
                mostFrequentSymbol = i;
            }
        }

        if (mostFrequentSymbol != -1) {
            printf(" | Top Sym: %d (Count: %d)", mostFrequentSymbol, maxCount);
        }
    }
};

#endif