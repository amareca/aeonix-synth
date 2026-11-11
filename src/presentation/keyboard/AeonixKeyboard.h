#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"

namespace aeonix
{

// ============================================================================
//  AeonixKeyboard — teclado "Studio Ivory".
//
//  Hereda de MidiKeyboardComponent para dibujar las teclas a mano:
//  marfil cálido, negras de carbón, sombras, hover sutil y glow teal al
//  pulsar. Mantiene intacto el estado MIDI, el clic y el feedback.
// ============================================================================
class AeonixKeyboard : public juce::MidiKeyboardComponent
{
public:
    AeonixKeyboard (juce::MidiKeyboardState& state, Orientation orientation)
        : juce::MidiKeyboardComponent (state, orientation)
    {
        setColour (juce::MidiKeyboardComponent::whiteNoteColourId,
                   juce::Colour (0xfff7f4eb));
        setColour (juce::MidiKeyboardComponent::blackNoteColourId,
                   juce::Colour (0xff1a1e24));
        setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,
                   juce::Colour (0xffb8b2a5));
        setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                   juce::Colour (0x12ffffff));
        setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,
                   juce::Colour (0x604ecdc4));
        setColour (juce::MidiKeyboardComponent::textLabelColourId,
                   juce::Colour (0xff6f6a60));
        setColour (juce::MidiKeyboardComponent::shadowColourId,
                   juce::Colour (0x60000000));
    }

    void setTheme (ThemeId theme)
    {
        theme_ = theme;
        repaint();
    }

    // ------------------------------------------------------------------------
    void drawWhiteNote (int midiNoteNumber,
                        juce::Graphics& g,
                        juce::Rectangle<float> area,
                        bool isDown,
                        bool isOver,
                        juce::Colour lineColour,
                        juce::Colour textColour) override
    {
        juce::ignoreUnused (textColour);

        auto r = area.reduced (0.5f);

        juce::Path key;
        key.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                 3.0f, 3.0f,
                                 false, false, true, true);

        juce::ColourGradient gradient (juce::Colour (0xfff7f4eb), r.getCentreX(), r.getY(),
                                       juce::Colour (0xffd8d2c4), r.getCentreX(), r.getBottom(),
                                       false);
        g.setGradientFill (gradient);
        g.fillPath (key);

        // Borde y separador.
        g.setColour (lineColour.withAlpha (0.9f));
        g.strokePath (key, juce::PathStrokeType (1.0f));

        // Sombra inferior muy suave.
        g.setColour (juce::Colour (0x18000000));
        g.fillRect (r.getX() + 1.0f, r.getBottom() - 2.0f, r.getWidth() - 2.0f, 2.0f);

        if (isOver)
        {
            g.setColour (juce::Colour (0x10ffffff));
            g.fillPath (key);
        }

        if (isDown)
        {
            if (theme_ == ThemeId::holoGlass)
            {
                juce::ColourGradient glow (holoColour (0.05f).withAlpha (0.42f),
                                           r.getX(), r.getY(),
                                           holoColour (0.75f).withAlpha (0.30f),
                                           r.getX(), r.getBottom(),
                                           false);
                g.setGradientFill (glow);
                g.fillPath (key);
                g.setColour (holoColour (0.85f).withAlpha (0.85f));
            }
            else
            {
                g.setColour (themeColours (theme_).keyGlow.withAlpha (0.33f));
                g.fillPath (key);
                g.setColour (themeColours (theme_).keyGlow.withAlpha (0.67f));
            }

            g.strokePath (key, juce::PathStrokeType (1.4f));
        }

        // Etiqueta solo en las C (la base devuelve vacio para el resto).
        if (const auto label = getWhiteNoteText (midiNoteNumber); label.isNotEmpty())
        {
            g.setColour (juce::Colour (0xff6f6a60));
            g.setFont (ui::font (9.5f));
            g.drawText (label,
                        r.withTrimmedTop (r.getHeight() - ui::px (19)),
                        juce::Justification::centred,
                        false);
        }
    }

    // ------------------------------------------------------------------------
    void drawBlackNote (int midiNoteNumber,
                        juce::Graphics& g,
                        juce::Rectangle<float> area,
                        bool isDown,
                        bool isOver,
                        juce::Colour noteFillColour) override
    {
        juce::ignoreUnused (midiNoteNumber, noteFillColour);

        auto r = area.reduced (0.5f);

        // Sombra proyectada.
        const auto shadowRect = r.translated (0.0f, 2.5f);
        juce::Path shadow;
        shadow.addRoundedRectangle (shadowRect.getX(), shadowRect.getY(),
                                    shadowRect.getWidth(), shadowRect.getHeight(),
                                    3.0f, 3.0f,
                                    false, false, true, true);
        g.setColour (juce::Colour (0x55000000));
        g.fillPath (shadow);

        juce::Path key;
        key.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                 3.0f, 3.0f,
                                 false, false, true, true);

        juce::ColourGradient gradient (juce::Colour (0xff2a2f38), r.getCentreX(), r.getY(),
                                       juce::Colour (0xff101318), r.getCentreX(), r.getBottom(),
                                       false);
        g.setGradientFill (gradient);
        g.fillPath (key);

        g.setColour (juce::Colour (0xff0a0c10));
        g.strokePath (key, juce::PathStrokeType (1.0f));

        if (isOver)
        {
            g.setColour (juce::Colour (0x12ffffff));
            g.fillPath (key);
        }

        if (isDown)
        {
            if (theme_ == ThemeId::holoGlass)
            {
                juce::ColourGradient glow (holoColour (0.05f).withAlpha (0.50f),
                                           r.getX(), r.getY(),
                                           holoColour (0.75f).withAlpha (0.38f),
                                           r.getX(), r.getBottom(),
                                           false);
                g.setGradientFill (glow);
                g.fillPath (key);
                g.setColour (holoColour (0.85f).withAlpha (0.9f));
            }
            else
            {
                g.setColour (themeColours (theme_).keyGlow.withAlpha (0.42f));
                g.fillPath (key);
                g.setColour (themeColours (theme_).keyGlow.withAlpha (0.56f));
            }

            g.strokePath (key, juce::PathStrokeType (1.4f));
        }

        // Brillo superior (se dibuja al final para que siga visible al pulsar).
        g.setColour (juce::Colour (0x22ffffff));
        g.fillRoundedRectangle (r.getX() + 1.0f, r.getY() + 1.0f,
                                r.getWidth() - 2.0f, 2.0f, 1.0f);
    }

private:
    ThemeId theme_ = ThemeId::prototype;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AeonixKeyboard)
};

} // namespace aeonix
