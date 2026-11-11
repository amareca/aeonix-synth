#pragma once

#include "Oscillator.h"

namespace aeonix
{

// ============================================================================
//  Lfo — Low Frequency Oscillator (oscilador lento que modula, no se oye).
//
//  Reutiliza Oscillator con una frecuencia muy baja y anade una "depth"
//  (profundidad) que escala la salida. nextSample() devuelve un valor bipolar
//  en [-depth, +depth].
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

    // Avanza la fase sin devolver (para modulacion a "control rate").
    void advance (int samples) { osc_.advance (samples); }

    // Valor de modulacion actual (bipolar, en [-depth, +depth]) sin avanzar.
    [[nodiscard]] float getValue() const { return depth_ * osc_.getValue(); }

private:
    Oscillator osc_;
    float depth_ = 0.0f;
};

} // namespace aeonix
