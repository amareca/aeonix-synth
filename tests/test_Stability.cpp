#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

#include "domain/ParameterIds.h"
#include "domain/SynthEngine.h"

using namespace aeonix;

namespace
{

// Renderiza "blocks" bloques y comprueba que toda la señal es finita.
// Devuelve el pico absoluto encontrado.
bool renderIsFinite (SynthEngine& engine, int blocks, int blockSize, float& peak)
{
    std::vector<float> left (static_cast<size_t> (blockSize), 0.0f);
    std::vector<float> right (static_cast<size_t> (blockSize), 0.0f);

    for (int b = 0; b < blocks; ++b)
    {
        engine.render (left.data(), right.data(), blockSize);

        for (int i = 0; i < blockSize; ++i)
        {
            if (! std::isfinite (left[i]) || ! std::isfinite (right[i]))
                return false;

            peak = std::max (peak, std::fabs (left[i]));
            peak = std::max (peak, std::fabs (right[i]));
        }
    }

    return true;
}

void setParam (SynthEngine& engine, const char* id, float normalised)
{
    engine.setParameter (id, normalised);
}

} // namespace

TEST_CASE ("SynthEngine stays finite with extreme settings", "[stability]")
{
    SynthEngine engine;
    engine.prepare (48000.0, 256, SynthEngine::maxVoiceCapacity);
    engine.setPolyphony (32);

    setParam (engine, param::masterVolume,    1.0f);
    setParam (engine, param::oscLevel,        1.0f);
    setParam (engine, param::filterCutoff,    0.0f);   // 20 Hz
    setParam (engine, param::filterResonance, 1.0f);   // Q máxima
    setParam (engine, param::filterEnvAmount, 1.0f);
    setParam (engine, param::filterAttack,    0.0f);
    setParam (engine, param::filterDecay,     0.0f);
    setParam (engine, param::filterSustain,   1.0f);
    setParam (engine, param::filterRelease,   0.0f);
    setParam (engine, param::ampAttack,       0.0f);
    setParam (engine, param::ampDecay,        0.0f);
    setParam (engine, param::ampSustain,      1.0f);
    setParam (engine, param::ampRelease,      0.0f);
    setParam (engine, param::delayTime,       1.0f);
    setParam (engine, param::delayFeedback,   1.0f);   // 0.9 real
    setParam (engine, param::delayMix,        1.0f);
    setParam (engine, param::reverbSize,      1.0f);
    setParam (engine, param::reverbMix,       1.0f);
    setParam (engine, param::lfoRate,         1.0f);
    setParam (engine, param::lfoDepth,        1.0f);
    setParam (engine, param::unisonVoices,    1.0f);   // 8 voces
    setParam (engine, param::unisonDetune,    1.0f);
    setParam (engine, param::unisonWidth,     1.0f);

    const int notes[] = { 36, 48, 60, 64, 67, 72, 76, 84 };
    for (int note : notes)
        engine.noteOn (note, 1.0f);

    float peak = 0.0f;
    REQUIRE (renderIsFinite (engine, 400, 256, peak));
    REQUIRE (engine.getActiveVoiceCount() > 0);
    REQUIRE (peak > 0.0f);
    REQUIRE (peak < 1000.0f);

    for (int note : notes)
        engine.noteOff (note);

    // Cola larga con delay/reverb al máximo: sigue siendo finita.
    REQUIRE (renderIsFinite (engine, 200, 256, peak));
    REQUIRE (peak < 1000.0f);
}

TEST_CASE ("SynthEngine survives rapid parameter changes", "[stability]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 128, SynthEngine::maxVoiceCapacity);
    engine.setPolyphony (16);

    engine.noteOn (60, 1.0f);
    engine.noteOn (64, 0.8f);
    engine.noteOn (67, 0.6f);

    float peak = 0.0f;

    for (int block = 0; block < 200; ++block)
    {
        const float on = (block % 2 == 0) ? 1.0f : 0.0f;

        setParam (engine, param::filterCutoff,    on);
        setParam (engine, param::filterResonance, 1.0f - on);
        setParam (engine, param::delayTime,       on);
        setParam (engine, param::lfoDepth,        on);
        setParam (engine, param::lfoRate,         on);

        REQUIRE (renderIsFinite (engine, 1, 128, peak));
    }

    REQUIRE (peak > 0.0f);
    REQUIRE (peak < 1000.0f);

    engine.allNotesOff();
    REQUIRE (renderIsFinite (engine, 100, 128, peak));
    REQUIRE (peak < 1000.0f);
}
