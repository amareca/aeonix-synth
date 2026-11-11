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
//  NoteEvent — comando de nota que viaja del hilo de MIDI al hilo de audio.
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
//  ParamEvent — cambio de parametro (id + valor normalizado). Viaja del hilo
//  de UI al hilo de audio. El id va en un buffer de tamano fijo para no
//  asignar memoria en el hilo de audio.
// ============================================================================
struct ParamEvent
{
    char id[32]    = {};
    float normalised = 0.0f;
};

// ============================================================================
//  SynthEngine — gestor de voces (polifonia) y renderizado por bloques.
//
//  Productores (hilo de MIDI/UI):
//      noteOn/noteOff/allNotesOff/setSustain/setParameter encolan eventos en
//      colas lock-free y nunca tocan las voces. Un mutex serializa a los
//      productores (el hilo de audio no lo usa).
//
//  Consumidor (hilo de audio):
//      render() vacia las colas (aplicando eventos y parametros) y suma las
//      voces. Solo este hilo toca las voces.
//
//  RT-safe: render() solo hace aritmetica sobre memoria preasignada.
// ============================================================================
class SynthEngine
{
public:
    // Capacidad maxima de voces reservada por la app. El limite efectivo se
    // puede bajar en caliente con setPolyphony() sin reasignar memoria.
    static constexpr int maxVoiceCapacity = 32;

    SynthEngine() = default;

    void prepare (double sampleRate, int blockSize, int polyphony);

    // Limite de voces simultaneas (8/16/32 en la UI). Seguro desde el hilo de
    // UI: solo actualiza atomics que lee el hilo de audio.
    void setPolyphony (int voices);

    // --- Lado productor (hilo de MIDI/UI). No bloquean. ---
    void noteOn (int midiNote, float velocity);
    void noteOff (int midiNote);
    void allNotesOff();
    void setSustain (bool on);
    void setPitchBend (float value);   // -1..+1, rango fijo +-2 semitonos
    void setModWheel (float value);    // 0..1, suma profundidad al LFO
    void setParameter (const std::string& id, float normalised);

    // --- Lado consumidor (hilo de audio). ---
    void render (float* left, float* right, int numSamples);

    [[nodiscard]] int getActiveVoiceCount() const;
    [[nodiscard]] int getPolyphony() const;

    // Diagnostico/tests: nota que ocupa un slot, o -1 si esta inactivo.
    [[nodiscard]] int getSlotNoteNumber (int slot) const;

private:
    void processPendingEvents();
    void applyNoteEvent (const NoteEvent& e);
    void applyParamEvent (const ParamEvent& e);
    void pushNoteEvent (const NoteEvent& e);
    void pushParamEvent (const ParamEvent& e);
    void applyParameters();
    void applyVoiceParams (Voice& v);

    // Elige la voz a robar: inactiva -> en release -> mas silenciosa -> mas antigua.
    [[nodiscard]] Voice* findVoiceToSteal (int voiceLimit);

    double sampleRate_ = 44100.0;
    std::vector<Voice> voices_;

    // requestedPolyphony_ lo fija la UI; voiceLimit_ es el valor efectivo que
    // usa el hilo de audio (nunca supera la capacidad reservada).
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

    // Punteros a los parametros (estables; resueltos en prepare()). Evitan
    // busquedas por string (y asignaciones) en el hilo de audio.
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

    std::mutex producerMutex_;              // serializa a los productores
    SpScQueue<NoteEvent, 256> noteEvents_;
    SpScQueue<ParamEvent, 256> paramEvents_;
};

} // namespace aeonix
