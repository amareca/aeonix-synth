#pragma once

#include <algorithm>
#include <cmath>

namespace aeonix
{

// ============================================================================
//  Limiter — limitador de seguridad para el bus master.
//
//  Detector de pico linkado (la misma reduccion se aplica a L y R, para no
//  mover la imagen estereo) con ataque inmediato y release exponencial.
//  No aplica makeup gain: por debajo del umbral es transparente. Un clip
//  duro final garantiza que la salida nunca supere +/-1.
//
//  RT-safe: solo aritmetica por muestra, sin asignaciones ni ramas costosas.
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

        // Coeficiente de una pole que vuelve a ~63 % del camino en releaseMs.
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
                gain_ = target;                                        // ataque inmediato
            else
                gain_ = gain_ * releaseCoeff_ + (1.0f - releaseCoeff_); // release suave

            // Red de seguridad: el clip duro solo actua si algo se escapa.
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
