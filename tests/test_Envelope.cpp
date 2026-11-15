#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <algorithm>

#include "domain/Envelope.h"

using namespace aeonix;

namespace
{
float run (aeonix::Envelope& env, int samples)
{
    float value = 0.0f;
    for (int i = 0; i < samples; ++i)
        value = env.nextSample();
    return value;
}
}

TEST_CASE ("Envelope attack rises linearly and clamps at 1", "[envelope]")
{
    Envelope env;
    env.prepare (1000.0);          // 1 second = 1000 samples
    env.setAttack (1.0);
    env.setDecay (100.0);
    env.setSustain (0.9);
    env.setRelease (100.0);

    env.noteOn();
    REQUIRE (env.getStage() == Envelope::Stage::Attack);

    // Halfway through the attack (500 samples) the value should be ~0.5.
    REQUIRE (run (env, 500) == Catch::Approx (0.5).margin (0.02));

    // When the attack completes it reaches 1.0 and never exceeds it.
    float maxValue = 0.0f;
    for (int i = 0; i < 600; ++i)
        maxValue = std::max (maxValue, env.nextSample());

    REQUIRE (maxValue == Catch::Approx (1.0).margin (1e-3));
}

TEST_CASE ("Envelope reaches sustain and holds", "[envelope]")
{
    Envelope env;
    env.prepare (1000.0);
    env.setAttack (0.1);
    env.setDecay (0.1);
    env.setSustain (0.5);
    env.setRelease (0.1);

    env.noteOn();
    run (env, 200);                 // attack + decay completed

    REQUIRE (env.getStage() == Envelope::Stage::Sustain);
    REQUIRE (env.getValue() == Catch::Approx (0.5).margin (1e-3));

    run (env, 500);                 // still sustained
    REQUIRE (env.getStage() == Envelope::Stage::Sustain);
    REQUIRE (env.getValue() == Catch::Approx (0.5).margin (1e-3));
}

TEST_CASE ("Envelope releases to zero and becomes idle", "[envelope]")
{
    Envelope env;
    env.prepare (1000.0);
    env.setAttack (0.001);
    env.setDecay (0.001);
    env.setSustain (0.5);
    env.setRelease (0.1);

    env.noteOn();
    run (env, 50);                  // reaches sustain
    REQUIRE (env.getStage() == Envelope::Stage::Sustain);

    env.noteOff();
    REQUIRE (env.getStage() == Envelope::Stage::Release);

    run (env, 200);                 // 0.1s * 1000 = 100 release samples
    REQUIRE (env.getStage() == Envelope::Stage::Idle);
    REQUIRE (env.getValue() == Catch::Approx (0.0).margin (1e-3));
    REQUIRE_FALSE (env.isActive());
}

TEST_CASE ("Envelope active state follows noteOn/noteOff", "[envelope]")
{
    Envelope env;
    env.prepare (1000.0);

    REQUIRE_FALSE (env.isActive());
    env.noteOn();
    REQUIRE (env.isActive());
}

TEST_CASE ("Envelope retrigger keeps the current level to avoid clicks", "[envelope]")
{
    Envelope env;
    env.prepare (1000.0);
    env.setAttack (0.001);
    env.setDecay (0.001);
    env.setSustain (1.0);
    env.setRelease (0.5);

    env.noteOn();
    run (env, 10);

    env.noteOff();
    run (env, 5);

    const double before = env.getValue();
    REQUIRE (before > 0.5);

    // When the voice is recycled it does not drop to zero: the attack starts from the
    // current level, which is what avoids the voice stealing click.
    env.noteOn();
    REQUIRE (env.getStage() == Envelope::Stage::Attack);
    REQUIRE (env.getValue() == Catch::Approx (before).margin (1e-12));

    const float after = env.nextSample();
    REQUIRE (after > static_cast<float> (before));
}
