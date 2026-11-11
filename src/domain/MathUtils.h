#pragma once

#include <algorithm>
#include <cmath>

namespace aeonix
{

// ============================================================================
//  Utilidades matematicas del nucleo (C++ puro, sin JUCE).
// ============================================================================

inline constexpr double pi    = 3.14159265358979323846;
inline constexpr double twoPi = 2.0 * pi;

// Convierte un numero de nota MIDI (60 = Do central) a frecuencia en Hz.
inline float midiNoteToFrequency (int midiNote)
{
    return 440.0f * std::pow (2.0f, static_cast<float> (midiNote - 69) / 12.0f);
}

// Relacion de afinacion de un pitch bend.
//   value: -1..+1 (centro = 0), rangeSemitones: rango total (p.ej. 2).
inline double pitchBendRatio (float value, float rangeSemitones = 2.0f)
{
    value = std::clamp (value, -1.0f, 1.0f);
    return std::pow (2.0, static_cast<double> (value * rangeSemitones) / 12.0);
}

// Ganancias de pan tipo "balance": centro = 1.0/1.0 (sin caida de nivel),
// extremo izquierdo = 1.0/0.0 y extremo derecho = 0.0/1.0.
inline void panGains (float pan, float& left, float& right)
{
    pan = std::clamp (pan, -1.0f, 1.0f);
    left  = (pan <= 0.0f) ? 1.0f : 1.0f - pan;
    right = (pan >= 0.0f) ? 1.0f : 1.0f + pan;
}

// Profundidad efectiva del LFO: la base del preset + la rueda de modulacion.
inline float effectiveLfoDepth (float baseDepth, float modWheel)
{
    return std::clamp (baseDepth + modWheel, 0.0f, 1.0f);
}

// Cutoff efectivo combinando la base, la envolvente de filtro y el LFO.
//   envAmount 0..1 -> hasta "octaves" octavas de apertura con la envolvente a 1.
inline float effectiveCutoff (float baseHz,
                              float envValue,
                              float envAmount,
                              float lfoValue,
                              float octaves = 6.0f)
{
    const float envOctaves = envAmount * std::clamp (envValue, 0.0f, 1.0f) * octaves;
    const float cutoff = baseHz * std::pow (2.0f, envOctaves) * (1.0f + lfoValue);
    return std::clamp (cutoff, 20.0f, 20000.0f);
}

} // namespace aeonix
