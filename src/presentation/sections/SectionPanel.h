#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <utility>

#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"

namespace aeonix
{

// ============================================================================
//  SectionPanel — caja visual de una seccion del sintetizador.
//
//  Pinta fondo, borde y titulo. En Prototipo usa el color de acento de la
//  seccion; en los temas Holo usa el degradado y las lineas del tema.
// ============================================================================
class SectionPanel : public juce::Component
{
public:
    SectionPanel (juce::String title, juce::Colour accent)
        : title_ (std::move (title)), accent_ (accent)
    {
        setInterceptsMouseClicks (false, true);
        setTitle (title_);
    }

    [[nodiscard]] juce::Colour getAccentColour() const noexcept { return accent_; }
    [[nodiscard]] const juce::String& getTitle() const noexcept { return title_; }

    void setTheme (ThemeId theme)
    {
        theme_ = theme;
        repaint();
    }

    // Area util (debajo del titulo) en coordenadas locales del panel.
    [[nodiscard]] juce::Rectangle<int> getContentBounds() const
    {
        return getLocalBounds().withTrimmedTop (ui::sectionTitleHeight())
                               .reduced (ui::px (10), 4);
    }

    void paint (juce::Graphics& g) override
    {
        const auto colours = themeColours (theme_);
        const auto bounds = getLocalBounds().toFloat().reduced (0.5f);

        if (colours.translucentPanels)
        {
            g.setColour (juce::Colour (0x33000000));
            g.fillRoundedRectangle (bounds.translated (0.0f, 2.0f), cornerRadius);
        }

        g.setColour (colours.translucentPanels
                         ? colours.panel.withAlpha (0.68f)
                         : colours.panel);
        g.fillRoundedRectangle (bounds, cornerRadius);

        g.setColour (colours.panelBorder.withAlpha (colours.translucentPanels ? 0.85f : 1.0f));
        g.drawRoundedRectangle (bounds, cornerRadius, 1.0f);

        if (colours.gradientLines)
        {
            const auto line = getLocalBounds().removeFromTop (2).reduced (ui::px (10), 0);
            juce::ColourGradient gradient (holoColour (0.0f),
                                           static_cast<float> (line.getX()), 0.0f,
                                           holoColour (1.0f),
                                           static_cast<float> (line.getRight()), 0.0f,
                                           false);
            g.setGradientFill (gradient);
            g.fillRect (line);
        }

        auto titleArea = getLocalBounds().removeFromTop (ui::sectionTitleHeight())
                                         .reduced (ui::px (10), 0);
        const auto titleFont = ui::font (11.0f, juce::Font::bold);

        if (theme_ == ThemeId::prototype)
        {
            g.setColour (accent_);
            g.fillRoundedRectangle (static_cast<float> (titleArea.getX()),
                                    static_cast<float> (titleArea.getCentreY()) - 6.0f,
                                    3.0f, 12.0f, 1.5f);

            g.setColour (accent_);
            g.setFont (titleFont);
            g.drawText (title_,
                        titleArea.withTrimmedLeft (10),
                        juce::Justification::centredLeft,
                        true);

            g.setColour (accent_.withAlpha (0.25f));
            g.fillRect (ui::px (10), ui::sectionTitleHeight() - 3,
                        getWidth() - ui::px (10) * 2, 1);
        }
        else if (theme_ == ThemeId::holoMinimal)
        {
            g.setColour (colours.text);
            g.setFont (titleFont);
            g.drawText (title_,
                        titleArea.withTrimmedLeft (4),
                        juce::Justification::centredLeft,
                        true);
        }
        else
        {
            // Titulo con degradado Holo.
            const auto textArea = titleArea.withTrimmedLeft (4);
            const auto baseline = static_cast<float> (textArea.getCentreY())
                                + (titleFont.getAscent() - titleFont.getDescent()) * 0.5f;

            juce::GlyphArrangement glyphs;
            glyphs.addLineOfText (titleFont, title_,
                                  static_cast<float> (textArea.getX()), baseline);

            juce::Path textPath;
            glyphs.createPath (textPath);

            const auto textWidth = juce::GlyphArrangement::getStringWidth (titleFont, title_);
            juce::ColourGradient gradient (holoColour (0.05f),
                                           static_cast<float> (textArea.getX()), 0.0f,
                                           holoColour (0.95f),
                                           static_cast<float> (textArea.getX()) + textWidth, 0.0f,
                                           false);
            g.setGradientFill (gradient);
            g.fillPath (textPath);
        }
    }

private:
    static constexpr float cornerRadius = 8.0f;

    juce::String title_;
    juce::Colour accent_;
    ThemeId theme_ = ThemeId::prototype;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionPanel)
};

} // namespace aeonix
