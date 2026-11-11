#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "domain/Lfo.h"

using namespace aeonix;

TEST_CASE ("LFO output follows a sine scaled by depth", "[lfo]")
{
    const double sr = 44100.0;

    Lfo lfo;
    lfo.prepare (sr);
    lfo.setRate (sr / 4.0);
    lfo.setWaveform (Waveform::Sine);
    lfo.setDepth (0.5f);
    lfo.reset();

    // sr/4 => ciclo en 4 muestras: 0, +0.5, 0, -0.5
    REQUIRE (lfo.nextSample() == Catch::Approx (0.0f).margin (1e-4f));
    REQUIRE (lfo.nextSample() == Catch::Approx (0.5f).margin (1e-4f));
    REQUIRE (lfo.nextSample() == Catch::Approx (0.0f).margin (1e-4f));
    REQUIRE (lfo.nextSample() == Catch::Approx (-0.5f).margin (1e-4f));
}

TEST_CASE ("LFO with zero depth outputs silence", "[lfo]")
{
    Lfo lfo;
    lfo.prepare (44100.0);
    lfo.setDepth (0.0f);
    lfo.reset();

    for (int i = 0; i < 100; ++i)
        REQUIRE (lfo.nextSample() == Catch::Approx (0.0f).margin (1e-6f));
}
