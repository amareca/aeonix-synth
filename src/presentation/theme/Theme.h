#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

#include <algorithm>
#include <cmath>

#include "presentation/core/I18n.h"

namespace aeonix
{

// ============================================================================
//  Tema visual de la aplicacion.
// ============================================================================
enum class ThemeId
{
    holoAccents = 0,
    holoGlass,
    holoMinimal,
    prototype
};

struct ThemeColours
{
    juce::Colour background;
    juce::Colour backgroundAlt;
    juce::Colour panel;
    juce::Colour panelBorder;
    juce::Colour text;
    juce::Colour muted;
    juce::Colour accent;
    juce::Colour arcTrack;
    juce::Colour keyGlow;
    bool gradientTitles   = false;
    bool gradientLines    = false;
    bool translucentPanels = false;
    bool starfield        = false;
    bool holoKnobs        = false;
};

// Degradado holografico: teal -> azul -> violeta -> ambar.
inline juce::Colour holoColour (float t)
{
    static const juce::Colour stops[] = {
        juce::Colour (0xff4ecdc4),
        juce::Colour (0xff5aa9e6),
        juce::Colour (0xffa78bfa),
        juce::Colour (0xffe6c15a)
    };

    t = juce::jlimit (0.0f, 1.0f, t);
    const float scaled = t * 3.0f;
    const int index = juce::jlimit (0, 2, static_cast<int> (std::floor (scaled)));
    const float fraction = scaled - static_cast<float> (index);

    return stops[index].interpolatedWith (stops[index + 1], fraction);
}

inline ThemeColours themeColours (ThemeId theme)
{
    switch (theme)
    {
        case ThemeId::holoAccents:
            return {
                juce::Colour (0xff0f141b),   // background
                juce::Colour (0xff121821),   // backgroundAlt
                juce::Colour (0xff1b1f27),   // panel
                juce::Colour (0xff2f3641),   // panelBorder
                juce::Colour (0xffe6ebf2),   // text
                juce::Colour (0xffa9b3c2),   // muted
                juce::Colour (0xff4ecdc4),   // accent
                juce::Colour (0xff3a414d),   // arcTrack
                juce::Colour (0xff4ecdc4),   // keyGlow
                true, true, false, false, true
            };

        case ThemeId::holoGlass:
            return {
                juce::Colour (0xff080c12),
                juce::Colour (0xff0c121a),
                juce::Colour (0xff1b2430),
                juce::Colour (0xff3a4a5c),
                juce::Colour (0xffe6ebf2),
                juce::Colour (0xffa9b3c2),
                juce::Colour (0xff4ecdc4),
                juce::Colour (0xff3a414d),
                juce::Colour (0xffa78bfa),
                true, true, true, true, true
            };

        case ThemeId::holoMinimal:
            return {
                juce::Colour (0xff11151a),
                juce::Colour (0xff0e1217),
                juce::Colour (0xff171b21),
                juce::Colour (0xff262d37),
                juce::Colour (0xffc8ced8),
                juce::Colour (0xffa9b3c2),
                juce::Colour (0xff4ecdc4),
                juce::Colour (0xff343b46),
                juce::Colour (0xff4ecdc4),
                false, false, false, false, false
            };

        case ThemeId::prototype:
        default:
            return {
                juce::Colour (0xff1b1f27),
                juce::Colour (0xff1b1f27),
                juce::Colour (0xff242a33),
                juce::Colour (0xff343c49),
                juce::Colour (0xffc8ced8),
                juce::Colour (0xffa9b3c2),
                juce::Colour (0xff4ecdc4),
                juce::Colour (0xff3a414d),
                juce::Colour (0xff4ecdc4),
                false, false, false, false, false
            };
    }
}

inline juce::String themeKey (ThemeId theme)
{
    switch (theme)
    {
        case ThemeId::holoAccents: return "holo.accents";
        case ThemeId::holoGlass:   return "holo.glass";
        case ThemeId::holoMinimal: return "holo.minimal";
        case ThemeId::prototype:   return "prototype";
    }

    return "holo.accents";
}

inline ThemeId themeFromKey (const juce::String& key)
{
    if (key == "holo.glass")   return ThemeId::holoGlass;
    if (key == "holo.minimal") return ThemeId::holoMinimal;
    if (key == "prototype")    return ThemeId::prototype;
    return ThemeId::holoAccents;
}

inline juce::String themeName (ThemeId theme)
{
    switch (theme)
    {
        case ThemeId::holoAccents: return tr (TextId::themeHoloAccents);
        case ThemeId::holoGlass:   return tr (TextId::themeHoloGlass);
        case ThemeId::holoMinimal: return tr (TextId::themeHoloMinimal);
        case ThemeId::prototype:   return tr (TextId::themePrototype);
    }

    return tr (TextId::themeHoloAccents);
}

inline int themeToIndex (ThemeId theme)
{
    return static_cast<int> (theme);
}

inline ThemeId themeFromIndex (int index)
{
    return static_cast<ThemeId> (juce::jlimit (0, 3, index));
}

} // namespace aeonix
