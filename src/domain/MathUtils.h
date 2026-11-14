#pragma once

#include <algorithm>
#include <cmath>

namespace aeonix
{

// ============================================================================
//  Core math utilities (pure C++, no JUCE).
// ============================================================================

inline constexpr double pi    = 3.14159265358979323846;
inline constexpr double twoPi = 2.0 * pi;

// Converts a MIDI note number (60 = middle C) to frequency in Hz.
inline float midiNoteToFrequency (int midiNote)
{
    return 440.0f * std::pow (2.0f, static_cast<float> (midiNote - 69) / 12.0f);
}

// Tuning ratio of a pitch bend.
//   value: -1..+1 (center = 0), rangeSemitones: total range (e.g. 2).
inline double pitchBendRatio (float value, float rangeSemitones = 2.0f)
{
    value = std::clamp (value, -1.0f, 1.0f);
    return std::pow (2.0, static_cast<double> (value * rangeSemitones) / 12.0);
}

// "balance" style pan gains: center = 1.0/1.0 (no level drop),
// hard left = 1.0/0.0 and hard right = 0.0/1.0.
inline void panGains (float pan, float& left, float& right)
{
    pan = std::clamp (pan, -1.0f, 1.0f);
    left  = (pan <= 0.0f) ? 1.0f : 1.0f - pan;
    right = (pan >= 0.0f) ? 1.0f : 1.0f + pan;
}

// Effective LFO depth: preset base + modulation wheel.
inline float effectiveLfoDepth (float baseDepth, float modWheel)
{
    return std::clamp (baseDepth + modWheel, 0.0f, 1.0f);
}

// Effective cutoff combining the base, the filter envelope and the LFO.
//   envAmount 0..1 -> up to "octaves" octaves of opening with the envelope at 1.
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
