#include "Patch.h"

#include "ParameterIds.h"

namespace aeonix
{

// ============================================================================
//  Default Patch for Phase 1.
//
//  Approximate ranges; fine mapping (for example logarithmic cutoff) will
//  be refined when the GUI is designed. For now we use simple linear ranges.
// ============================================================================
Patch makeDefaultPatch()
{
    Patch patch;

    // Master.
    patch.add (Parameter (param::masterVolume, "Master Volume", 0.0f, 1.0f, 0.8f));

    // Oscillator (0 = saw, 1 = square, 2 = sine, 3 = triangle).
    patch.add (Parameter (param::oscWaveform, "Waveform", 0.0f, 3.0f, 0.0f));
    patch.add (Parameter (param::oscLevel,   "Osc Level",  0.0f, 1.0f, 0.5f));

    // Filter (low-pass). Logarithmic cutoff (Hz).
    patch.add (Parameter (param::filterCutoff,    "Filter Cutoff",    20.0f, 20000.0f, 0.8f, true));
    patch.add (Parameter (param::filterResonance, "Filter Resonance", 0.0f,  1.0f,    0.1f));

    // Filter envelope (opens the cutoff on note attack).
    patch.add (Parameter (param::filterEnvAmount, "Filter Env Amount", 0.0f, 1.0f, 0.0f));
    patch.add (Parameter (param::filterAttack,    "Filter Attack",  0.001f, 5.0f, 0.2f, true));
    patch.add (Parameter (param::filterDecay,     "Filter Decay",   0.001f, 5.0f, 0.67f, true));
    patch.add (Parameter (param::filterSustain,   "Filter Sustain", 0.0f,   1.0f, 0.2f));
    patch.add (Parameter (param::filterRelease,   "Filter Release", 0.001f, 5.0f, 0.67f, true));

    // Amplitude envelope (ADSR). Logarithmic times (seconds).
    patch.add (Parameter (param::ampAttack,  "Amp Attack",  0.001f, 5.0f, 0.15f, true));
    patch.add (Parameter (param::ampDecay,   "Amp Decay",   0.001f, 5.0f, 0.45f, true));
    patch.add (Parameter (param::ampSustain, "Amp Sustain", 0.0f,   1.0f, 0.7f));
    patch.add (Parameter (param::ampRelease, "Amp Release", 0.001f, 5.0f, 0.5f,  true));

    // Effects: delay + reverb.
    patch.add (Parameter (param::delayTime,     "Delay Time",     0.05f, 1.0f, 0.6f, true));
    patch.add (Parameter (param::delayFeedback, "Delay Feedback", 0.0f,  0.9f, 0.4f));
    patch.add (Parameter (param::delayMix,      "Delay Mix",      0.0f,  1.0f, 0.0f));
    patch.add (Parameter (param::reverbSize,    "Reverb Size",    0.0f,  1.0f, 0.5f));
    patch.add (Parameter (param::reverbMix,     "Reverb Mix",     0.0f,  1.0f, 0.15f));

    // LFO (slow modulation, e.g. of the cutoff).
    patch.add (Parameter (param::lfoRate,     "LFO Rate",     0.05f, 10.0f, 0.435f, true));
    patch.add (Parameter (param::lfoDepth,    "LFO Depth",    0.0f,   1.0f, 0.0f));
    patch.add (Parameter (param::lfoWaveform, "LFO Waveform", 0.0f,   3.0f, 2.0f / 3.0f));  // 2 = sine

    // Unison: several slightly detuned voices per note.
    patch.add (Parameter (param::unisonVoices, "Unison Voices", 1.0f, 8.0f, 0.0f));
    patch.add (Parameter (param::unisonDetune, "Unison Detune", 0.0f, 50.0f, 0.2f));  // 10 cents
    patch.add (Parameter (param::unisonWidth,  "Unison Width",  0.0f, 1.0f,  0.5f));   // pan

    return patch;
}

} // namespace aeonix
