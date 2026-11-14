#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace aeonix
{

// ============================================================================
//  Parameter - defines a synthesizer parameter.
//
//  - Has a unique id (text), a readable name and a range [min, max].
//  - Internally stores a NORMALIZED value in [0, 1]. getValue() converts it
//    to the real range.
//  - Supports LOGARITHMIC mapping (for frequencies and times), which makes a
//    0..1 slider intuitive when the range spans several decades.
//
//  It is a value object: no audio logic, no dependencies.
// ============================================================================
class Parameter
{
public:
    Parameter (std::string id,
               std::string name,
               float min,
               float max,
               float defaultNormalised = 0.0f,
               bool logarithmic = false)
        : id_ (std::move (id)),
          name_ (std::move (name)),
          min_ (min),
          max_ (max),
          logarithmic_ (logarithmic),
          defaultNormalised_ (clamp01 (defaultNormalised)),
          normalised_ (defaultNormalised_)
    {
    }

    [[nodiscard]] const std::string& getId()   const noexcept { return id_; }
    [[nodiscard]] const std::string& getName() const noexcept { return name_; }
    [[nodiscard]] float getMin()               const noexcept { return min_; }
    [[nodiscard]] float getMax()               const noexcept { return max_; }
    [[nodiscard]] bool  isLogarithmic()        const noexcept { return logarithmic_; }
    [[nodiscard]] float getDefaultNormalised() const noexcept { return defaultNormalised_; }

    // Normalized value (0..1).
    [[nodiscard]] float getNormalised() const noexcept { return normalised_; }
    void setNormalised (float n) noexcept { normalised_ = clamp01 (n); }

    // Real value (in the range [min, max]).
    [[nodiscard]] float getValue() const noexcept
    {
        if (logarithmic_)
            return min_ * std::pow (max_ / min_, normalised_);

        return min_ + normalised_ * (max_ - min_);
    }

    void setValue (float v) noexcept
    {
        v = std::clamp (v, min_, max_);

        if (max_ == min_ || (logarithmic_ && min_ <= 0.0f))
        {
            normalised_ = 0.0f;
            return;
        }

        if (logarithmic_)
            normalised_ = std::log (v / min_) / std::log (max_ / min_);
        else
            normalised_ = (v - min_) / (max_ - min_);
    }

private:
    static float clamp01 (float x) noexcept { return std::clamp (x, 0.0f, 1.0f); }

    std::string id_;
    std::string name_;
    float min_;
    float max_;
    bool logarithmic_;
    float defaultNormalised_;
    float normalised_;
};

} // namespace aeonix
