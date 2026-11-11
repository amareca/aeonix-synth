#pragma once

#include <juce_core/juce_core.h>

#include <string>
#include <string_view>

#include "domain/ParameterIds.h"
#include "presentation/core/I18n.h"

namespace aeonix
{

// ============================================================================
//  ParameterMetadata — textos de interfaz de cada parametro.
//
//  Un solo punto de verdad para la etiqueta corta y el tooltip que se muestran
//  en los knobs. La tabla guarda claves del catalogo (I18n) y parameterUiInfo()
//  devuelve el texto ya traducido al idioma activo. Los ids vienen de
//  domain/ParameterIds.h para no duplicar cadenas y evitar erratas.
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

    // Fallback defensivo: si se anade un parametro sin metadatos, al menos
    // la UI sigue funcionando y muestra el id.
    return { std::string (id), juce::String (id), juce::String (id) };
}

} // namespace aeonix
