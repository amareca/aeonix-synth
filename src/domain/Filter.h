#pragma once

#include <algorithm>

#include "MathUtils.h"

namespace aeonix
{

// ============================================================================
//  Filter — filtro paso-bajo resonante (biquad, tipo RBJ).
//
//  Atenua las frecuencias por encima de "cutoff"; la resonancia (Q) crea un
//  pico en el corte. Coeficientes recalculados en setCutoff/setResonance (no
//  en el bucle de audio), por lo que process() es RT-safe.
// ============================================================================
class Filter
{
public:
    Filter() = default;

    void prepare (double sampleRate)
    {
        sampleRate_ = sampleRate;
        updateCoefficients();
    }

    void setCutoff (double hz)
    {
        cutoff_ = hz;
        updateCoefficients();
    }

    // q = factor de calidad (mayor => mas resonancia). Se limita para estabilidad.
    void setResonance (double q)
    {
        q_ = std::clamp (q, 0.1, 20.0);
        updateCoefficients();
    }

    void reset()
    {
        x1_ = x2_ = y1_ = y2_ = 0.0;
    }

    float process (float input)
    {
        const double x = input;
        const double y = b0_ * x + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;

        x2_ = x1_;
        x1_ = x;
        y2_ = y1_;
        y1_ = y;

        return static_cast<float> (y);
    }

private:
    void updateCoefficients()
    {
        if (sampleRate_ <= 0.0)
            return;

        const double cutoff = std::clamp (cutoff_, 1.0, sampleRate_ * 0.49);
        const double w0 = twoPi * cutoff / sampleRate_;
        const double cosw0 = std::cos (w0);
        const double alpha = std::sin (w0) / (2.0 * q_);

        const double b0 = (1.0 - cosw0) * 0.5;
        const double b1 = 1.0 - cosw0;
        const double b2 = (1.0 - cosw0) * 0.5;
        const double a0 = 1.0 + alpha;
        const double a1 = -2.0 * cosw0;
        const double a2 = 1.0 - alpha;

        b0_ = b0 / a0;
        b1_ = b1 / a0;
        b2_ = b2 / a0;
        a1_ = a1 / a0;
        a2_ = a2 / a0;
    }

    double sampleRate_ = 44100.0;
    double cutoff_     = 20000.0;
    double q_          = 0.7071067811865475;   // Butterworth

    double b0_ = 1.0, b1_ = 0.0, b2_ = 0.0;
    double a1_ = 0.0, a2_ = 0.0;

    double x1_ = 0.0, x2_ = 0.0;
    double y1_ = 0.0, y2_ = 0.0;
};

} // namespace aeonix
