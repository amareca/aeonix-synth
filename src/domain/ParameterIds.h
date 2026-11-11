#pragma once

namespace aeonix::param
{

// ============================================================================
//  Identificadores de los parametros del sintetizador.
//
//  Un solo punto de verdad para los ids, compartido por el nucleo, la GUI y
//  los presets. Aqui esta el subconjunto que usa la Fase 1; la lista completa
//  del MVP (2 osc, unison, LFOs, FX...) se anadira en fases posteriores.
// ============================================================================

inline constexpr const char* masterVolume    = "master.volume";
inline constexpr const char* polyphony       = "master.polyphony";

inline constexpr const char* oscWaveform     = "osc.waveform";
inline constexpr const char* oscLevel        = "osc.level";

inline constexpr const char* filterCutoff    = "filter.cutoff";
inline constexpr const char* filterResonance = "filter.resonance";

// Envolvente de filtro (independiente de la de amplitud).
inline constexpr const char* filterEnvAmount = "filter.envAmount";
inline constexpr const char* filterAttack    = "filter.attack";
inline constexpr const char* filterDecay     = "filter.decay";
inline constexpr const char* filterSustain   = "filter.sustain";
inline constexpr const char* filterRelease   = "filter.release";

inline constexpr const char* ampAttack       = "amp.attack";
inline constexpr const char* ampDecay        = "amp.decay";
inline constexpr const char* ampSustain      = "amp.sustain";
inline constexpr const char* ampRelease      = "amp.release";

inline constexpr const char* delayTime       = "fx.delay.time";
inline constexpr const char* delayFeedback   = "fx.delay.feedback";
inline constexpr const char* delayMix        = "fx.delay.mix";
inline constexpr const char* reverbSize      = "fx.reverb.size";
inline constexpr const char* reverbMix       = "fx.reverb.mix";

inline constexpr const char* lfoRate         = "lfo.rate";
inline constexpr const char* lfoDepth        = "lfo.depth";
inline constexpr const char* lfoWaveform     = "lfo.waveform";
inline constexpr const char* unisonVoices    = "unison.voices";
inline constexpr const char* unisonDetune    = "unison.detune";
inline constexpr const char* unisonWidth     = "unison.width";

} // namespace aeonix::param
