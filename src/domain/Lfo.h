#pragma once

#include "Oscillator.h"

namespace aeonix
{

// ============================================================================
//  Lfo - Low Frequency Oscillator (slow oscillator that modulates, not heard).
//
//  Reuses Oscillator at a very low frequency and adds a depth
//  that scales the output. nextSample() returns a bipolar value
//  in [-depth, +depth].
// ============================================================================
class Lfo
{
public:
    Lfo() = default;

    void prepare (double sampleRate) { osc_.setSampleRate (sampleRate); }
    void setRate (double hz)         { osc_.setFrequency (hz); }
    void setWaveform (Waveform w)    { osc_.setWaveform (w); }
    void setDepth (float depth) noexcept { depth_ = depth; }

    void reset() { osc_.reset(); }

    [[nodiscard]] float getDepth() const noexcept { return depth_; }

    float nextSample()
    {
        return depth_ * osc_.nextSample();
    }

    // Advances the phase without returning (for control-rate modulation).
    void advance (int samples) { osc_.advance (samples); }

    // Current modulation value (bipolar, in [-depth, +depth]) without advancing.
    [[nodiscard]] float getValue() const { return depth_ * osc_.getValue(); }

private:
    Oscillator osc_;
    float depth_ = 0.0f;
};

} // namespace aeonix
