#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace aeonix
{

// ============================================================================
//  Observable — propiedad observable (nuestro "LiveData" de Android).
//
//  Guarda un valor y notifica a los observadores suscritos cuando cambia.
//  Los ViewModels exponen estas propiedades; las Views se suscriben para
//  redibujar/actualizar sus controles.
//
//  La suscripcion devuelve un Subscription RAII: al destruirse se da de baja
//  del Observable. Es importante que el Subscription se destruya ANTES que el
//  Observable (declarar el Observable antes que la View que se suscribe).
// ============================================================================
template <typename T>
class Observable
{
public:
    class Subscription
    {
    public:
        Subscription() = default;

        Subscription (Subscription&& other) noexcept
            : observable_ (other.observable_), id_ (other.id_)
        {
            other.observable_ = nullptr;
            other.id_ = 0;
        }

        Subscription& operator= (Subscription&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                observable_ = other.observable_;
                id_ = other.id_;
                other.observable_ = nullptr;
                other.id_ = 0;
            }

            return *this;
        }

        ~Subscription() { reset(); }

        Subscription (const Subscription&) = delete;
        Subscription& operator= (const Subscription&) = delete;

        void reset() noexcept
        {
            if (observable_ != nullptr)
            {
                observable_->unsubscribe (id_);
                observable_ = nullptr;
                id_ = 0;
            }
        }

    private:
        friend class Observable;

        Subscription (Observable* observable, std::size_t id)
            : observable_ (observable), id_ (id) {}

        Observable* observable_ = nullptr;
        std::size_t id_ = 0;
    };

    Observable() = default;
    explicit Observable (T initial) : value_ (std::move (initial)) {}

    Observable (const Observable&) = delete;
    Observable& operator= (const Observable&) = delete;

    [[nodiscard]] const T& get() const noexcept { return value_; }

    // Establece el valor y notifica a los observadores (solo si cambio).
    void set (T value)
    {
        if (value == value_)
            return;

        value_ = std::move (value);
        notify();
    }

    // Suscribe un observador y devuelve un token RAII para cancelarlo.
    [[nodiscard]] Subscription subscribe (std::function<void (const T&)> listener)
    {
        const auto id = nextId_++;
        listeners_.push_back ({ id, std::move (listener) });
        return Subscription (this, id);
    }

private:
    struct Entry
    {
        std::size_t id;
        std::function<void (const T&)> fn;
    };

    void unsubscribe (std::size_t id)
    {
        listeners_.erase (std::remove_if (listeners_.begin(), listeners_.end(),
                                          [id] (const Entry& entry)
                                          {
                                              return entry.id == id;
                                          }),
                          listeners_.end());
    }

    [[nodiscard]] bool isSubscribed (std::size_t id) const
    {
        return std::any_of (listeners_.begin(), listeners_.end(),
                            [id] (const Entry& entry) { return entry.id == id; });
    }

    void notify()
    {
        // Copia defensiva: un listener puede darse de baja (o destruir una
        // View) durante la notificacion.
        const auto snapshot = listeners_;

        for (const auto& entry : snapshot)
            if (isSubscribed (entry.id))
                entry.fn (value_);
    }

    T value_ {};
    std::vector<Entry> listeners_;
    std::size_t nextId_ = 1;
};

} // namespace aeonix
