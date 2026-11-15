#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace aeonix
{

// ============================================================================
//  Observable - observable property (our Android-style "LiveData").
//
//  Stores a value and notifies subscribed observers when it changes.
//  ViewModels expose these properties; Views subscribe to
//  repaint/update their controls.
//
//  Subscribing returns an RAII Subscription: on destruction it unsubscribes
//  from the Observable. The Subscription must be destroyed BEFORE the
//  Observable (declare the Observable before the View that subscribes).
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

    // Sets the value and notifies the observers (only if it changed).
    void set (T value)
    {
        if (value == value_)
            return;

        value_ = std::move (value);
        notify();
    }

    // Subscribes an observer and returns an RAII token to cancel it.
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
        // Defensive copy: a listener may unsubscribe (or destroy a
        // View) during the notification.
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
