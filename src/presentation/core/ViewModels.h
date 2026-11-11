#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <utility>

#include "presentation/core/Observable.h"

namespace aeonix
{

// ============================================================================
//  ViewModels (capa de presentacion).
//
//  Cada ViewModel expone propiedades observables y metodos "set" que:
//    1. actualizan la propiedad (notificando a las Views suscritas), y
//    2. llaman al caso de uso (setParameter) para enviar el cambio al motor.
//
//  Los ViewModels viven en el hilo de UI y NO hacen DSP. No conocen el
//  SynthEngine: reciben un callback (el caso de uso) inyectado.
// ============================================================================

using SetParameterFn = std::function<void (const std::string&, float)>;

inline float clamp01 (float x) { return std::clamp (x, 0.0f, 1.0f); }

// ----------------------------------------------------------------------------
class OscillatorViewModel
{
public:
    explicit OscillatorViewModel (SetParameterFn setParam)
        : setParam_ (std::move (setParam)) {}

    Observable<int>   waveform { 0 };     // 0..3 (Saw, Square, Sine, Triangle)
    Observable<float> level    { 0.5f };  // 0..1 normalizado

    void setWaveform (int index)
    {
        index = std::clamp (index, 0, 3);
        waveform.set (index);
        setParam_ ("osc.waveform", index / 3.0f);   // normalizado 0..1
    }

    void setLevel (float normalised)
    {
        normalised = clamp01 (normalised);
        level.set (normalised);
        setParam_ ("osc.level", normalised);
    }

private:
    SetParameterFn setParam_;
};

// ----------------------------------------------------------------------------
class FilterViewModel
{
public:
    explicit FilterViewModel (SetParameterFn setParam)
        : setParam_ (std::move (setParam)) {}

    Observable<float> cutoff    { 0.8f };  // normalizado (log)
    Observable<float> resonance { 0.1f };  // normalizado

    void setCutoff (float normalised)
    {
        normalised = clamp01 (normalised);
        cutoff.set (normalised);
        setParam_ ("filter.cutoff", normalised);
    }

    void setResonance (float normalised)
    {
        normalised = clamp01 (normalised);
        resonance.set (normalised);
        setParam_ ("filter.resonance", normalised);
    }

private:
    SetParameterFn setParam_;
};

// ----------------------------------------------------------------------------
class EnvelopeViewModel
{
public:
    explicit EnvelopeViewModel (SetParameterFn setParam)
        : setParam_ (std::move (setParam)) {}

    Observable<float> attack  { 0.15f };
    Observable<float> decay   { 0.45f };
    Observable<float> sustain { 0.7f };
    Observable<float> release { 0.5f };

    void setAttack  (float n) { n = clamp01 (n); attack.set (n);  setParam_ ("amp.attack",  n); }
    void setDecay   (float n) { n = clamp01 (n); decay.set (n);   setParam_ ("amp.decay",   n); }
    void setSustain (float n) { n = clamp01 (n); sustain.set (n); setParam_ ("amp.sustain", n); }
    void setRelease (float n) { n = clamp01 (n); release.set (n); setParam_ ("amp.release", n); }

private:
    SetParameterFn setParam_;
};

// ----------------------------------------------------------------------------
class FilterEnvViewModel
{
public:
    explicit FilterEnvViewModel (SetParameterFn setParam)
        : setParam_ (std::move (setParam)) {}

    Observable<float> amount  { 0.0f };   // 0..1
    Observable<float> attack  { 0.2f };   // normalizado (log)
    Observable<float> decay   { 0.67f };  // normalizado (log)
    Observable<float> sustain { 0.2f };
    Observable<float> release { 0.67f };  // normalizado (log)

    void setAmount  (float n) { n = clamp01 (n); amount.set (n);  setParam_ ("filter.envAmount", n); }
    void setAttack  (float n) { n = clamp01 (n); attack.set (n);  setParam_ ("filter.attack",  n); }
    void setDecay   (float n) { n = clamp01 (n); decay.set (n);   setParam_ ("filter.decay",   n); }
    void setSustain (float n) { n = clamp01 (n); sustain.set (n); setParam_ ("filter.sustain", n); }
    void setRelease (float n) { n = clamp01 (n); release.set (n); setParam_ ("filter.release", n); }

private:
    SetParameterFn setParam_;
};

// ----------------------------------------------------------------------------
class SynthViewModel
{
public:
    explicit SynthViewModel (SetParameterFn setParam)
        : setParam_ (std::move (setParam)) {}

    Observable<float> masterVolume { 0.8f };

    void setMasterVolume (float normalised)
    {
        normalised = clamp01 (normalised);
        masterVolume.set (normalised);
        setParam_ ("master.volume", normalised);
    }

private:
    SetParameterFn setParam_;
};

// ----------------------------------------------------------------------------
class FxViewModel
{
public:
    explicit FxViewModel (SetParameterFn setParam)
        : setParam_ (std::move (setParam)) {}

    Observable<float> delayTime     { 0.6f };
    Observable<float> delayFeedback { 0.4f };
    Observable<float> delayMix      { 0.0f };
    Observable<float> reverbSize    { 0.5f };
    Observable<float> reverbMix     { 0.15f };

    void setDelayTime     (float n) { n = clamp01 (n); delayTime.set (n);     setParam_ ("fx.delay.time", n); }
    void setDelayFeedback (float n) { n = clamp01 (n); delayFeedback.set (n); setParam_ ("fx.delay.feedback", n); }
    void setDelayMix      (float n) { n = clamp01 (n); delayMix.set (n);      setParam_ ("fx.delay.mix", n); }
    void setReverbSize    (float n) { n = clamp01 (n); reverbSize.set (n);    setParam_ ("fx.reverb.size", n); }
    void setReverbMix     (float n) { n = clamp01 (n); reverbMix.set (n);     setParam_ ("fx.reverb.mix", n); }

private:
    SetParameterFn setParam_;
};

// ----------------------------------------------------------------------------
class LfoViewModel
{
public:
    explicit LfoViewModel (SetParameterFn setParam) : setParam_ (std::move (setParam)) {}

    Observable<float> rate     { 0.435f };  // normalizado (log)
    Observable<float> depth    { 0.0f };    // 0..1
    Observable<int>   waveform { 2 };       // 0..3 (Saw, Square, Sine, Triangle)

    void setRate (float n)  { n = clamp01 (n); rate.set (n);  setParam_ ("lfo.rate", n); }
    void setDepth (float n) { n = clamp01 (n); depth.set (n); setParam_ ("lfo.depth", n); }

    void setWaveform (int index)
    {
        index = std::clamp (index, 0, 3);
        waveform.set (index);
        setParam_ ("lfo.waveform", index / 3.0f);
    }

private:
    SetParameterFn setParam_;
};

// ----------------------------------------------------------------------------
class UnisonViewModel
{
public:
    explicit UnisonViewModel (SetParameterFn setParam) : setParam_ (std::move (setParam)) {}

    Observable<int>   voices { 1 };      // 1..8
    Observable<float> detune { 0.2f };   // normalizado (rango 0..50 cents)
    Observable<float> width  { 0.5f };   // 0..1 (reparto estereo)

    void setVoices (int n)
    {
        n = std::clamp (n, 1, 8);
        voices.set (n);
        setParam_ ("unison.voices", (n - 1) / 7.0f);
    }

    void setDetune (float n) { n = clamp01 (n); detune.set (n); setParam_ ("unison.detune", n); }
    void setWidth  (float n) { n = clamp01 (n); width.set (n);  setParam_ ("unison.width", n); }

private:
    SetParameterFn setParam_;
};

} // namespace aeonix
