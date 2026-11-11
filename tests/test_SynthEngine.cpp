#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include "domain/SynthEngine.h"
#include "domain/ParameterIds.h"

using namespace aeonix;

namespace
{
// Renderiza n muestras estereo y devuelve el pico absoluto de salida.
// Tambien sirve para "drenar" los eventos pendientes del motor.
float renderPeak (SynthEngine& engine, int numSamples)
{
    std::vector<float> left (numSamples, 0.0f);
    std::vector<float> right (numSamples, 0.0f);
    engine.render (left.data(), right.data(), numSamples);

    float peak = 0.0f;
    for (const float s : left)
        peak = std::max (peak, std::fabs (s));

    return peak;
}
}

TEST_CASE ("SynthEngine allocates voices on noteOn", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 4);

    REQUIRE (engine.getActiveVoiceCount() == 0);

    engine.noteOn (60, 1.0f);
    renderPeak (engine, 1);                 // aplica el evento
    REQUIRE (engine.getActiveVoiceCount() == 1);

    engine.noteOn (64, 1.0f);
    engine.noteOn (67, 1.0f);
    engine.noteOn (72, 1.0f);
    renderPeak (engine, 1);
    REQUIRE (engine.getActiveVoiceCount() == 4);
}

TEST_CASE ("SynthEngine caps voices at polyphony (voice stealing)", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 4);

    engine.noteOn (60, 1.0f);
    engine.noteOn (64, 1.0f);
    engine.noteOn (67, 1.0f);
    engine.noteOn (72, 1.0f);
    renderPeak (engine, 1);
    REQUIRE (engine.getActiveVoiceCount() == 4);

    engine.noteOn (76, 1.0f);               // roba la voz mas antigua (nota 60)
    renderPeak (engine, 1);
    REQUIRE (engine.getActiveVoiceCount() == 4);
}

TEST_CASE ("SynthEngine noteOff releases the voice", "[engine]")
{
    SynthEngine engine;
    engine.prepare (1000.0, 256, 4);

    engine.noteOn (60, 1.0f);
    renderPeak (engine, 1);
    REQUIRE (engine.getActiveVoiceCount() == 1);

    engine.noteOff (60);
    renderPeak (engine, 400);               // aplica NoteOff y completa el release
    REQUIRE (engine.getActiveVoiceCount() == 0);
}

TEST_CASE ("SynthEngine allNotesOff releases everything", "[engine]")
{
    SynthEngine engine;
    engine.prepare (1000.0, 256, 4);

    engine.noteOn (60, 1.0f);
    engine.noteOn (64, 1.0f);
    engine.noteOn (67, 1.0f);
    renderPeak (engine, 1);
    REQUIRE (engine.getActiveVoiceCount() == 3);

    engine.allNotesOff();
    renderPeak (engine, 400);
    REQUIRE (engine.getActiveVoiceCount() == 0);
}

TEST_CASE ("SynthEngine sustain holds notes until released", "[engine]")
{
    SynthEngine engine;
    engine.prepare (1000.0, 256, 4);

    engine.noteOn (60, 1.0f);
    engine.setSustain (true);
    engine.noteOff (60);
    renderPeak (engine, 1);                 // aplica NoteOn + SustainOn + NoteOff

    // Con el pedal pisado, la nota sigue activa (retenida).
    REQUIRE (engine.getActiveVoiceCount() == 1);

    engine.setSustain (false);
    renderPeak (engine, 400);               // se libera y completa el release
    REQUIRE (engine.getActiveVoiceCount() == 0);
}

TEST_CASE ("SynthEngine renders silence without notes, sound with notes", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 4);

    REQUIRE (renderPeak (engine, 256) == Catch::Approx (0.0f));

    engine.noteOn (60, 1.0f);
    REQUIRE (renderPeak (engine, 256) > 0.0f);
}

TEST_CASE ("SynthEngine setParameter takes effect", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 4);

    engine.noteOn (60, 1.0f);
    REQUIRE (renderPeak (engine, 256) > 0.0f);

    // Baja el volumen a 0: la siguiente renderizacion debe ser silencio.
    engine.setParameter (param::masterVolume, 0.0f);
    REQUIRE (renderPeak (engine, 256) == Catch::Approx (0.0f));
}

