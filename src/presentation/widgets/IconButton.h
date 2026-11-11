#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "presentation/theme/Theme.h"

namespace aeonix
{

// ============================================================================
//  IconButton — boton compacto solo-icono, con estilo de tema.
//
//  Se usa en la barra de titulo (ajustes, MIDI, minimizar/maximizar/cerrar)
//  y junto a los presets (abrir carpeta). El nombre/tooltip es accesible; el
//  layout audit lo trata como icono (sin texto visible).
// ============================================================================
enum class IconType
{
    folder,
    settings,
    midi,
    minimise,
    maximise,
    restore,
    close
};

class IconButton final : public juce::Button
{
public:
    IconButton (IconType icon, const juce::String& tooltipText)
        : juce::Button (tooltipText), icon_ (icon)
    {
        setTooltip (tooltipText);
        setWantsKeyboardFocus (false);
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        getProperties().set ("aeonix.iconOnly", true);
    }

    void setIcon (IconType icon)
    {
        if (icon_ != icon)
        {
            icon_ = icon;
            repaint();
        }
    }

    void setTheme (ThemeId theme)
    {
        if (theme_ != theme)
        {
            theme_ = theme;
            repaint();
        }
    }

    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
        const auto colours = themeColours (theme_);
        const auto bounds = getLocalBounds().toFloat().reduced (0.5f);
        const bool isClose = icon_ == IconType::close;
        const bool hover = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;

        if (icon_ == IconType::folder)
        {
            g.setColour (hover ? colours.panelBorder.withAlpha (shouldDrawButtonAsDown ? 0.95f : 0.70f)
                               : colours.panel);
            g.fillRoundedRectangle (bounds, 5.0f);

            g.setColour (hover ? colours.accent.withAlpha (0.90f) : colours.panelBorder);
            g.drawRoundedRectangle (bounds, 5.0f, 1.0f);
        }
        else if (hover)
        {
            g.setColour (isClose
                             ? juce::Colour (0xffc4433c)
                             : colours.panelBorder.withAlpha (shouldDrawButtonAsDown ? 0.95f : 0.65f));
            g.fillRoundedRectangle (bounds, 5.0f);
        }

        const auto colour = (isClose && shouldDrawButtonAsHighlighted)
                                ? juce::Colours::white
                                : (shouldDrawButtonAsHighlighted ? colours.text : colours.muted);

        g.setColour (colour);

        const auto centre = bounds.getCentre();
        constexpr float stroke = 1.3f;
        const juce::PathStrokeType pathStroke (1.3f,
                                               juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded);

        switch (icon_)
        {
            case IconType::minimise:
                g.drawLine (centre.x - 5.0f, centre.y,
                            centre.x + 5.0f, centre.y, stroke);
                break;

            case IconType::maximise:
                g.drawRoundedRectangle (centre.x - 5.0f, centre.y - 5.0f,
                                        10.0f, 10.0f, 1.2f, 1.1f);
                break;

            case IconType::restore:
                g.drawRoundedRectangle (centre.x - 1.5f, centre.y - 5.0f,
                                        6.5f, 6.5f, 1.0f, 1.0f);
                g.drawRoundedRectangle (centre.x - 5.0f, centre.y - 1.5f,
                                        6.5f, 6.5f, 1.0f, 1.0f);
                break;

            case IconType::close:
                g.drawLine (centre.x - 4.5f, centre.y - 4.5f,
                            centre.x + 4.5f, centre.y + 4.5f, stroke);
                g.drawLine (centre.x + 4.5f, centre.y - 4.5f,
                            centre.x - 4.5f, centre.y + 4.5f, stroke);
                break;

            case IconType::settings:
                for (int i = 0; i < 3; ++i)
                {
                    const auto y = centre.y - 4.0f + static_cast<float> (i) * 4.0f;
                    const auto knobX = centre.x + ((i % 2 == 0) ? 2.4f : -2.4f);

                    g.drawLine (centre.x - 5.5f, y, knobX - 2.0f, y, 1.15f);
                    g.drawLine (knobX + 2.0f, y, centre.x + 5.5f, y, 1.15f);
                    g.drawEllipse (knobX - 2.0f, y - 2.0f, 4.0f, 4.0f, 1.15f);
                }
                break;

            case IconType::midi:
            {
                const auto box = juce::Rectangle<float> (centre.x - 6.0f, centre.y - 4.5f,
                                                         12.0f, 9.0f);
                g.drawRoundedRectangle (box, 1.5f, 1.1f);

                for (int i = 0; i < 4; ++i)
                {
                    const auto x = box.getX() + 2.4f + static_cast<float> (i) * 2.4f;
                    g.drawLine (x, box.getY() + 2.6f, x, box.getY() + 5.1f, 1.0f);
                }
                break;
            }

            case IconType::folder:
            {
                juce::Path body;
                body.addRoundedRectangle (centre.x - 6.5f, centre.y - 2.0f,
                                          13.0f, 8.0f, 1.6f);
                g.strokePath (body, pathStroke);

                juce::Path tab;
                tab.startNewSubPath (centre.x - 6.5f, centre.y - 2.0f);
                tab.lineTo (centre.x - 6.5f, centre.y - 4.3f);
                tab.lineTo (centre.x - 2.2f, centre.y - 4.3f);
                tab.lineTo (centre.x - 0.8f, centre.y - 2.0f);
                g.strokePath (tab, pathStroke);
                break;
            }
        }
    }

private:
    IconType icon_;
    ThemeId theme_ = ThemeId::holoAccents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IconButton)
};

} // namespace aeonix
