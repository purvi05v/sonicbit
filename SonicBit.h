#ifndef SONICBIT_H
#define SONICBIT_H

#include <vector>
#include <cstdint>

#include "DeltaEncoder.h"
#include "FrequencyTable.h"
#include "HuffmanBuilder.h"
#include "CanonicalGen.h"
#include "BitWriter.h"

class SonicBit {
public:
    SonicBit() {
        compressedSymbols.reserve(4096); 
    }
    std::vector<uint8_t>& compress(const std::vector<int16_t>& input) {
        DeltaEncoder::encode(input, compressedSymbols);
        freqTable.scan(compressedSymbols);
        huffBuilder.build(freqTable);
        canonGen.generate(huffBuilder.codeLengths);
        bitWriter.clear();
        
        for (uint16_t sym : compressedSymbols) {
            bitWriter.write(canonGen.codes[sym], canonGen.lengths[sym]);
        }
        bitWriter.flush();
        return bitWriter.bytes;
    }
    float getLastRatio(size_t originalBytes) const {
        if (bitWriter.getSize() == 0) return 0.0f;
        return (float)originalBytes / (float)bitWriter.getSize();
    }

private:
    FrequencyTable freqTable;
    HuffmanBuilder huffBuilder;
    CanonicalGen canonGen;
    BitWriter bitWriter;

    std::vector<uint16_t> compressedSymbols;
};

#endif