TEST_CASE ("SynthEngine unison allocates multiple voices per note", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 8);

    // unison.voices = 4 (rango 1..8, normalizado = (4-1)/7).
    engine.setParameter (param::unisonVoices, 3.0f / 7.0f);
    renderPeak (engine, 1);   // aplica el parametro

    engine.noteOn (60, 1.0f);
    renderPeak (engine, 1);
    REQUIRE (engine.getActiveVoiceCount() == 4);
}

TEST_CASE ("SynthEngine LFO does not break rendering", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 8);

    engine.setParameter (param::lfoRate, 0.5f);
    engine.setParameter (param::lfoDepth, 0.5f);
    engine.noteOn (60, 1.0f);

    REQUIRE (renderPeak (engine, 256) > 0.0f);
}

TEST_CASE ("SynthEngine pitch bend and mod wheel do not break rendering", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 8);

    engine.setPitchBend (0.5f);
    engine.setModWheel (1.0f);
    engine.noteOn (60, 1.0f);

    REQUIRE (renderPeak (engine, 256) > 0.0f);

    // Volver al centro y soltar la rueda sigue siendo seguro.
    engine.setPitchBend (0.0f);
    engine.setModWheel (0.0f);
    REQUIRE (renderPeak (engine, 256) > 0.0f);
}

TEST_CASE ("SynthEngine filter envelope renders", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 4);

    engine.setParameter (param::filterCutoff, 0.3f);
    engine.setParameter (param::filterEnvAmount, 1.0f);
    engine.setParameter (param::filterDecay, 0.2f);
    engine.setParameter (param::filterSustain, 0.0f);
    engine.noteOn (60, 1.0f);

    REQUIRE (renderPeak (engine, 512) > 0.0f);
}

TEST_CASE ("SynthEngine unison width spreads voices across channels", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 8);

    engine.setParameter (param::unisonVoices, 1.0f / 7.0f);   // 2 voces
    engine.setParameter (param::unisonDetune, 1.0f);          // 50 cents
    engine.setParameter (param::unisonWidth, 1.0f);           // pan duro
    engine.noteOn (60, 1.0f);

    const int numSamples = 4096;
    std::vector<float> left (numSamples, 0.0f);
    std::vector<float> right (numSamples, 0.0f);
    engine.render (left.data(), right.data(), numSamples);

    bool channelsDiffer = false;
    for (int i = 0; i < numSamples; ++i)
    {
        if (std::fabs (left[i] - right[i]) > 1e-4f)
        {
            channelsDiffer = true;
            break;
        }
    }

    REQUIRE (channelsDiffer);
}

TEST_CASE ("SynthEngine setPolyphony limits the active voices", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, SynthEngine::maxVoiceCapacity);

    engine.setPolyphony (8);
    REQUIRE (engine.getPolyphony() == 8);

    for (int i = 0; i < 12; ++i)
        engine.noteOn (48 + i * 2, 1.0f);

    renderPeak (engine, 1);
    REQUIRE (engine.getActiveVoiceCount() <= 8);

    engine.setPolyphony (16);
    REQUIRE (engine.getPolyphony() == 16);

    for (int i = 0; i < 12; ++i)
        engine.noteOn (72 + i, 1.0f);

    renderPeak (engine, 1);
    REQUIRE (engine.getActiveVoiceCount() <= 16);
}

TEST_CASE ("SynthEngine steals a releasing voice before sustained ones", "[engine]")
{
    SynthEngine engine;
    engine.prepare (44100.0, 256, 8);
    engine.setPolyphony (8);
    engine.setParameter (param::ampRelease, 1.0f);   // release largo

    for (int i = 0; i < 8; ++i)
        engine.noteOn (48 + i, 1.0f);

    renderPeak (engine, 64);
    REQUIRE (engine.getActiveVoiceCount() == 8);
    REQUIRE (engine.getSlotNoteNumber (0) == 48);

    engine.noteOff (48);
    renderPeak (engine, 64);

    // La voz 0 esta en release: la nota nueva debe reutilizarla en lugar de
    // cortar una voz que sigue sostenida.
    engine.noteOn (72, 1.0f);
    renderPeak (engine, 64);

    REQUIRE (engine.getActiveVoiceCount() == 8);
    REQUIRE (engine.getSlotNoteNumber (0) == 72);
    REQUIRE (engine.getSlotNoteNumber (1) == 49);
}
