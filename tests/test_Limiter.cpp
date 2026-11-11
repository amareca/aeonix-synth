#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include "domain/Limiter.h"

using namespace aeonix;

namespace
{

float processPeak (Limiter& limiter, float amplitude, int numSamples, double frequency)
{
    std::vector<float> left (static_cast<size_t> (numSamples));
    std::vector<float> right (static_cast<size_t> (numSamples));

    for (int i = 0; i < numSamples; ++i)
    {
        const float sample = amplitude * std::sin (
            2.0f * 3.14159265358979323846f * static_cast<float> (frequency)
            * static_cast<float> (i) / 48000.0f);

        left[static_cast<size_t> (i)] = sample;
        right[static_cast<size_t> (i)] = sample;
    }

    limiter.process (left.data(), right.data(), numSamples);

    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        peak = std::max (peak, std::fabs (left[static_cast<size_t> (i)]));
        peak = std::max (peak, std::fabs (right[static_cast<size_t> (i)]));
    }

    return peak;
}

} // namespace

TEST_CASE ("Limiter keeps the output at or below unity", "[limiter]")
{
    Limiter limiter;
    limiter.prepare (48000.0);

    const float peak = processPeak (limiter, 4.0f, 4096, 220.0f);

    REQUIRE (peak > 0.9f);
    REQUIRE (peak <= 1.0f);
}

TEST_CASE ("Limiter is transparent below its threshold", "[limiter]")
{
    Limiter limiter;
    limiter.prepare (48000.0);

    std::vector<float> left (512);
    std::vector<float> right (512);

    for (int i = 0; i < 512; ++i)
    {
        const float sample = 0.5f * std::sin (
            2.0f * 3.14159265358979323846f * 440.0f * static_cast<float> (i) / 48000.0f);

        left[static_cast<size_t> (i)] = sample;
        right[static_cast<size_t> (i)] = sample;
    }

    auto expectedLeft = left;
    auto expectedRight = right;

    limiter.process (left.data(), right.data(), 512);

    for (int i = 0; i < 512; ++i)
    {
        REQUIRE (left[static_cast<size_t> (i)] == expectedLeft[static_cast<size_t> (i)]);
        REQUIRE (right[static_cast<size_t> (i)] == expectedRight[static_cast<size_t> (i)]);
    }
}

TEST_CASE ("Limiter lets silence through", "[limiter]")
{
    Limiter limiter;
    limiter.prepare (44100.0);

    std::vector<float> left (256, 0.0f);
    std::vector<float> right (256, 0.0f);

    limiter.process (left.data(), right.data(), 256);

    for (int i = 0; i < 256; ++i)
    {
        REQUIRE (left[static_cast<size_t> (i)] == 0.0f);
        REQUIRE (right[static_cast<size_t> (i)] == 0.0f);
    }

    REQUIRE (limiter.getGain() == 1.0f);
}
