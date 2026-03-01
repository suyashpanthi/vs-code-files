#pragma once
#include <atomic>
#include <array>
#include <cstddef>

// Lock-free SPSC (Single Producer, Single Consumer) ring buffer
// Used for transferring pitch data from audio thread to GUI thread
template <typename T, size_t Capacity>
class RingBuffer
{
public:
    RingBuffer() : writePos(0), readPos(0) {}

    bool push(const T& item)
    {
        size_t w = writePos.load(std::memory_order_relaxed);
        size_t next = (w + 1) % Capacity;
        if (next == readPos.load(std::memory_order_acquire))
            return false; // full
        buffer[w] = item;
        writePos.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& item)
    {
        size_t r = readPos.load(std::memory_order_relaxed);
        if (r == writePos.load(std::memory_order_acquire))
            return false; // empty
        item = buffer[r];
        readPos.store((r + 1) % Capacity, std::memory_order_release);
        return true;
    }

    bool isEmpty() const
    {
        return readPos.load(std::memory_order_acquire) == writePos.load(std::memory_order_acquire);
    }

    size_t getNumReady() const
    {
        size_t w = writePos.load(std::memory_order_acquire);
        size_t r = readPos.load(std::memory_order_acquire);
        return (w >= r) ? (w - r) : (Capacity - r + w);
    }

private:
    std::array<T, Capacity> buffer;
    std::atomic<size_t> writePos;
    std::atomic<size_t> readPos;
};

// Pitch data point for visualization
struct PitchDataPoint
{
    float inputPitchHz  = 0.0f;   // Detected input pitch
    float outputPitchHz = 0.0f;   // Corrected output pitch
    float confidence    = 0.0f;   // YIN confidence (0-1)
};
