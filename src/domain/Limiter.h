#pragma once

#include <algorithm>
#include <cmath>

namespace aeonix
{

// ============================================================================
//  Limiter - safety limiter for the master bus.
//
//  Linked peak detector (the same reduction is applied to L and R, so the
//  stereo image does not move) with immediate attack and exponential release.
//  No makeup gain: below the threshold it is transparent. A final hard
//  clip guarantees the output never exceeds +/-1.
//
//  RT-safe: per-sample arithmetic only, no allocations or costly branches.
// ============================================================================
class Limiter
{
public:
    static constexpr float defaultThreshold = 0.95f;
    static constexpr float defaultReleaseMs = 120.0f;

    void prepare (double sampleRate)
    {
        sampleRate_ = sampleRate > 0.0 ? sampleRate : 44100.0;
        setReleaseMs (releaseMs_);
        reset();
    }

    void reset() noexcept { gain_ = 1.0f; }

    void setThreshold (float threshold) noexcept
    {
        threshold_ = std::clamp (threshold, 0.1f, 1.0f);
    }

    void setReleaseMs (float milliseconds) noexcept
    {
        releaseMs_ = std::max (milliseconds, 1.0f);

        // Coeff of a pole that returns to ~63 % of the way in releaseMs.
        releaseCoeff_ = static_cast<float> (
            std::exp (-1.0 / (0.001 * static_cast<double> (releaseMs_) * sampleRate_)));
    }

    void process (float* left, float* right, int numSamples) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const float peak = std::max (std::fabs (left[i]), std::fabs (right[i]));
            const float target = peak > threshold_ ? threshold_ / peak : 1.0f;

            if (target < gain_)
                gain_ = target;                                        // immediate attack
            else
                gain_ = gain_ * releaseCoeff_ + (1.0f - releaseCoeff_); // smooth release

            // Safety net: the hard clip only acts if something escapes.
            left[i]  = std::clamp (left[i] * gain_, -1.0f, 1.0f);
            right[i] = std::clamp (right[i] * gain_, -1.0f, 1.0f);
        }
    }

    [[nodiscard]] float getGain() const noexcept { return gain_; }

private:
    double sampleRate_  = 44100.0;
    float threshold_    = defaultThreshold;
    float releaseMs_    = defaultReleaseMs;
    float releaseCoeff_ = 0.0f;
    float gain_         = 1.0f;
};

} // namespace aeonix
