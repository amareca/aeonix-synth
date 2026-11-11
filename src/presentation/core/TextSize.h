#pragma once

#include <juce_core/juce_core.h>

namespace aeonix
{

// ============================================================================
//  TextSize — escala de legibilidad de la interfaz.
//
//  La app arranca en el tamano detectado por la altura de pantalla si no hay
//  una eleccion guardada. El valor se persiste en config.json con la clave
//  "ui.textSize" ("normal", "large" o "xlarge") y se aplica al reiniciar.
// ============================================================================
enum class TextSize
{
    normal = 0,
    large,
    extraLarge
};

inline juce::String textSizeKey (TextSize size)
{
    switch (size)
    {
        case TextSize::large:      return "large";
        case TextSize::extraLarge: return "xlarge";
        case TextSize::normal:
        default:                   return "normal";
    }
}

inline TextSize textSizeFromKey (const juce::String& key)
{
    if (key == "large")  return TextSize::large;
    if (key == "xlarge") return TextSize::extraLarge;
    return TextSize::normal;
}

// Altura fisica (px reales, ya multiplicada por el escalado de Windows).
// 768 px se queda en Normal; 1080p arranca en Grande; 1440p y 4K en Muy grande.
inline TextSize defaultTextSizeForHeight (int screenHeight)
{
    if (screenHeight >= 1300)
        return TextSize::extraLarge;

    return screenHeight >= 900 ? TextSize::large : TextSize::normal;
}

// Escala de fuente. La base real es 12 px: 12 / 15 / 18.
inline float textFontScaleForSize (TextSize size)
{
    switch (size)
    {
        case TextSize::large:      return 15.0f / 12.0f;
        case TextSize::extraLarge: return 18.0f / 12.0f;
        case TextSize::normal:
        default:                   return 1.0f;
    }
}

// El layout crece menos que la fuente: el texto gana presencia sin que la
// ventana se dispare. Normal 1042x714, Grande 1160x724, XL 1300x744.
inline float textLayoutScaleForSize (TextSize size)
{
    switch (size)
    {
        case TextSize::large:      return 1.07f;
        case TextSize::extraLarge: return 1.15f;
        case TextSize::normal:
        default:                   return 1.0f;
    }
}

inline int minWindowWidthForSize (TextSize size)
{
    switch (size)
    {
        case TextSize::large:      return 1160;
        case TextSize::extraLarge: return 1300;
        case TextSize::normal:
        default:                   return 1042;
    }
}

inline int minWindowHeightForSize (TextSize size)
{
    switch (size)
    {
        case TextSize::large:      return 724;
        case TextSize::extraLarge: return 744;
        case TextSize::normal:
        default:                   return 714;
    }
}

inline int defaultWindowWidthForSize (TextSize size)
{
    switch (size)
    {
        case TextSize::large:      return 1220;
        case TextSize::extraLarge: return 1320;
        case TextSize::normal:
        default:                   return 1182;
    }
}

inline int defaultWindowHeightForSize (TextSize size)
{
    switch (size)
    {
        case TextSize::large:      return 770;
        case TextSize::extraLarge: return 790;
        case TextSize::normal:
        default:                   return 754;
    }
}

} // namespace aeonix
