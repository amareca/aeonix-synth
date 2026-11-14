#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "ParameterIds.h"
#include "Patch.h"

namespace aeonix
{

// ============================================================================
//  ParameterFormat - converts a normalized value (0..1) to the real
//  parameter value and formats it with readable units (Hz, ms/s, cents, %).
//
//  Reuses makeDefaultPatch() as the single source of ranges and
//  logarithmic mapping, so that information is not duplicated in the GUI.
//  Pure C++: can be tested without JUCE.
// ============================================================================

namespace detail
{

inline const Patch& displayPatch()
{
    static const Patch patch = makeDefaultPatch();
    return patch;
}

// Formats with "decimals" decimals and trims trailing zeros ("20.00" -> "20").
inline std::string formatNumber (double value, int decimals)
{
    char buffer[64] = {};
    std::snprintf (buffer, sizeof (buffer), "%.*f", decimals, value);

    std::string s (buffer);
    if (const auto dot = s.find ('.'); dot != std::string::npos)
    {
        while (! s.empty() && s.back() == '0')
            s.pop_back();

        if (! s.empty() && s.back() == '.')
            s.pop_back();
    }

    return s;
}

inline bool isTimeParameter (const std::string& id)
{
    return id == param::ampAttack   || id == param::ampDecay   || id == param::ampRelease
        || id == param::filterAttack || id == param::filterDecay || id == param::filterRelease
        || id == param::delayTime;
}

} // namespace detail

inline std::string formatParameterValue (const std::string& id, float normalised)
{
    const Parameter* definition = detail::displayPatch().find (id);
    if (definition == nullptr)
        return {};

    // Copies the definition and applies the received normalized value: getValue()
    // uses the value stored in the Parameter, not the argument.
    Parameter p = *definition;
    p.setNormalised (normalised);
    const float value = p.getValue();

    // Waveforms: index -> name.
    if (id == param::oscWaveform || id == param::lfoWaveform)
    {
        static const char* const names[] = { "Saw", "Square", "Sine", "Triangle" };
        const int index = std::clamp (static_cast<int> (std::lround (value)), 0, 3);
        return names[index];
    }

    // Frequencies.
    if (id == param::filterCutoff || id == param::lfoRate)
    {
        if (value >= 1000.0f)
            return detail::formatNumber (value / 1000.0f, 1) + " kHz";

        if (value >= 100.0f)
            return detail::formatNumber (value, 0) + " Hz";

        return detail::formatNumber (value, 1) + " Hz";
    }

    // Times: ms below 1 s, seconds above.
    if (detail::isTimeParameter (id))
    {
        if (value < 1.0f)
        {
            const float ms = value * 1000.0f;
            return detail::formatNumber (ms, ms < 10.0f ? 1 : 0) + " ms";
        }

        return detail::formatNumber (value, 2) + " s";
    }

    // Unison detune (cents).
    if (id == param::unisonDetune)
        return detail::formatNumber (value, 1) + " ct";

    // Unison voice count: integer with no unit.
    if (id == param::unisonVoices)
        return detail::formatNumber (value, 0);

    // Everything else is a normalized level -> percentage.
    return detail::formatNumber (static_cast<double> (normalised) * 100.0, 0) + "%";
}

} // namespace aeonix
