#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>

#include "domain/MathUtils.h"

using namespace aeonix;

TEST_CASE ("pitchBendRatio follows the semitone range and clamps", "[math]")
{
    REQUIRE (pitchBendRatio (0.0f) == Catch::Approx (1.0));
    REQUIRE (pitchBendRatio (1.0f) == Catch::Approx (std::pow (2.0, 2.0 / 12.0)));
    REQUIRE (pitchBendRatio (-1.0f) == Catch::Approx (std::pow (2.0, -2.0 / 12.0)));

    // Out of range it is clamped to -1..+1.
    REQUIRE (pitchBendRatio (5.0f) == Catch::Approx (pitchBendRatio (1.0f)));
}

TEST_CASE ("panGains keeps unity at centre and silences the far side", "[math]")
{
    float left = 0.0f;
    float right = 0.0f;

    panGains (0.0f, left, right);
    REQUIRE (left == Catch::Approx (1.0f));
    REQUIRE (right == Catch::Approx (1.0f));

    panGains (1.0f, left, right);
    REQUIRE (left == Catch::Approx (0.0f));
    REQUIRE (right == Catch::Approx (1.0f));

    panGains (-1.0f, left, right);
    REQUIRE (left == Catch::Approx (1.0f));
    REQUIRE (right == Catch::Approx (0.0f));

    panGains (0.5f, left, right);
    REQUIRE (left == Catch::Approx (0.5f));
    REQUIRE (right == Catch::Approx (1.0f));
}

TEST_CASE ("effectiveLfoDepth adds the mod wheel and clamps", "[math]")
{
    REQUIRE (effectiveLfoDepth (0.0f, 0.0f) == Catch::Approx (0.0f));
    REQUIRE (effectiveLfoDepth (0.2f, 0.3f) == Catch::Approx (0.5f));
    REQUIRE (effectiveLfoDepth (0.8f, 1.0f) == Catch::Approx (1.0f));
}

TEST_CASE ("effectiveCutoff combines base, filter envelope and LFO", "[math]")
{
    // No envelope and no LFO: base cutoff.
    REQUIRE (effectiveCutoff (1000.0f, 1.0f, 0.0f, 0.0f) == Catch::Approx (1000.0f));

    // Envelope at maximum and amount 1 -> +6 octaves (x64).
    REQUIRE (effectiveCutoff (100.0f, 1.0f, 1.0f, 0.0f) == Catch::Approx (6400.0f));

    // Bipolar LFO multiplies the cutoff.
    REQUIRE (effectiveCutoff (1000.0f, 0.0f, 0.0f, 0.5f) == Catch::Approx (1500.0f));

    // It is clamped to the audible range.
    REQUIRE (effectiveCutoff (1000.0f, 1.0f, 1.0f, 0.0f) == Catch::Approx (20000.0f));
    REQUIRE (effectiveCutoff (20.0f, 0.0f, 0.0f, -1.0f) == Catch::Approx (20.0f));
}
