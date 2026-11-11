#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace aeonix
{

// ============================================================================
//  SpScQueue — cola lock-free Single-Producer / Single-Consumer.
//
//  - Un unico hilo produce (push) y un unico hilo consume (pop).
//  - No usa locks ni asigna memoria: es segura para el hilo de audio (RT-safe).
//  - Los indices son contadores monotonos (nunca decrecen); se enmascaran para
//    indexar el buffer. Asi puede llenarse al completo sin slot desperdiciado.
//
//  Capacidad fija: si esta llena, tryPush devuelve false (el evento se pierde).
//  Se elige una capacidad generosa para que esto no ocurra en la practica.
// ============================================================================
template <typename T, std::size_t Capacity>
class SpScQueue
{
    static_assert (Capacity > 0 && (Capacity & (Capacity - 1)) == 0,
                   "Capacity debe ser una potencia de dos");

public:
    SpScQueue() = default;

    // Lado productor (un solo hilo).
    bool tryPush (const T& item)
    {
        const std::uint64_t write = writePos_.load (std::memory_order_relaxed);
        const std::uint64_t read  = readPos_.load (std::memory_order_acquire);

        if (write - read >= Capacity)
            return false;   // llena

        buffer_[static_cast<std::size_t> (write & mask_)] = item;
        writePos_.store (write + 1, std::memory_order_release);
        return true;
    }

    // Lado consumidor (un solo hilo).
    bool tryPop (T& out)
    {
        const std::uint64_t write = writePos_.load (std::memory_order_acquire);
        const std::uint64_t read  = readPos_.load (std::memory_order_relaxed);

        if (read == write)
            return false;   // vacia

        out = buffer_[static_cast<std::size_t> (read & mask_)];
        readPos_.store (read + 1, std::memory_order_release);
        return true;
    }

    // Numero de elementos actualmente en la cola (aproximado).
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
