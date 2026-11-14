#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "Delay.h"
#include "Lfo.h"
#include "Limiter.h"
#include "Parameter.h"
#include "Patch.h"
#include "Reverb.h"
#include "SpScQueue.h"
#include "Voice.h"

namespace aeonix
{

// ============================================================================
//  NoteEvent - note command that travels from the MIDI thread to the audio thread.
// ============================================================================
struct NoteEvent
{
    enum class Type
    {
        NoteOn,
        NoteOff,
        AllNotesOff,
        SustainOn,
        SustainOff,
        PitchBend,
        ModWheel
    };

    Type type        = Type::NoteOff;
    int noteNumber   = 0;
    float velocity   = 0.0f;
    float value      = 0.0f;   // PitchBend: -1..+1 | ModWheel: 0..1
};

// ============================================================================
//  ParamEvent - parameter change (id + normalized value). Travels from the UI
//  thread to the audio thread. The id goes in a fixed-size buffer so as not to
//  allocate memory on the audio thread.
// ============================================================================
struct ParamEvent
{
    char id[32]    = {};
    float normalised = 0.0f;
};

// ============================================================================
//  SynthEngine - voice manager (polyphony) and block rendering.
//
//  Producers (MIDI/UI thread):
//      noteOn/noteOff/allNotesOff/setSustain/setParameter enqueue events into
//      lock-free queues and never touch the voices. A mutex serializes the
//      producers (the audio thread does not use it).
//
//  Consumer (audio thread):
//      render() drains the queues (applying events and parameters) and sums the
//      voices. Only this thread touches the voices.
//
//  RT-safe: render() only does arithmetic on preallocated memory.
// ============================================================================
class SynthEngine
{
public:
    // Maximum voice capacity reserved by the app. The effective limit can be
    // lowered at runtime with setPolyphony() without reallocating memory.
    static constexpr int maxVoiceCapacity = 32;

    SynthEngine() = default;

    void prepare (double sampleRate, int blockSize, int polyphony);

    // Simultaneous voice limit (8/16/32 in the UI). Safe from the UI
    // thread: only updates atomics that the audio thread reads.
    void setPolyphony (int voices);

    // --- Producer side (MIDI/UI thread). They do not block. ---
    void noteOn (int midiNote, float velocity);
    void noteOff (int midiNote);
    void allNotesOff();
    void setSustain (bool on);
    void setPitchBend (float value);   // -1..+1, fixed +-2 semitone range
    void setModWheel (float value);    // 0..1, adds depth to the LFO
    void setParameter (const std::string& id, float normalised);

    // --- Consumer side (audio thread). ---
    void render (float* left, float* right, int numSamples);

    [[nodiscard]] int getActiveVoiceCount() const;
    [[nodiscard]] int getPolyphony() const;

    // Diagnostics/tests: note occupying a slot, or -1 if inactive.
    [[nodiscard]] int getSlotNoteNumber (int slot) const;

private:
    void processPendingEvents();
    void applyNoteEvent (const NoteEvent& e);
    void applyParamEvent (const ParamEvent& e);
    void pushNoteEvent (const NoteEvent& e);
    void pushParamEvent (const ParamEvent& e);
    void applyParameters();
    void applyVoiceParams (Voice& v);

    // Chooses the voice to steal: inactive -> in release -> quietest -> oldest.
    [[nodiscard]] Voice* findVoiceToSteal (int voiceLimit);

    double sampleRate_ = 44100.0;
    std::vector<Voice> voices_;

    // requestedPolyphony_ is set by the UI; voiceLimit_ is the effective value
    // used by the audio thread (never exceeds the reserved capacity).
    std::atomic<int> requestedPolyphony_ { 16 };
    std::atomic<int> voiceLimit_ { 1 };
    std::atomic<int> voiceCapacity_ { 1 };

    Patch patch_     = makeDefaultPatch();
    float masterGain_ = 0.8f;
    bool paramsDirty_ = false;

    Delay delay_;
    Reverb reverb_;
    Lfo lfo_;
    Limiter limiter_;

    float lfoDepth_          = 0.0f;
    int unisonVoices_        = 1;
    float unisonDetuneCents_ = 0.0f;
    float unisonWidth_       = 0.5f;
    float pitchBend_         = 0.0f;
    float modWheel_          = 0.0f;

    // Pointers to the parameters (stable; resolved in prepare()). They avoid
    // string lookups (and allocations) on the audio thread.
    Parameter* pMasterVolume_ = nullptr;
    Parameter* pWaveform_     = nullptr;
    Parameter* pOscLevel_     = nullptr;
    Parameter* pCutoff_       = nullptr;
    Parameter* pResonance_    = nullptr;
    Parameter* pFilterEnvAmount_ = nullptr;
    Parameter* pFilterAttack_ = nullptr;
    Parameter* pFilterDecay_  = nullptr;
    Parameter* pFilterSustain_ = nullptr;
    Parameter* pFilterRelease_ = nullptr;
    Parameter* pAttack_       = nullptr;
    Parameter* pDecay_        = nullptr;
    Parameter* pSustain_      = nullptr;
    Parameter* pRelease_      = nullptr;
    Parameter* pDelayTime_    = nullptr;
    Parameter* pDelayFeedback_ = nullptr;
    Parameter* pDelayMix_     = nullptr;
    Parameter* pReverbSize_   = nullptr;
    Parameter* pReverbMix_    = nullptr;
    Parameter* pLfoRate_      = nullptr;
    Parameter* pLfoDepth_     = nullptr;
    Parameter* pLfoWaveform_  = nullptr;
    Parameter* pUnisonVoices_ = nullptr;
    Parameter* pUnisonDetune_ = nullptr;
    Parameter* pUnisonWidth_  = nullptr;

    bool sustain_           = false;
    std::uint64_t nextNoteId_ = 1;

    std::mutex producerMutex_;              // serializes the producers
    SpScQueue<NoteEvent, 256> noteEvents_;
    SpScQueue<ParamEvent, 256> paramEvents_;
};

} // namespace aeonix
