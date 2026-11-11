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
    env.prepare (1000.0);          // 1 segundo = 1000 muestras
    env.setAttack (1.0);
    env.setDecay (100.0);
    env.setSustain (0.9);
    env.setRelease (100.0);

    env.noteOn();
    REQUIRE (env.getStage() == Envelope::Stage::Attack);

    // A mitad del ataque (500 muestras) el valor deberia ser ~0.5.
    REQUIRE (run (env, 500) == Catch::Approx (0.5).margin (0.02));

    // Al completarse el ataque llega a 1.0 y nunca lo supera.
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
    run (env, 200);                 // ataque + decaimiento completados

    REQUIRE (env.getStage() == Envelope::Stage::Sustain);
    REQUIRE (env.getValue() == Catch::Approx (0.5).margin (1e-3));

    run (env, 500);                 // sigue sostenido
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
    run (env, 50);                  // llega a sustain
    REQUIRE (env.getStage() == Envelope::Stage::Sustain);

    env.noteOff();
    REQUIRE (env.getStage() == Envelope::Stage::Release);

    run (env, 200);                 // 0.1s * 1000 = 100 muestras de release
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

    // Al reciclar la voz no se cae a cero: el ataque arranca desde el nivel
    // actual, que es lo que evita el clic del voice stealing.
    env.noteOn();
    REQUIRE (env.getStage() == Envelope::Stage::Attack);
    REQUIRE (env.getValue() == Catch::Approx (before).margin (1e-12));

    const float after = env.nextSample();
    REQUIRE (after > static_cast<float> (before));
}
