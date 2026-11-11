#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "domain/Parameter.h"

using namespace aeonix;

TEST_CASE ("Parameter clamps normalised values to [0,1]", "[parameter]")
{
    Parameter p ("filter.cutoff", "Cutoff", 20.0f, 20000.0f, 0.5f);

    p.setNormalised (2.0f);
    REQUIRE (p.getNormalised() == Catch::Approx (1.0f));

    p.setNormalised (-1.0f);
    REQUIRE (p.getNormalised() == Catch::Approx (0.0f));
}

TEST_CASE ("Parameter maps normalised to real value", "[parameter]")
{
    Parameter p ("amp.sustain", "Sustain", 0.0f, 1.0f, 0.7f);
    p.setNormalised (0.5f);
    REQUIRE (p.getValue() == Catch::Approx (0.5f));

    Parameter cutoff ("filter.cutoff", "Cutoff", 20.0f, 20000.0f, 0.0f);
    cutoff.setNormalised (0.5f);
    REQUIRE (cutoff.getValue() == Catch::Approx (10010.0f));   // 20 + 0.5 * (20000 - 20)
}

TEST_CASE ("Parameter setValue round-trips through the range", "[parameter]")
{
    Parameter p ("x", "X", 0.0f, 10.0f, 0.0f);
    p.setValue (7.5f);
    REQUIRE (p.getNormalised() == Catch::Approx (0.75f));
    REQUIRE (p.getValue() == Catch::Approx (7.5f));
}

TEST_CASE ("Parameter default is clamped to range", "[parameter]")
{
    Parameter p ("x", "X", 0.0f, 1.0f, 1.5f);
    REQUIRE (p.getNormalised() == Catch::Approx (1.0f));
}

TEST_CASE ("Parameter logarithmic mapping", "[parameter]")
{
    // 20..20000 log: el punto medio (0.5) = 20 * sqrt(1000) ~= 632.46 Hz.
    Parameter cutoff ("filter.cutoff", "Cutoff", 20.0f, 20000.0f, 0.5f, true);
    REQUIRE (cutoff.getValue() == Catch::Approx (632.46f).margin (1.0f));

    // setValue hace la conversion inversa.
    cutoff.setValue (632.46f);
    REQUIRE (cutoff.getNormalised() == Catch::Approx (0.5f).margin (0.01f));
}
