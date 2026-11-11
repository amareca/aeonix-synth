#include "Voice.h"

namespace aeonix
{

void Voice::prepare (double sampleRate)
{
    sampleRate_ = sampleRate;
    osc_.setSampleRate (sampleRate);
    filter_.prepare (sampleRate);
    ampEnv_.prepare (sampleRate);
    filterEnv_.prepare (sampleRate);
}

void Voice::noteOn (int midiNote, float velocity, std::uint64_t noteId, double detuneCents)
{
    midiNote_    = midiNote;
    velocity_    = velocity;
    noteId_      = noteId;
    sustainHeld_ = false;

    baseFrequency_ = midiNoteToFrequency (midiNote);
    detuneRatio_   = std::pow (2.0, detuneCents / 1200.0);
    bendValue_     = 0.0f;

    osc_.setFrequency (baseFrequency_ * detuneRatio_);
    ampEnv_.noteOn();
    filterEnv_.noteOn();
}

void Voice::noteOff()
{
    sustainHeld_ = false;
    ampEnv_.noteOff();
    filterEnv_.noteOff();
}

void Voice::holdNote()
{
    sustainHeld_ = true;
}

void Voice::renderAdd (float* left, float* right, int numSamples)
{
    // El cutoff se recalcula cada 16 muestras (control rate interno): la
    // envolvente se mueve con suficiente resolucion y sin recalcular el
    // biquad en cada muestra.
    int cutoffCounter = 0;

    for (int i = 0; i < numSamples; ++i)
    {
        if (!ampEnv_.isActive())
            break;   // voz terminada: no aporta mas audio

        const float env      = ampEnv_.nextSample();
        const float filterEnv = filterEnv_.nextSample();

        if (--cutoffCounter <= 0)
        {
            updateFilterCutoff (filterEnv);
            cutoffCounter = 16;
        }

        const float osc      = osc_.nextSample();
        const float filtered = filter_.process (osc);
        const float out      = filtered * env * velocity_ * oscLevel_;

        left[i]  += out * leftGain_;
        right[i] += out * rightGain_;
    }
}

bool Voice::isActive() const                 { return ampEnv_.isActive(); }
bool Voice::isSustainHeld() const            { return sustainHeld_; }
bool Voice::isReleasing() const              { return ampEnv_.getStage() == Envelope::Stage::Release; }
float Voice::getAmpLevel() const             { return static_cast<float> (ampEnv_.getValue()); }
int Voice::getNoteNumber() const             { return midiNote_; }
std::uint64_t Voice::getNoteId() const       { return noteId_; }

void Voice::setWaveform (Waveform w)         { osc_.setWaveform (w); }
void Voice::setOscLevel (float level)        { oscLevel_ = level; }
void Voice::setFilterResonance (float q)     { filter_.setResonance (q); }

void Voice::setFilterCutoff (float hz)
{
    baseCutoff_ = hz;
    updateFilterCutoff (filterEnv_.getValue());
}

void Voice::setAmpEnvelope (float attack, float decay, float sustain, float release)
{
    ampEnv_.setAttack (attack);
    ampEnv_.setDecay (decay);
    ampEnv_.setSustain (sustain);
    ampEnv_.setRelease (release);
}

void Voice::setFilterEnvelope (float attack, float decay, float sustain, float release)
{
    filterEnv_.setAttack (attack);
    filterEnv_.setDecay (decay);
    filterEnv_.setSustain (sustain);
    filterEnv_.setRelease (release);
}

void Voice::setFilterEnvAmount (float amount)
{
    filterEnvAmount_ = std::clamp (amount, 0.0f, 1.0f);
}

void Voice::setLfoModulation (float lfoValue)
{
    lfoMod_ = std::clamp (lfoValue, -1.0f, 1.0f);
}

void Voice::setPitchBend (float value)
{
    bendValue_ = std::clamp (value, -1.0f, 1.0f);
    osc_.setFrequency (baseFrequency_ * detuneRatio_ * pitchBendRatio (bendValue_));
}

void Voice::setPan (float pan)
{
    panGains (pan, leftGain_, rightGain_);
}

void Voice::updateFilterCutoff (float envValue)
{
    filter_.setCutoff (effectiveCutoff (baseCutoff_, envValue, filterEnvAmount_, lfoMod_));
}

} // namespace aeonix
