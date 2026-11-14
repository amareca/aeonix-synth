#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>

#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"

namespace aeonix
{

// ============================================================================
//  AeonixLookAndFeel — estilo visual de la aplicacion.
//
//  Cuatro temas: Holo Accents, Holo Glass, Holo Minimal y Prototipo.
//  El color de acento por seccion se sigue leyendo de
//  Slider::rotarySliderFillColourId en Prototipo; los temas Holo usan el
//  degradado teal -> azul -> violeta -> ambar.
// ============================================================================
class AeonixLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AeonixLookAndFeel()
    {
        setTheme (ThemeId::holoAccents);
    }

    void setTheme (ThemeId theme)
    {
        theme_ = theme;
        colours_ = themeColours (theme);

        const auto& c = colours_;

        setColour (juce::ResizableWindow::backgroundColourId, c.background);
        setColour (juce::DocumentWindow::textColourId, c.text);

        setColour (juce::TextButton::buttonColourId, c.panel);
        setColour (juce::TextButton::buttonOnColourId, c.panelBorder);
        setColour (juce::TextButton::textColourOffId, c.text);
        setColour (juce::TextButton::textColourOnId, juce::Colours::white);

        setColour (juce::ComboBox::backgroundColourId, c.panel);
        setColour (juce::ComboBox::outlineColourId, c.panelBorder);
        setColour (juce::ComboBox::textColourId, c.text);
        setColour (juce::ComboBox::arrowColourId, c.muted);

        setColour (juce::PopupMenu::backgroundColourId, c.panel);
        setColour (juce::PopupMenu::textColourId, c.text);

        setColour (juce::Label::textColourId, c.text);

        setColour (juce::Slider::rotarySliderOutlineColourId, c.arcTrack);
        setColour (juce::Slider::rotarySliderFillColourId, c.accent);
        setColour (juce::Slider::thumbColourId, c.accent);
        setColour (juce::Slider::textBoxTextColourId, c.muted);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

        setColour (juce::TextEditor::backgroundColourId, c.panel);
        setColour (juce::TextEditor::outlineColourId, c.panelBorder);
        setColour (juce::TextEditor::textColourId, c.text);
        setColour (juce::TextEditor::highlightColourId, c.accent.withAlpha (0.5f));
        setColour (juce::CaretComponent::caretColourId, c.accent);

        setColour (juce::TooltipWindow::backgroundColourId, c.panel);
        setColour (juce::TooltipWindow::textColourId, c.text);
        setColour (juce::TooltipWindow::outlineColourId, c.panelBorder);
    }

    [[nodiscard]] ThemeId getTheme() const noexcept { return theme_; }

    // ------------------------------------------------------------------------
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

        if (radius <= 2.0f)
            return;

