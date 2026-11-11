#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "presentation/theme/Typography.h"

namespace aeonix
{

// ============================================================================
//  Holo Delta — marca de AEONIX.
//
//  Triángulo relleno con hueco interior y corte transversal, pintado con el
//  degradado holográfico de la app (teal -> azul -> violeta -> ámbar).
//  Se usa en la cabecera y para el icono de la ventana.
// ============================================================================

namespace holo
{

inline juce::Path createDeltaPath (juce::Rectangle<float> area)
{
    const auto scale = area.getWidth() / 100.0f;

    juce::Path path;
    path.setUsingNonZeroWinding (false);
    path.addTriangle (50.0f, 10.0f, 90.0f, 86.0f, 10.0f, 86.0f);
    path.addTriangle (50.0f, 32.0f, 72.0f, 74.0f, 28.0f, 74.0f);
    path.addRectangle (31.0f, 57.0f, 38.0f, 7.0f);

    path.applyTransform (juce::AffineTransform::scale (scale)
                             .translated (area.getX(), area.getY()));
    return path;
}

inline juce::ColourGradient createGradient (juce::Rectangle<float> area)
{
    juce::ColourGradient gradient (juce::Colour (0xff4ecdc4),
                                   area.getX(), area.getY(),
                                   juce::Colour (0xffe6c15a),
                                   area.getRight(), area.getBottom(),
                                   false);

    gradient.addColour (0.30, juce::Colour (0xff5aa9e6));
    gradient.addColour (0.62, juce::Colour (0xffa78bfa));
    gradient.addColour (0.85, juce::Colour (0xffe6c15a));
    return gradient;
}

inline void drawDelta (juce::Graphics& g, juce::Rectangle<float> area, bool drawGlow = true)
{
    if (drawGlow && area.getWidth() > 14.0f)
    {
        const auto centre = area.getCentre();
        juce::ColourGradient glow (juce::Colour (0xff5aa9e6).withAlpha (0.16f),
                                   centre.x, centre.y,
                                   juce::Colours::transparentBlack,
                                   centre.x + area.getWidth() * 0.72f,
                                   centre.y,
                                   true);
        g.setGradientFill (glow);
        g.fillEllipse (area.expanded (area.getWidth() * 0.20f));
    }

    g.setGradientFill (createGradient (area));
    g.fillPath (createDeltaPath (area));
}

} // namespace holo

// Icono de app: tile redondeado oscuro + Holo Delta centrado.
inline juce::Image createAeonixAppIcon (int size)
{
    juce::Image image (juce::Image::ARGB, size, size, true);
    juce::Graphics g (image);

    const auto tile = juce::Rectangle<float> (0.0f, 0.0f,
                                              static_cast<float> (size),
                                              static_cast<float> (size));

    const auto radius = size * 0.24f;

    g.setColour (juce::Colour (0xff141922));
    g.fillRoundedRectangle (tile, radius);

    juce::ColourGradient bg (juce::Colour (0xff5aa9e6).withAlpha (0.18f),
                             size * 0.5f, size * 0.22f,
                             juce::Colours::transparentBlack,
                             size * 0.5f, size * 0.98f,
                             true);
    g.setGradientFill (bg);
    g.fillRoundedRectangle (tile, radius);

    const auto iconSize = size * 0.66f;
    const auto iconArea = juce::Rectangle<float> ((size - iconSize) * 0.5f,
                                                  (size - iconSize) * 0.5f,
                                                  iconSize, iconSize);

    holo::drawDelta (g, iconArea, size >= 32);

    g.setColour (juce::Colour (0xff2b333f));
    g.drawRoundedRectangle (tile.reduced (0.5f), radius, 1.0f);

    return image;
}

// ============================================================================
//  AeonixLogo — icono Holo Delta + wordmark "AEONIX" con degradado.
// ============================================================================
class AeonixLogo : public juce::Component
{
public:
    AeonixLogo()
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto iconSize = juce::jmin (30.0f, bounds.getHeight() - 2.0f);

        const auto iconArea = juce::Rectangle<float> (0.0f,
                                                      (bounds.getHeight() - iconSize) * 0.5f,
                                                      iconSize, iconSize);

        holo::drawDelta (g, iconArea);

        const auto font = aeonix::ui::font (16.0f);
        constexpr float tracking = 1.8f;
        const juce::String text ("AEONIX");

        juce::GlyphArrangement glyphs;
        float x = iconArea.getRight() + 10.0f;
        const float baseline = bounds.getCentreY()
                             + (font.getAscent() - font.getDescent()) * 0.5f;

        for (int i = 0; i < text.length(); ++i)
        {
            const auto letter = text.substring (i, i + 1);
            glyphs.addLineOfText (font, letter, x, baseline);
            x += juce::GlyphArrangement::getStringWidth (font, letter) + tracking;
        }

        juce::Path textPath;
        glyphs.createPath (textPath);

        juce::ColourGradient gradient (juce::Colour (0xff4ecdc4),
                                       iconArea.getRight() + 10.0f, 0.0f,
                                       juce::Colour (0xffe6c15a),
                                       x, 0.0f,
                                       false);
        gradient.addColour (0.30, juce::Colour (0xff5aa9e6));
        gradient.addColour (0.62, juce::Colour (0xffa78bfa));
        gradient.addColour (0.85, juce::Colour (0xffe6c15a));

        g.setGradientFill (gradient);
        g.fillPath (textPath);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AeonixLogo)
};

} // namespace aeonix
