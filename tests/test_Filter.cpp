#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <algorithm>
#include <cmath>

#include "domain/Filter.h"

using namespace aeonix;

TEST_CASE ("Low-pass filter passes DC (gain ~1)", "[filter]")
{
    Filter filter;
    filter.prepare (44100.0);
    filter.setCutoff (1000.0);
    filter.reset();

    float out = 0.0f;
    for (int i = 0; i < 2000; ++i)
        out = filter.process (1.0f);   // senal constante (DC)

    REQUIRE (out == Catch::Approx (1.0f).margin (1e-3f));
}

TEST_CASE ("Low-pass filter attenuates the Nyquist frequency", "[filter]")
{
    Filter filter;
    filter.prepare (44100.0);
    filter.setCutoff (1000.0);
    filter.reset();

    float peak = 0.0f;
    for (int i = 0; i < 2000; ++i)
    {
        const float input = (i % 2 == 0) ? 1.0f : -1.0f;   // 22.05 kHz
        const float out = filter.process (input);

        if (i > 1000)   // ignora el transitorio inicial
            peak = std::max (peak, std::fabs (out));
    }

    REQUIRE (peak < 0.05f);
}
