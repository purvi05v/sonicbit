#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>
#include <atomic>
#include <cstdint>
#include <algorithm> 

class LockFreeRingBuffer {
public:
    LockFreeRingBuffer(size_t size) : buffer(size), capacity(size) {
        readIndex.store(0);
        writeIndex.store(0);
    }
    bool push(const int16_t* data, size_t count) {
        size_t currentWrite = writeIndex.load(std::memory_order_acquire);
        size_t currentRead = readIndex.load(std::memory_order_acquire);
        
        size_t availableSpace = capacity - (currentWrite - currentRead);
        
        if (count > availableSpace) {
            return false; 
        }

        for (size_t i = 0; i < count; i++) {
            buffer[(currentWrite + i) % capacity] = data[i];
        }
        writeIndex.store(currentWrite + count, std::memory_order_release);
        return true;
    }
    size_t pop(std::vector<int16_t>& output, size_t requestedCount) {
        size_t currentWrite = writeIndex.load(std::memory_order_acquire);
        size_t currentRead = readIndex.load(std::memory_order_acquire);

        size_t availableSamples = currentWrite - currentRead;
        size_t toRead = std::min(availableSamples, requestedCount);

        if (toRead == 0) return 0;

        if (output.size() < toRead) output.resize(toRead);

        for (size_t i = 0; i < toRead; i++) {
            output[i] = buffer[(currentRead + i) % capacity];
        }
        readIndex.store(currentRead + toRead, std::memory_order_release);
        return toRead;
    }
    size_t available() const {
        return writeIndex.load() - readIndex.load();
    }

private:
    std::vector<int16_t> buffer;
    size_t capacity;
    std::atomic<size_t> readIndex;
    std::atomic<size_t> writeIndex;
};

#endif