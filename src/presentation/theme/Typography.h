#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "presentation/core/I18n.h"
#include "presentation/core/TextSize.h"

namespace aeonix::ui
{

// ============================================================================
//  Tipografia centralizada.
//
//  Nunca escribas tamanos de fuente a mano en las vistas: usa ui::font() y
//  las alturas derivadas. Asi el ajuste Normal / Grande / Muy grande cambia
//  toda la interfaz de forma coherente.
// ============================================================================
inline TextSize& textSizeState()
{
    static TextSize value = TextSize::normal;
    return value;
}

inline void setTextSize (TextSize size)       { textSizeState() = size; }
inline TextSize getTextSize()                 { return textSizeState(); }

// Base real de la tipografia. Los tamanos de las vistas siguen expresados
// en la rejilla historica de 11 px; font() aplica 12/11 y luego el nivel
// Normal / Grande / Muy grande.
inline constexpr float baseFontScale = 12.0f / 11.0f;

inline const char* typefaceName() noexcept { return "Segoe UI"; }

inline float fontScale()   { return textFontScaleForSize (getTextSize()); }
inline float layoutScale() { return textLayoutScaleForSize (getTextSize()); }

inline juce::Font font (float basePixels, int style = juce::Font::plain)
{
    const auto height = basePixels * baseFontScale * fontScale();

    return juce::Font (juce::FontOptions (typefaceName(), height, style)
                           .withFallbacks ({ "Tahoma" }));
}

inline int px (int base)
{
    return juce::roundToInt (static_cast<float> (base) * layoutScale());
}

// Campos de una linea: texto, caret y placeholder comparten linea base.
// Sin la justificacion vertical, JUCE pega el texto al borde superior.
inline void styleSingleLineEditor (juce::TextEditor& editor,
                                   float basePixels = 12.0f,
                                   int leftIndentBase = 8)
{
    editor.setFont (font (basePixels));
    editor.setJustification (juce::Justification::centredLeft);
    editor.setIndents (px (leftIndentBase), 0);
}

inline int lineHeight (float basePixels = 11.0f)
{
    return juce::roundToInt (font (basePixels).getHeight());
}

inline int labelHeight()          { return lineHeight (11.0f) + 3; }
inline int valueBoxHeight()       { return lineHeight (11.0f) + 3; }
inline int comboHeight()          { return lineHeight (12.0f) + 10; }
inline int presetRowHeight()      { return lineHeight (12.0f) + 14; }
inline int sectionTitleHeight()   { return lineHeight (11.0f) + 15; }

inline int sectionRow1Height()    { return 150 + (labelHeight() - 16) * 2; }
inline int sectionRow2Height()    { return 140 + (labelHeight() - 16) * 2; }

inline int presetBarHeight()
{
    return presetRowHeight() + px (8);
}

inline juce::String textSizeName (TextSize size)
{
    switch (size)
    {
        case TextSize::large:      return tr (TextId::textSizeLarge);
        case TextSize::extraLarge: return tr (TextId::textSizeExtraLarge);
        case TextSize::normal:
        default:                   return tr (TextId::textSizeNormal);
    }
}

} // namespace aeonix::ui
