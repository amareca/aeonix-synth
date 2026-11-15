#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>

#include "domain/Oscillator.h"

using namespace aeonix;

TEST_CASE ("Oscillator generates a sine at quarter sample rate", "[oscillator]")
{
    const double sr = 44100.0;

    Oscillator osc;
    osc.setSampleRate (sr);
    osc.setWaveform (Waveform::Sine);
    osc.setFrequency (sr / 4.0);
    osc.reset();

    // A sine at sr/4 gives: 0, +1, 0, -1, 0 ...
    REQUIRE (osc.nextSample() == Catch::Approx (0.0f).margin (1e-4f));
    REQUIRE (osc.nextSample() == Catch::Approx (1.0f).margin (1e-4f));
    REQUIRE (osc.nextSample() == Catch::Approx (0.0f).margin (1e-4f));
    REQUIRE (osc.nextSample() == Catch::Approx (-1.0f).margin (1e-4f));
    REQUIRE (osc.nextSample() == Catch::Approx (0.0f).margin (1e-4f));
}

TEST_CASE ("Oscillator saw rises linearly away from the discontinuity", "[oscillator]")
{
    const double sr = 1000.0;

    Oscillator osc;
    osc.setSampleRate (sr);
    osc.setWaveform (Waveform::Saw);
    osc.setFrequency (25.0);   // 40 samples per period (dt = 0.025)
    osc.reset();

    // PolyBLEP smooths the jump: the first sample no longer drops to -1.
    REQUIRE (std::fabs (osc.nextSample()) < 1.0f);

    // Outside the jump edges the saw stays linear.
    for (int i = 1; i < 39; ++i)
    {
        const float expected = static_cast<float> (2.0 * (i * 0.025) - 1.0);
        REQUIRE (osc.nextSample() == Catch::Approx (expected).margin (1e-3f));
    }
}

TEST_CASE ("Oscillator output stays within its level", "[oscillator]")
{
    const double sr = 1000.0;

    Oscillator osc;
    osc.setSampleRate (sr);
    osc.setWaveform (Waveform::Sine);
    osc.setFrequency (7.0);
    osc.setLevel (0.5f);
    osc.reset();

    for (int i = 0; i < 1000; ++i)
    {
        const float s = osc.nextSample();
        REQUIRE (s >= -0.5f);
        REQUIRE (s <= 0.5f);
    }
}
