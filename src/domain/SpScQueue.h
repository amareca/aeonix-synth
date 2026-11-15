#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace aeonix
{

// ============================================================================
//  SpScQueue - lock-free Single-Producer / Single-Consumer queue.
//
//  - A single thread produces (push) and a single thread consumes (pop).
//  - Uses no locks and allocates no memory: safe for the audio thread (RT-safe).
//  - The indices are monotonic counters (never decrease); they are masked to
//    index the buffer. That way it can be filled completely with no wasted slot.
//
//  Fixed capacity: if full, tryPush returns false (the event is dropped).
//  A generous capacity is chosen so this does not happen in practice.
// ============================================================================
template <typename T, std::size_t Capacity>
class SpScQueue
{
    static_assert (Capacity > 0 && (Capacity & (Capacity - 1)) == 0,
                   "Capacity debe ser una potencia de dos");

public:
    SpScQueue() = default;

    // Producer side (single thread).
    bool tryPush (const T& item)
    {
        const std::uint64_t write = writePos_.load (std::memory_order_relaxed);
        const std::uint64_t read  = readPos_.load (std::memory_order_acquire);

        if (write - read >= Capacity)
            return false;   // full

        buffer_[static_cast<std::size_t> (write & mask_)] = item;
        writePos_.store (write + 1, std::memory_order_release);
        return true;
    }

    // Consumer side (single thread).
    bool tryPop (T& out)
    {
        const std::uint64_t write = writePos_.load (std::memory_order_acquire);
        const std::uint64_t read  = readPos_.load (std::memory_order_relaxed);

        if (read == write)
            return false;   // empty

        out = buffer_[static_cast<std::size_t> (read & mask_)];
        readPos_.store (read + 1, std::memory_order_release);
        return true;
    }

    // Number of elements currently in the queue (approximate).
    [[nodiscard]] std::uint64_t size() const noexcept
    {
        const std::uint64_t write = writePos_.load (std::memory_order_acquire);
        const std::uint64_t read  = readPos_.load (std::memory_order_acquire);
        return write - read;
    }

private:
    static constexpr std::uint64_t mask_ = Capacity - 1;

    std::array<T, Capacity> buffer_;
    std::atomic<std::uint64_t> readPos_ { 0 };
    std::atomic<std::uint64_t> writePos_ { 0 };
};

} // namespace aeonix
