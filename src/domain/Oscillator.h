#pragma once

#include "MathUtils.h"

namespace aeonix
{

// Waveforms supported by the oscillator (and the LFO).
enum class Waveform
{
    Saw,
    Square,
    Sine,
    Triangle
};

// ============================================================================
//  Oscillator - oscillator with waveform, frequency and level.
//
//  Normalized phase range [0, 1). nextSample() is called once per
//  sample; it does no hot-path memory allocations (RT-safe).
//
//  Saw and square use PolyBLEP (first-order band-limited correction)
//  to reduce aliasing. Sine and triangle are generated directly:
//  the triangle has harmonics at 1/n^2 and its aliasing is negligible.
//
//  getValue() still returns the "naive" wave: used by the LFO and by the
//  waveform drawing, where the correction adds nothing.
// ============================================================================
class Oscillator
{
public:
    Oscillator() = default;

    void setSampleRate (double sr)
    {
        sampleRate_ = sr;
        updatePhaseIncrement();
    }

    void setFrequency (double hz)
    {
        frequency_ = hz;
        updatePhaseIncrement();
    }

    void setWaveform (Waveform w) noexcept { waveform_ = w; }

    void setLevel (float level) noexcept { level_ = level; }

    void reset() noexcept { phase_ = 0.0; }

    // Advances the phase without returning a value (for "control rate" use, e.g. LFO).
    void advance (int samples)
    {
        phase_ += phaseIncrement_ * samples;
        if (phase_ >= 1.0)
            phase_ -= std::floor (phase_);   // wrap to [0, 1)
    }

    // Value of the wave at the current phase, without advancing.
    [[nodiscard]] float getValue() const
    {
        return valueAtPhase (phase_) * level_;
    }

    // Generates the next sample in [-level, +level].
    float nextSample()
    {
        const float out = valueAtPhaseBandLimited (phase_) * level_;
        advance (1);
        return out;
    }

private:
    // First-order PolyBLEP correction around a phase jump.
    // t: normalized phase [0, 1). dt: phase increment per sample.
    // Returns 0 away from the jump and a quadratic branch at the edges.
    [[nodiscard]] static double polyBlep (double t, double dt) noexcept
    {
        if (dt <= 0.0)
            return 0.0;

        if (t < dt)
        {
            const double x = t / dt;
            return x + x - x * x - 1.0;
        }

        if (t > 1.0 - dt)
        {
            const double x = (t - 1.0) / dt;
            return x * x + x + x + 1.0;
        }

        return 0.0;
    }

    [[nodiscard]] float valueAtPhaseBandLimited (double p) const
    {
        switch (waveform_)
        {
            case Waveform::Saw:
                return static_cast<float> (2.0 * p - 1.0 - polyBlep (p, phaseIncrement_));

            case Waveform::Square:
            {
                double value = (p < 0.5) ? 1.0 : -1.0;

                // Jump at phase 0 (rising) and at phase 0.5 (falling).
                value += polyBlep (p, phaseIncrement_);

                double halfPhase = p - 0.5;
                if (halfPhase < 0.0)
                    halfPhase += 1.0;

                value -= polyBlep (halfPhase, phaseIncrement_);
                return static_cast<float> (value);
            }

            default:
                return valueAtPhase (p);
        }
    }

    [[nodiscard]] float valueAtPhase (double p) const
    {
        switch (waveform_)
        {
            case Waveform::Saw:      return static_cast<float> (2.0 * p - 1.0);
            case Waveform::Square:   return (p < 0.5) ? 1.0f : -1.0f;
            case Waveform::Sine:     return static_cast<float> (std::sin (twoPi * p));
            case Waveform::Triangle: return static_cast<float> (1.0 - 4.0 * std::abs (p - 0.5));
        }

        return 0.0f;
    }

    void updatePhaseIncrement()
    {
        phaseIncrement_ = (sampleRate_ > 0.0) ? frequency_ / sampleRate_ : 0.0;
    }

    double sampleRate_      = 44100.0;
    double frequency_       = 440.0;
    double phase_           = 0.0;
    double phaseIncrement_  = 440.0 / 44100.0;
    Waveform waveform_      = Waveform::Sine;
    float level_            = 1.0f;
};

} // namespace aeonix
