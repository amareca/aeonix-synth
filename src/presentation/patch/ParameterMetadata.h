#pragma once

#include <juce_core/juce_core.h>

#include <string>
#include <string_view>

#include "domain/ParameterIds.h"
#include "presentation/core/I18n.h"

namespace aeonix
{

// ============================================================================
//  ParameterMetadata - interface texts for each parameter.
//
//  A single source of truth for the short label and tooltip shown on the
//  knobs. The table stores catalogue keys (I18n) and parameterUiInfo()
//  returns the text already translated to the active language. The ids come
//  from domain/ParameterIds.h to avoid duplicating strings and typos.
// ============================================================================
struct ParameterUiInfo
{
    std::string id;
    juce::String label;
    juce::String tooltip;
};

inline ParameterUiInfo parameterUiInfo (const char* id)
{
    using namespace param;

    struct Entry
    {
        const char* id;
        TextId label;
        TextId tooltip;
    };

    static constexpr Entry entries[] = {
        { masterVolume,    TextId::paramVolume,         TextId::tipVolume },
        { oscLevel,        TextId::paramOscLevel,       TextId::tipOscLevel },
        { filterCutoff,    TextId::paramCutoff,         TextId::tipCutoff },
        { filterResonance, TextId::paramResonance,      TextId::tipResonance },
        { filterEnvAmount, TextId::paramEnvAmount,      TextId::tipEnvAmount },
        { filterAttack,    TextId::paramFilterAttack,   TextId::tipFilterAttack },
        { filterDecay,     TextId::paramFilterDecay,    TextId::tipFilterDecay },
        { filterSustain,   TextId::paramFilterSustain,  TextId::tipFilterSustain },
        { filterRelease,   TextId::paramFilterRelease,  TextId::tipFilterRelease },
        { ampAttack,       TextId::paramAmpAttack,      TextId::tipAmpAttack },
        { ampDecay,        TextId::paramAmpDecay,       TextId::tipAmpDecay },
        { ampSustain,      TextId::paramAmpSustain,     TextId::tipAmpSustain },
        { ampRelease,      TextId::paramAmpRelease,     TextId::tipAmpRelease },
        { lfoRate,         TextId::paramLfoRate,        TextId::tipLfoRate },
        { lfoDepth,        TextId::paramLfoDepth,       TextId::tipLfoDepth },
        { unisonVoices,    TextId::paramVoices,         TextId::tipVoices },
        { unisonDetune,    TextId::paramDetune,         TextId::tipDetune },
        { unisonWidth,     TextId::paramSpread,         TextId::tipSpread },
        { delayTime,       TextId::paramDelayTime,      TextId::tipDelayTime },
        { delayFeedback,   TextId::paramDelayFeedback,  TextId::tipDelayFeedback },
        { delayMix,        TextId::paramDelayMix,       TextId::tipDelayMix },
        { reverbSize,      TextId::paramReverbSize,     TextId::tipReverbSize },
        { reverbMix,       TextId::paramReverbMix,      TextId::tipReverbMix },
    };

    for (const auto& entry : entries)
        if (std::string_view (id) == entry.id)
            return { std::string (id), tr (entry.label), tr (entry.tooltip) };

    // Defensive fallback: if a parameter is added without metadata, at least
    // the UI keeps working and shows the id.
    return { std::string (id), juce::String (id), juce::String (id) };
}

} // namespace aeonix