        const auto centre    = bounds.getCentre();
        const auto lineWidth = juce::jmax (2.0f, radius * 0.16f);
        const auto arcRadius = radius - lineWidth * 0.5f;
        const auto angle     = rotaryStartAngle
                             + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        const auto outline = colours_.arcTrack;
        const auto fill    = (theme_ == ThemeId::prototype)
                                 ? slider.findColour (juce::Slider::rotarySliderFillColourId)
                                 : colours_.accent;

        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (outline);
        g.strokePath (track, juce::PathStrokeType (lineWidth,
                                                   juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        if (slider.isEnabled() && sliderPosProportional > 0.0001f)
        {
            if (colours_.holoKnobs)
            {
                const int steps = juce::jlimit (8, 64,
                    juce::roundToInt ((angle - rotaryStartAngle) / 0.06f) + 1);

                // Los angulos de Path::addCentredArc se miden en sentido
                // horario desde las 12 en punto (0 = arriba), igual que
                // Point::getPointOnCircumference. Hay que usar esa misma
                // convencion o el relleno queda girado 90 grados.
                auto pointOnArc = [&] (float a)
                {
                    return juce::Point<float> (centre.x + arcRadius * std::sin (a),
                                               centre.y - arcRadius * std::cos (a));
                };

                const auto fullRange = rotaryEndAngle - rotaryStartAngle;

                for (int i = 0; i < steps; ++i)
                {
                    const auto t0 = static_cast<float> (i) / static_cast<float> (steps);
                    const auto t1 = static_cast<float> (i + 1) / static_cast<float> (steps);
                    const auto a0 = rotaryStartAngle + (angle - rotaryStartAngle) * t0;
                    const auto a1 = rotaryStartAngle + (angle - rotaryStartAngle) * t1;

                    // Color segun la posicion dentro del recorrido COMPLETO:
                    // asi el minimo es teal y el maximo ambar, y el puntero
                    // siempre coincide con el color del final del relleno.
                    const auto globalT = (a0 - rotaryStartAngle) / fullRange;
                    const auto colour = holoColour (globalT);

                    if (theme_ == ThemeId::holoGlass)
                    {
                        g.setColour (colour.withAlpha (0.16f));
                        g.drawLine (juce::Line<float> (pointOnArc (a0), pointOnArc (a1)),
                                    lineWidth * 2.3f);
                    }

                    g.setColour (colour);
                    g.drawLine (juce::Line<float> (pointOnArc (a0), pointOnArc (a1)),
                                lineWidth);
                }
            }
            else
            {
                juce::Path value;
                value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                     rotaryStartAngle, angle, true);
                g.setColour (fill);
                g.strokePath (value, juce::PathStrokeType (lineWidth,
                                                           juce::PathStrokeType::curved,
                                                           juce::PathStrokeType::rounded));
            }
        }

        const auto innerRadius = juce::jmax (2.0f, arcRadius - lineWidth * 1.9f);

        const auto innerColour = theme_ == ThemeId::holoGlass
                                     ? juce::Colour (0xff18202a)
                                     : (theme_ == ThemeId::holoMinimal
                                            ? juce::Colour (0xff171b21)
                                            : juce::Colour (0xff20252d));

        g.setColour (innerColour);
        g.fillEllipse (centre.x - innerRadius, centre.y - innerRadius,
                       innerRadius * 2.0f, innerRadius * 2.0f);

        g.setColour (outline.withAlpha (0.9f));
        g.drawEllipse (centre.x - innerRadius, centre.y - innerRadius,
                       innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

        const auto pointerColour = colours_.holoKnobs
                                       ? holoColour (sliderPosProportional)
                                       : fill;

        juce::Path pointer;
        pointer.addRoundedRectangle (-1.5f, -innerRadius * 0.92f,
                                     3.0f, innerRadius * 0.55f, 1.0f);
        // El rectangulo apunta hacia arriba (12 en punto) y "angle" tambien se
        // mide desde arriba en sentido horario, asi que basta rotar por angle.
        pointer.applyTransform (juce::AffineTransform::rotation (angle)
                                    .translated (centre.x, centre.y));
        g.setColour (pointerColour);
        g.fillPath (pointer);
    }

    // ------------------------------------------------------------------------
    void drawLinearSlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           juce::Slider::SliderStyle style,
                           juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearVertical)
        {
            juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height,
                                                    sliderPos, minSliderPos, maxSliderPos,
                                                    style, slider);
            return;
        }

        juce::ignoreUnused (minSliderPos, maxSliderPos);

        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
        const auto centreX = bounds.getCentreX();
        const auto trackTop = bounds.getY() + 6.0f;
        const auto trackBottom = bounds.getBottom() - 6.0f;
        const auto track = juce::Rectangle<float> (centreX - 3.0f,
                                                   trackTop,
                                                   6.0f,
                                                   juce::jmax (1.0f, trackBottom - trackTop));

        g.setColour (colours_.arcTrack.withAlpha (0.9f));
        g.fillRoundedRectangle (track, 3.0f);

        // Marca central de Pitch (punto neutro).
        if (slider.getName() == "Pitch")
        {
            const auto midY = (trackTop + trackBottom) * 0.5f;
            g.setColour (colours_.muted.withAlpha (0.6f));
            g.fillRoundedRectangle (centreX - 5.0f, midY - 1.0f, 10.0f, 2.0f, 1.0f);
        }

        const auto thumbY = juce::jlimit (trackTop, trackBottom, sliderPos);
        const auto thumbColour = slider.findColour (juce::Slider::thumbColourId);
        const auto active = slider.isMouseOverOrDragging();
        const auto glow = theme_ == ThemeId::holoGlass
                              ? (active ? 0.45f : 0.30f)
                              : (active ? 0.35f : 0.22f);

        g.setColour (thumbColour.withAlpha (glow));
        g.fillEllipse (centreX - 9.0f, thumbY - 9.0f, 18.0f, 18.0f);

        g.setColour (active ? thumbColour.brighter (0.12f) : thumbColour);
        g.fillEllipse (centreX - 6.0f, thumbY - 6.0f, 12.0f, 12.0f);

        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawEllipse (centreX - 6.0f, thumbY - 6.0f, 12.0f, 12.0f, 1.0f);
    }

    // ------------------------------------------------------------------------
    //  Menu emergente (PopupMenu y desplegables de ComboBox).
    //  Un solo lenguaje: panel, borde de 1 px, radio 4, filas de 24 y la opcion
    //  activa con tinte de acento y check dibujado.
    // ------------------------------------------------------------------------
    int getPopupMenuBorderSize() override { return ui::px (4); }

    juce::Font getPopupMenuFont() override { return ui::font (12.0f); }

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        // La ventana del menu es opaca, asi que se rellena entera y el borde
        // redondeado se dibuja dentro. Sobre el panel del gestor las esquinas
        // no se ven porque comparten color.
        g.fillAll (colours_.panel);

        const auto bounds = juce::Rectangle<float> (0.0f, 0.0f,
                                                    static_cast<float> (width),
                                                    static_cast<float> (height)).reduced (0.5f);

        g.setColour (colours_.panelBorder);
        g.drawRoundedRectangle (bounds, static_cast<float> (ui::px (4)), 1.0f);
    }

    void drawPopupMenuItem (juce::Graphics& g,
                            const juce::Rectangle<int>& area,
                            bool isSeparator,
                            bool isActive,
                            bool isHighlighted,
                            bool isTicked,
                            bool hasSubMenu,
                            const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon,
                            const juce::Colour* textColourToUse) override
    {
        if (isSeparator)
        {
            auto line = area.reduced (ui::px (8), 0);
            g.setColour (colours_.panelBorder.withAlpha (0.55f));
            g.fillRect (line.getX(), area.getCentreY(), line.getWidth(), 1);
            return;
        }

        auto r = area.reduced (ui::px (4), ui::px (2));

        if (isHighlighted && isActive)
        {
            g.setColour (colours_.accent.withAlpha (isTicked ? 0.14f : 0.08f));
            g.fillRoundedRectangle (r.toFloat(), static_cast<float> (ui::px (4)));
        }

        auto colour = isTicked ? colours_.accent : colours_.text;

        if (! isActive)
            colour = colours_.muted.withAlpha (0.45f);

        if (textColourToUse != nullptr)
            colour = *textColourToUse;

        auto gutter = r.removeFromLeft (ui::px (10));
        r.removeFromLeft (ui::px (7));

        if (icon != nullptr)
        {
            icon->drawWithin (g, gutter.toFloat(),
                              juce::RectanglePlacement::centred
                                  | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
        }
        else if (isTicked)
        {
            juce::Path tick;
            tick.startNewSubPath (0.0f, 0.45f);
            tick.lineTo (0.38f, 0.82f);
            tick.lineTo (1.0f, 0.12f);

            const auto box = gutter.reduced (0, ui::px (1)).toFloat();
            const auto transform = tick.getTransformToScaleToFit (box, true);

            g.setColour (colour);
            g.strokePath (tick,
                          juce::PathStrokeType (1.6f,
                                                juce::PathStrokeType::mitered,
                                                juce::PathStrokeType::rounded),
                          transform);
        }

        if (hasSubMenu)
        {
            auto arrow = r.removeFromRight (ui::px (12)).toFloat();
            const auto cx = arrow.getCentreX();
            const auto cy = arrow.getCentreY();
            const auto w = static_cast<float> (ui::px (3));

            juce::Path chevron;
            chevron.startNewSubPath (cx - w, cy - w);
            chevron.lineTo (cx, cy);
            chevron.lineTo (cx - w, cy + w);

            g.setColour (colour);
            g.strokePath (chevron, juce::PathStrokeType (1.4f));
        }

        r.removeFromRight (ui::px (8));

        g.setColour (colour);
        g.setFont (getPopupMenuFont());
        g.drawFittedText (text, r, juce::Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            auto shortcut = r.removeFromRight (ui::px (60));
            g.setColour (colour.withAlpha (0.7f));
            g.drawFittedText (shortcutKeyText, shortcut,
                              juce::Justification::centredRight, 1);
        }
    }

    void getIdealPopupMenuItemSize (const juce::String& text,
                                    bool isSeparator,
                                    int standardMenuItemHeight,
                                    int& idealWidth,
                                    int& idealHeight) override
    {
        juce::LookAndFeel_V4::getIdealPopupMenuItemSize (text, isSeparator,
                                                         standardMenuItemHeight,
                                                         idealWidth, idealHeight);

        if (isSeparator)
        {
            idealHeight = ui::px (9);
            return;
        }

        idealHeight = standardMenuItemHeight > 0
                          ? standardMenuItemHeight
                          : juce::jmax (ui::px (22), idealHeight);

        idealWidth += ui::px (8);
    }

    // ------------------------------------------------------------------------
    void drawComboBox (juce::Graphics& g,
                       int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override
    {
        juce::ignoreUnused (buttonX, buttonY, buttonW, buttonH);

        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);

        auto background = box.findColour (juce::ComboBox::backgroundColourId);
        if (isButtonDown)
            background = background.brighter (0.08f);

        g.setColour (background);
        g.fillRoundedRectangle (bounds, 5.0f);

        g.setColour (box.findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (bounds, 5.0f, 1.0f);

        const auto cx = static_cast<float> (width - 14);
        const auto cy = static_cast<float> (height) * 0.5f;

        juce::Path arrow;
        arrow.addTriangle (cx - 4.0f, cy - 2.0f,
                           cx + 4.0f, cy - 2.0f,
                           cx,        cy + 3.0f);

        g.setColour (box.findColour (juce::ComboBox::arrowColourId));
        g.fillPath (arrow);
    }

    // ------------------------------------------------------------------------
    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

        auto colour = backgroundColour;
        if (shouldDrawButtonAsDown)
            colour = colour.brighter (0.14f);
        else if (shouldDrawButtonAsHighlighted)
            colour = colour.brighter (0.07f);

        g.setColour (colour);
        g.fillRoundedRectangle (bounds, 5.0f);

        g.setColour (button.findColour (juce::TextButton::buttonOnColourId));
        g.drawRoundedRectangle (bounds, 5.0f, 1.0f);
    }

    // ------------------------------------------------------------------------
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
    {
        const auto byHeight = static_cast<float> (buttonHeight) * 0.55f;
        const auto preferred = ui::font (13.0f);
        return preferred.withHeight (juce::jmin (preferred.getHeight(), byHeight));
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return ui::font (12.0f);
    }

    // Valores de los knobs: mas compactos para que quepan en celdas estrechas.
    juce::Label* createSliderTextBox (juce::Slider& slider) override
    {
        auto* label = juce::LookAndFeel_V4::createSliderTextBox (slider);
        const auto fontSize = slider.getSliderStyle() == juce::Slider::LinearVertical
                                  ? 10.0f
                                  : 11.0f;
        label->setFont (ui::font (fontSize));
        label->setJustificationType (juce::Justification::centred);
        return label;
    }

private:
    ThemeId theme_ = ThemeId::holoAccents;
    ThemeColours colours_ = themeColours (ThemeId::holoAccents);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AeonixLookAndFeel)
};

} // namespace aeonix
