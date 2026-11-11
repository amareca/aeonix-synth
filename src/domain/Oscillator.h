#pragma once

#include "MathUtils.h"

namespace aeonix
{

// Formas de onda soportadas por el oscilador (y el LFO).
enum class Waveform
{
    Saw,
    Square,
    Sine,
    Triangle
};

// ============================================================================
//  Oscillator — oscilador con forma de onda, frecuencia y nivel.
//
//  Rango de fase normalizado [0, 1). Se llama a nextSample() una vez por
//  muestra; no hace asignaciones de memoria en caliente (RT-safe).
//
//  Sierra y cuadrada usan PolyBLEP (correccion de banda limitada de primer
//  orden) para reducir el aliasing. Seno y triangulo se generan de forma
//  directa: el triangulo tiene armonicos a 1/n^2 y su aliasing es inapreciable.
//
//  getValue() sigue devolviendo la onda "naive": lo usan el LFO y el dibujo de
//  formas de onda, donde la correccion no aporta nada.
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

    // Avanza la fase sin devolver valor (para uso a "control rate", p.ej. LFO).
    void advance (int samples)
    {
        phase_ += phaseIncrement_ * samples;
        if (phase_ >= 1.0)
            phase_ -= std::floor (phase_);   // wrap a [0, 1)
    }

    // Valor de la onda en la fase actual, sin avanzar.
    [[nodiscard]] float getValue() const
    {
        return valueAtPhase (phase_) * level_;
    }

    // Genera la siguiente muestra en [-level, +level].
    float nextSample()
    {
        const float out = valueAtPhaseBandLimited (phase_) * level_;
        advance (1);
        return out;
    }

private:
    // Correccion PolyBLEP de primer orden alrededor de un salto de fase.
    // t: fase normalizada [0, 1). dt: incremento de fase por muestra.
    // Devuelve 0 lejos del salto y una rama cuadratica en los bordes.
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

                // Salto en fase 0 (subida) y en fase 0.5 (bajada).
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
