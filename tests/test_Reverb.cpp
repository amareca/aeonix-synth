#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>
#include <vector>

#include "domain/Reverb.h"

using namespace aeonix;

TEST_CASE ("Reverb produces a tail after an impulse", "[reverb]")
{
    Reverb reverb;
    reverb.prepare (44100.0);
    reverb.setMix (1.0f);
    reverb.reset();

    const int n = 3000;
    std::vector<float> left (n, 0.0f), right (n, 0.0f);
    left[0] = 1.0f;   // impulso

    reverb.process (left.data(), right.data(), n);

    // Debe haber salida no nula despues del impulso (cola de reverberacion).
    bool foundTail = false;
    for (int i = 100; i < n; ++i)
        if (std::fabs (left[i]) > 0.0001f) { foundTail = true; break; }

    REQUIRE (foundTail);
}

TEST_CASE ("Reverb with mix 0 passes the signal unchanged", "[reverb]")
{
    Reverb reverb;
    reverb.prepare (44100.0);
    reverb.setMix (0.0f);
    reverb.reset();

    const int n = 64;
    std::vector<float> left (n, 0.0f), right (n, 0.0f);
    for (int i = 0; i < n; ++i)
        left[i] = 0.5f;

    reverb.process (left.data(), right.data(), n);

    for (int i = 0; i < n; ++i)
        REQUIRE (left[i] == Catch::Approx (0.5f));
}
