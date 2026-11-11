#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include "domain/Oscillator.h"

// El optimizador de MSVC se cae generando codigo para las funciones de medida
// espectral de este archivo (CL.exe termina con un error interno). Como es un
// test y su coste no importa, se compila sin optimizar.
#if defined(_MSC_VER)
    #pragma optimize ("", off)
#endif

using namespace aeonix;

namespace
{

constexpr double pi = 3.14159265358979323846;

// Magnitud de Goertzel en una frecuencia concreta (sin FFT ni JUCE).
double goertzelMagnitude (const std::vector<float>& samples,
                          double frequency,
                          double sampleRate)
{
    const double omega = 2.0 * pi * frequency / sampleRate;
    const double coeff = 2.0 * std::cos (omega);

    double s1 = 0.0;
    double s2 = 0.0;

    for (const float sample : samples)
    {
        const double s0 = static_cast<double> (sample) + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    return std::sqrt (std::max (0.0, s1 * s1 + s2 * s2 - coeff * s1 * s2));
}

std::vector<float> renderSaw (double frequency, int numSamples)
{
    Oscillator osc;
    osc.setSampleRate (44100.0);
    osc.setWaveform (Waveform::Saw);
    osc.setFrequency (frequency);
    osc.reset();

    std::vector<float> out (static_cast<size_t> (numSamples));

    for (auto& sample : out)
        sample = osc.nextSample();

    return out;
}

// Sierra ideal sin correccion, como referencia de lo que aliasa el naive.
std::vector<float> renderNaiveSaw (double frequency, int numSamples)
{
    const double increment = frequency / 44100.0;

    std::vector<float> out (static_cast<size_t> (numSamples));

    for (int i = 0; i < numSamples; ++i)
    {
        const double phase = std::fmod (i * increment, 1.0);
        out[static_cast<size_t> (i)] = static_cast<float> (2.0 * phase - 1.0);
    }

    return out;
}

} // namespace

TEST_CASE ("PolyBLEP saw has less alias energy than the naive saw", "[aliasing]")
{
    constexpr double sampleRate  = 44100.0;
    constexpr double fundamental = 3000.0;

    // El 8o armonico cae en 24 kHz (> Nyquist) y se pliega a 20.1 kHz.
    constexpr double aliasFrequency = sampleRate - 8.0 * fundamental;

    const int numSamples = 16384;

    const auto bandLimited = renderSaw (fundamental, numSamples);
    const auto naive       = renderNaiveSaw (fundamental, numSamples);

    const double blepAlias  = goertzelMagnitude (bandLimited, aliasFrequency, sampleRate);
    const double naiveAlias = goertzelMagnitude (naive, aliasFrequency, sampleRate);

    const double blepFundamental  = goertzelMagnitude (bandLimited, fundamental, sampleRate);
    const double naiveFundamental = goertzelMagnitude (naive, fundamental, sampleRate);

    // La fundamental se conserva (y su energia es comparable).
    REQUIRE (blepFundamental > 0.0);
    REQUIRE (blepFundamental > naiveFundamental * 0.5);

    // Donde el naive deja un alias claro, el PolyBLEP lo reduce de forma
    // notoria (en la practica bastante mas que la mitad).
    REQUIRE (naiveAlias > 0.0);
    REQUIRE (blepAlias < naiveAlias * 0.5);
}

TEST_CASE ("PolyBLEP square stays within a reasonable range", "[aliasing]")
{
    Oscillator osc;
    osc.setSampleRate (44100.0);
    osc.setWaveform (Waveform::Square);
    osc.setFrequency (2000.0);
    osc.reset();

    float peak = 0.0f;

    for (int i = 0; i < 4096; ++i)
    {
        const float sample = osc.nextSample();
        peak = std::max (peak, std::fabs (sample));
    }

    // La correccion puede superar ligeramente +/-1 (Gibbs), pero no dispararse.
    REQUIRE (peak > 0.5f);
    REQUIRE (peak < 1.25f);
}
