#pragma once

#include <cstdint>

#include "Envelope.h"
#include "Filter.h"
#include "MathUtils.h"
#include "Oscillator.h"

namespace aeonix
{

// ============================================================================
//  Voice - an active note: oscillator -> filter -> amplitude envelope.
//
//  Combines the DSP blocks to produce a single voice. In Phase 1 it is a
//  simple chain (1 oscillator + filter + amp ADSR); the 2nd oscillator, unison,
//  filter envelope and LFO are added in later phases.
// ============================================================================
class Voice
{
public:
    Voice() = default;

    void prepare (double sampleRate);

    void noteOn (int midiNote, float velocity, std::uint64_t noteId, double detuneCents = 0.0);
    void noteOff();         // moves to release (releases the note)
    void holdNote();        // sustain: holds the note without releasing it

    // Adds this voice's samples to the buffers (left/right).
    void renderAdd (float* left, float* right, int numSamples);

    [[nodiscard]] bool isActive() const;        // there is sound in progress (not Idle)
    [[nodiscard]] bool isSustainHeld() const;   // held by the sustain pedal
    [[nodiscard]] bool isReleasing() const;     // already released, in the release tail
    [[nodiscard]] float getAmpLevel() const;    // current amplitude envelope level
    [[nodiscard]] int getNoteNumber() const;
    [[nodiscard]] std::uint64_t getNoteId() const;

    // Settings applied from the Patch.
    void setWaveform (Waveform w);
    void setOscLevel (float level);
    void setFilterCutoff (float hz);            // base cutoff (unmodulated)
    void setFilterResonance (float q);
    void setAmpEnvelope (float attack, float decay, float sustain, float release);
    void setFilterEnvelope (float attack, float decay, float sustain, float release);

    // Per-note modulations.
    void setFilterEnvAmount (float amount);     // 0..1
    void setLfoModulation (float lfoValue);     // bipolar [-1, +1]
    void setPitchBend (float value);            // -1..+1 (fixed +-2 semitone range)
    void setPan (float pan);                    // -1 (left) .. +1 (right)

private:
    void updateFilterCutoff (float envValue);

    double sampleRate_ = 44100.0;

    Oscillator osc_;
    Filter filter_;
    Envelope ampEnv_;
    Envelope filterEnv_;

    int midiNote_        = -1;
    float velocity_      = 0.0f;
    float oscLevel_      = 1.0f;
    std::uint64_t noteId_ = 0;
    bool sustainHeld_    = false;

    double baseFrequency_ = 440.0;   // MIDI note without detune or bend
    double detuneRatio_   = 1.0;     // unison detune (cents -> ratio)
    float  bendValue_     = 0.0f;    // current pitch bend (-1..+1)

    float baseCutoff_     = 20000.0f;
    float filterEnvAmount_ = 0.0f;
    float lfoMod_         = 0.0f;

    float leftGain_       = 1.0f;
    float rightGain_      = 1.0f;
};

} // namespace aeonix
