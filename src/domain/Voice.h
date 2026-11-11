#pragma once

#include <cstdint>

#include "Envelope.h"
#include "Filter.h"
#include "MathUtils.h"
#include "Oscillator.h"

namespace aeonix
{

// ============================================================================
//  Voice — una nota activa: oscilador -> filtro -> envolvente de amplitud.
//
//  Combina los bloques de DSP para producir una sola voz. En la Fase 1 es una
//  cadena simple (1 oscilador + filtro + amp ADSR); el 2o oscilador, unison,
//  envolvente de filtro y LFO se anaden en fases posteriores.
// ============================================================================
class Voice
{
public:
    Voice() = default;

    void prepare (double sampleRate);

    void noteOn (int midiNote, float velocity, std::uint64_t noteId, double detuneCents = 0.0);
    void noteOff();         // pasa a release (suelta la nota)
    void holdNote();        // sustain: mantiene la nota sin soltar

    // Suma las muestras de esta voz a los buffers (izq/der).
    void renderAdd (float* left, float* right, int numSamples);

    [[nodiscard]] bool isActive() const;        // hay sonido en curso (no Idle)
    [[nodiscard]] bool isSustainHeld() const;   // retenida por el pedal de sustain
    [[nodiscard]] bool isReleasing() const;     // ya soltada, en cola de release
    [[nodiscard]] float getAmpLevel() const;    // nivel actual de la envolvente de amplitud
    [[nodiscard]] int getNoteNumber() const;
    [[nodiscard]] std::uint64_t getNoteId() const;

    // Ajustes aplicados desde el Patch.
    void setWaveform (Waveform w);
    void setOscLevel (float level);
    void setFilterCutoff (float hz);            // cutoff base (sin modular)
    void setFilterResonance (float q);
    void setAmpEnvelope (float attack, float decay, float sustain, float release);
    void setFilterEnvelope (float attack, float decay, float sustain, float release);

    // Modulaciones por nota.
    void setFilterEnvAmount (float amount);     // 0..1
    void setLfoModulation (float lfoValue);     // bipolar [-1, +1]
    void setPitchBend (float value);            // -1..+1 (rango fijo +-2 semitonos)
    void setPan (float pan);                    // -1 (izq) .. +1 (der)

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

    double baseFrequency_ = 440.0;   // nota MIDI sin detune ni bend
    double detuneRatio_   = 1.0;     // desafinacion de unison (cents -> ratio)
    float  bendValue_     = 0.0f;    // pitch bend actual (-1..+1)

    float baseCutoff_     = 20000.0f;
    float filterEnvAmount_ = 0.0f;
    float lfoMod_         = 0.0f;

    float leftGain_       = 1.0f;
    float rightGain_      = 1.0f;
};

} // namespace aeonix
