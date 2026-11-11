#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <vector>

#include "domain/Delay.h"

using namespace aeonix;

TEST_CASE ("Delay echoes an impulse (dry + delayed copy)", "[delay]")
{
    Delay delay;
    delay.prepare (44100.0, 2.0);
    delay.setTime (0.001);      // ~44 muestras
    delay.setFeedback (0.0f);
    delay.setMix (0.5f);
    delay.reset();

    const int n = 100;
    std::vector<float> left (n, 0.0f), right (n, 0.0f);
    left[0] = 1.0f;             // impulso

    delay.process (left.data(), right.data(), n);

    // Muestra 0: senal seca = 1.0 * (1 - 0.5) = 0.5.
    REQUIRE (left[0] == Catch::Approx (0.5f));

    // Debe aparecer una copia retardada (eco) mas adelante.
    bool foundEcho = false;
    for (int i = 1; i < n; ++i)
        if (left[i] > 0.3f) { foundEcho = true; break; }

    REQUIRE (foundEcho);
}

TEST_CASE ("Delay with mix 0 passes the signal unchanged", "[delay]")
{
    Delay delay;
    delay.prepare (44100.0, 2.0);
    delay.setMix (0.0f);
    delay.reset();

    const int n = 64;
    std::vector<float> left (n, 0.0f), right (n, 0.0f);
    for (int i = 0; i < n; ++i)
        left[i] = 0.25f;

    delay.process (left.data(), right.data(), n);

    for (int i = 0; i < n; ++i)
        REQUIRE (left[i] == Catch::Approx (0.25f));
}
