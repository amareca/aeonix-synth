#include "Preset.h"

#include <cmath>

#include "ParameterIds.h"
#include "Patch.h"

namespace aeonix
{

std::map<std::string, float> makeDefaultParameters()
{
    const auto patch = makeDefaultPatch();

    std::map<std::string, float> m;
    for (const auto& p : patch.parameters())
        m[p.getId()] = p.getNormalised();

    return m;
}

namespace
{

// Frecuencia del LFO (Hz) -> valor normalizado (rango log 0.05..10 Hz).
float lfoRateNormalised (float hz)
{
    return static_cast<float> (std::log (hz / 0.05f) / std::log (200.0f));
}

// Construye un preset de pad a partir de sus rasgos principales, con
// unison (anchura) y LFO (movimiento del cutoff).
Preset makePadPreset (const std::string& name,
                      float cutoff,
                      float attack,
                      float release,
                      float reverbMix,
                      float reverbSize,
                      float delayMix,
                      int unisonVoices,
                      float detuneCents,
                      float lfoRateHz,
                      float lfoDepth,
                      float waveformIndex = 0.0f)
{
    auto p = makeDefaultParameters();
    p["osc.waveform"]      = waveformIndex / 3.0f;   // 0=sierra, 1=cuadrada...
    p["amp.attack"]        = attack;
    p["amp.decay"]         = 0.6f;
    p["amp.sustain"]       = 0.8f;
    p["amp.release"]       = release;
    p["filter.cutoff"]     = cutoff;
    p["filter.resonance"]  = 0.12f;
    p["fx.reverb.mix"]     = reverbMix;
    p["fx.reverb.size"]    = reverbSize;
    p["fx.delay.mix"]      = delayMix;

    // Envolvente de filtro suave: un poco de movimiento al atacar el pad.
    p["filter.envAmount"]  = 0.15f;
    p["filter.attack"]     = 0.35f;
    p["filter.decay"]      = 0.7f;
    p["filter.sustain"]    = 0.5f;
    p["filter.release"]    = 0.7f;

    // Unison (anchura): n voces + separacion en cents + reparto estereo.
    p["unison.voices"]     = (unisonVoices - 1) / 7.0f;
    p["unison.detune"]     = detuneCents / 50.0f;
    p["unison.width"]      = 0.85f;

    // LFO (movimiento del cutoff), forma seno.
    p["lfo.waveform"]      = 2.0f / 3.0f;
    p["lfo.rate"]          = lfoRateNormalised (lfoRateHz);
    p["lfo.depth"]         = lfoDepth;

    return { name, p };
}

} // namespace

std::vector<Preset> makeFactoryPresets()
{
    std::vector<Preset> presets;

    presets.push_back ({ "Default", makeDefaultParameters() });

    // Pluck: ataque rapido, caida corta, brillante. Unison suave para grosor.
    auto pluck = makeDefaultParameters();
    pluck["amp.attack"]       = 0.02f;
    pluck["amp.decay"]        = 0.35f;
    pluck["amp.sustain"]      = 0.15f;
    pluck["amp.release"]      = 0.25f;
    pluck["filter.cutoff"]    = 0.72f;
    pluck["filter.resonance"] = 0.25f;
    pluck["filter.envAmount"] = 0.65f;          // "wah" de pluck
    pluck["filter.attack"]    = 0.1f;
    pluck["filter.decay"]     = 0.45f;
    pluck["filter.sustain"]   = 0.0f;
    pluck["filter.release"]   = 0.4f;
    pluck["unison.voices"]    = 1.0f / 7.0f;    // 2 voces
    pluck["unison.detune"]    = 8.0f / 50.0f;   // 8 cents
    pluck["unison.width"]     = 0.25f;
    presets.push_back ({ "Pluck", pluck });

    // Acid Bass: estilo TB-303 (squelch). Sin unison/LFO (acido "apretado").
    auto acid = makeDefaultParameters();
    acid["osc.waveform"]      = 1.0f / 3.0f;   // Square
    acid["osc.level"]         = 0.6f;
    acid["filter.cutoff"]     = 0.35f;
    acid["filter.resonance"]  = 0.82f;
    acid["filter.envAmount"]  = 0.75f;          // squelch acido
    acid["filter.attack"]     = 0.08f;
    acid["filter.decay"]      = 0.35f;
    acid["filter.sustain"]    = 0.05f;
    acid["filter.release"]    = 0.3f;
    acid["unison.width"]      = 0.0f;           // mono y apretado
    acid["amp.attack"]        = 0.02f;
    acid["amp.decay"]         = 0.6f;
    acid["amp.sustain"]       = 0.15f;
    acid["amp.release"]       = 0.35f;
    acid["fx.delay.time"]     = 0.4f;
    acid["fx.delay.feedback"] = 0.45f;
    acid["fx.delay.mix"]      = 0.18f;
    acid["fx.reverb.mix"]     = 0.1f;
    presets.push_back ({ "Acid Bass", acid });

    // Pads cinematograficos/espaciales (con unison + LFO).
    presets.push_back (makePadPreset ("Warm Pad",   0.32f, 0.72f, 0.70f, 0.45f, 0.60f, 0.00f, 2,  8.0f, 0.25f, 0.20f));
    presets.push_back (makePadPreset ("Lush Pad",   0.50f, 0.76f, 0.75f, 0.50f, 0.65f, 0.12f, 3, 12.0f, 0.35f, 0.30f));
    presets.push_back (makePadPreset ("Dark Pad",   0.20f, 0.82f, 0.80f, 0.40f, 0.70f, 0.00f, 2,  6.0f, 0.20f, 0.25f));
    presets.push_back (makePadPreset ("Space Pad",  0.35f, 0.80f, 0.85f, 0.65f, 0.75f, 0.25f, 4, 18.0f, 0.40f, 0.40f));
    presets.push_back (makePadPreset ("Bright Pad", 0.60f, 0.70f, 0.65f, 0.40f, 0.55f, 0.00f, 3, 10.0f, 0.30f, 0.25f, 1.0f));

    // ---- Leads: ataque rapido, sustain medio y presencia en el corte. ----

    // Lead Saw: sierra clasica con unison suave y delay corto.
    auto leadSaw = makeDefaultParameters();
    leadSaw["osc.waveform"]      = 0.0f;            // Saw
    leadSaw["osc.level"]         = 0.55f;
    leadSaw["filter.cutoff"]     = 0.78f;           // ~4 kHz
    leadSaw["filter.resonance"]  = 0.30f;
    leadSaw["filter.envAmount"]  = 0.45f;
    leadSaw["filter.attack"]     = 0.06f;
    leadSaw["filter.decay"]      = 0.38f;
    leadSaw["filter.sustain"]    = 0.30f;
    leadSaw["filter.release"]    = 0.40f;
    leadSaw["amp.attack"]        = 0.10f;           // ~2 ms
    leadSaw["amp.decay"]         = 0.40f;
    leadSaw["amp.sustain"]       = 0.75f;
    leadSaw["amp.release"]       = 0.50f;
    leadSaw["unison.voices"]     = 1.0f / 7.0f;     // 2 voces
    leadSaw["unison.detune"]     = 7.0f / 50.0f;    // 7 cents
    leadSaw["unison.width"]      = 0.25f;
    leadSaw["fx.delay.time"]     = 0.50f;           // ~220 ms
    leadSaw["fx.delay.feedback"] = 0.35f;
    leadSaw["fx.delay.mix"]      = 0.18f;
    leadSaw["fx.reverb.size"]    = 0.45f;
    leadSaw["fx.reverb.mix"]     = 0.12f;
    leadSaw["lfo.rate"]          = lfoRateNormalised (5.0f);
    leadSaw["lfo.depth"]         = 0.06f;
    presets.push_back ({ "Lead Saw", leadSaw });

    // Lead Square: cuadrada hueca, mas resonancia y delay mas largo.
    auto leadSquare = makeDefaultParameters();
    leadSquare["osc.waveform"]      = 1.0f / 3.0f;  // Square
    leadSquare["osc.level"]         = 0.45f;
    leadSquare["filter.cutoff"]     = 0.72f;        // ~2,9 kHz
    leadSquare["filter.resonance"]  = 0.45f;
    leadSquare["filter.envAmount"]  = 0.55f;
    leadSquare["filter.attack"]     = 0.05f;
    leadSquare["filter.decay"]      = 0.30f;
    leadSquare["filter.sustain"]    = 0.25f;
    leadSquare["filter.release"]    = 0.35f;
    leadSquare["amp.attack"]        = 0.08f;
    leadSquare["amp.decay"]         = 0.35f;
    leadSquare["amp.sustain"]       = 0.65f;
    leadSquare["amp.release"]       = 0.45f;
    leadSquare["unison.voices"]     = 1.0f / 7.0f;  // 2 voces
    leadSquare["unison.detune"]     = 5.0f / 50.0f; // 5 cents
    leadSquare["unison.width"]      = 0.15f;
    leadSquare["fx.delay.time"]     = 0.55f;        // ~260 ms
    leadSquare["fx.delay.feedback"] = 0.42f;
    leadSquare["fx.delay.mix"]      = 0.22f;
    leadSquare["fx.reverb.size"]    = 0.40f;
    leadSquare["fx.reverb.mix"]     = 0.10f;
    leadSquare["lfo.rate"]          = lfoRateNormalised (6.0f);
    leadSquare["lfo.depth"]         = 0.05f;
    presets.push_back ({ "Lead Square", leadSquare });

    // Lead Soft: triangulo, mas unison y reverb para un lead cantable.
    auto leadSoft = makeDefaultParameters();
    leadSoft["osc.waveform"]      = 1.0f;           // Triangle
    leadSoft["osc.level"]         = 0.60f;
    leadSoft["filter.cutoff"]     = 0.65f;          // ~1,8 kHz
    leadSoft["filter.resonance"]  = 0.15f;
    leadSoft["filter.envAmount"]  = 0.20f;
    leadSoft["filter.attack"]     = 0.30f;
    leadSoft["filter.decay"]      = 0.55f;
    leadSoft["filter.sustain"]    = 0.55f;
    leadSoft["filter.release"]    = 0.55f;
    leadSoft["amp.attack"]        = 0.22f;          // ~7 ms
    leadSoft["amp.decay"]         = 0.50f;
    leadSoft["amp.sustain"]       = 0.85f;
    leadSoft["amp.release"]       = 0.60f;
    leadSoft["unison.voices"]     = 2.0f / 7.0f;    // 3 voces
    leadSoft["unison.detune"]     = 6.0f / 50.0f;   // 6 cents
    leadSoft["unison.width"]      = 0.45f;
    leadSoft["fx.delay.time"]     = 0.60f;          // ~300 ms
    leadSoft["fx.delay.feedback"] = 0.45f;
    leadSoft["fx.delay.mix"]      = 0.28f;
    leadSoft["fx.reverb.size"]    = 0.55f;
    leadSoft["fx.reverb.mix"]     = 0.25f;
    leadSoft["lfo.rate"]          = lfoRateNormalised (4.5f);
    leadSoft["lfo.depth"]         = 0.12f;
    presets.push_back ({ "Lead Soft", leadSoft });

    // ---- Strings: ataque lento, sustain alto y ensemble ancho. ----

    // String Ensemble: sierra con unison amplio, el "string machine" clasico.
    auto stringEnsemble = makeDefaultParameters();
    stringEnsemble["osc.waveform"]      = 0.0f;            // Saw
    stringEnsemble["osc.level"]         = 0.50f;
    stringEnsemble["filter.cutoff"]     = 0.62f;           // ~1,5 kHz
    stringEnsemble["filter.resonance"]  = 0.10f;
    stringEnsemble["filter.envAmount"]  = 0.15f;
    stringEnsemble["filter.attack"]     = 0.50f;
    stringEnsemble["filter.decay"]      = 0.65f;
    stringEnsemble["filter.sustain"]    = 0.65f;
    stringEnsemble["filter.release"]    = 0.70f;
    stringEnsemble["amp.attack"]        = 0.68f;           // ~300 ms
    stringEnsemble["amp.decay"]         = 0.60f;
    stringEnsemble["amp.sustain"]       = 0.90f;
    stringEnsemble["amp.release"]       = 0.70f;
    stringEnsemble["unison.voices"]     = 4.0f / 7.0f;     // 5 voces
    stringEnsemble["unison.detune"]     = 14.0f / 50.0f;   // 14 cents
    stringEnsemble["unison.width"]      = 0.90f;
    stringEnsemble["fx.delay.time"]     = 0.55f;
    stringEnsemble["fx.delay.feedback"] = 0.30f;
    stringEnsemble["fx.delay.mix"]      = 0.08f;
    stringEnsemble["fx.reverb.size"]    = 0.65f;
    stringEnsemble["fx.reverb.mix"]     = 0.40f;
    stringEnsemble["lfo.rate"]          = lfoRateNormalised (0.6f);
    stringEnsemble["lfo.depth"]         = 0.12f;
    presets.push_back ({ "String Ensemble", stringEnsemble });

    // String Soft: triangulo y corte bajo, para colchones suaves.
    auto stringSoft = makeDefaultParameters();
    stringSoft["osc.waveform"]      = 1.0f;           // Triangle
    stringSoft["osc.level"]         = 0.55f;
    stringSoft["filter.cutoff"]     = 0.52f;          // ~800 Hz
    stringSoft["filter.resonance"]  = 0.08f;
    stringSoft["filter.envAmount"]  = 0.10f;
    stringSoft["filter.attack"]     = 0.55f;
    stringSoft["filter.decay"]      = 0.70f;
    stringSoft["filter.sustain"]    = 0.70f;
    stringSoft["filter.release"]    = 0.72f;
    stringSoft["amp.attack"]        = 0.72f;          // ~420 ms
    stringSoft["amp.decay"]         = 0.65f;
    stringSoft["amp.sustain"]       = 0.95f;
    stringSoft["amp.release"]       = 0.72f;
    stringSoft["unison.voices"]     = 3.0f / 7.0f;    // 4 voces
    stringSoft["unison.detune"]     = 10.0f / 50.0f;  // 10 cents
    stringSoft["unison.width"]      = 0.80f;
    stringSoft["fx.delay.time"]     = 0.50f;
    stringSoft["fx.delay.feedback"] = 0.25f;
    stringSoft["fx.delay.mix"]      = 0.05f;
    stringSoft["fx.reverb.size"]    = 0.70f;
    stringSoft["fx.reverb.mix"]     = 0.45f;
    stringSoft["lfo.rate"]          = lfoRateNormalised (0.35f);
    stringSoft["lfo.depth"]         = 0.08f;
    presets.push_back ({ "String Soft", stringSoft });

    // String Bright: mas corte, mas voces de unison y movimiento algo mayor.
    auto stringBright = makeDefaultParameters();
    stringBright["osc.waveform"]      = 0.0f;            // Saw
    stringBright["osc.level"]         = 0.50f;
    stringBright["filter.cutoff"]     = 0.72f;           // ~2,9 kHz
    stringBright["filter.resonance"]  = 0.12f;
    stringBright["filter.envAmount"]  = 0.25f;
    stringBright["filter.attack"]     = 0.45f;
    stringBright["filter.decay"]      = 0.60f;
    stringBright["filter.sustain"]    = 0.60f;
    stringBright["filter.release"]    = 0.65f;
    stringBright["amp.attack"]        = 0.65f;           // ~240 ms
    stringBright["amp.decay"]         = 0.55f;
    stringBright["amp.sustain"]       = 0.90f;
    stringBright["amp.release"]       = 0.68f;
    stringBright["unison.voices"]     = 5.0f / 7.0f;     // 6 voces
    stringBright["unison.detune"]     = 16.0f / 50.0f;   // 16 cents
    stringBright["unison.width"]      = 0.95f;
    stringBright["fx.delay.time"]     = 0.58f;
    stringBright["fx.delay.feedback"] = 0.35f;
    stringBright["fx.delay.mix"]      = 0.12f;
    stringBright["fx.reverb.size"]    = 0.60f;
    stringBright["fx.reverb.mix"]     = 0.35f;
    stringBright["lfo.rate"]          = lfoRateNormalised (0.8f);
    stringBright["lfo.depth"]         = 0.15f;
    presets.push_back ({ "String Bright", stringBright });

    return presets;
}

} // namespace aeonix
