#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>
#include <atomic>
#include <cstdint>
#include <algorithm> // For std::min

class LockFreeRingBuffer {
public:
    // Initialize with a fixed size (e.g., 1 second of audio = 44100 samples)
    LockFreeRingBuffer(size_t size) : buffer(size), capacity(size) {
        readIndex.store(0);
        writeIndex.store(0);
    }

    // 1. PUSH: Called by the Audio Callback (Producer)
    // Returns false if buffer is full
    bool push(const int16_t* data, size_t count) {
        size_t currentWrite = writeIndex.load(std::memory_order_acquire);
        size_t currentRead = readIndex.load(std::memory_order_acquire);
        
        size_t availableSpace = capacity - (currentWrite - currentRead);
        
        if (count > availableSpace) {
            return false; // Buffer overflow! Data is lost.
        }

        for (size_t i = 0; i < count; i++) {
            // Use modulo operator to wrap around the buffer
            buffer[(currentWrite + i) % capacity] = data[i];
        }

        // Update the write index atomically
        writeIndex.store(currentWrite + count, std::memory_order_release);
        return true;
    }

    // 2. POP: Called by the Main Thread (Consumer/Compressor)
    // Returns the number of samples actually read
    size_t pop(std::vector<int16_t>& output, size_t requestedCount) {
        size_t currentWrite = writeIndex.load(std::memory_order_acquire);
        size_t currentRead = readIndex.load(std::memory_order_acquire);

        size_t availableSamples = currentWrite - currentRead;
        size_t toRead = std::min(availableSamples, requestedCount);

        if (toRead == 0) return 0;

        // Resize output vector to fit the new data
        if (output.size() < toRead) output.resize(toRead);

        for (size_t i = 0; i < toRead; i++) {
            output[i] = buffer[(currentRead + i) % capacity];
        }

        // Update read index atomically
        readIndex.store(currentRead + toRead, std::memory_order_release);
        return toRead;
    }

    // Helper to check how many samples are waiting
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