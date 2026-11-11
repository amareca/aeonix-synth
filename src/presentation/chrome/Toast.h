#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "presentation/theme/Typography.h"

namespace aeonix
{

// ============================================================================
//  Toast — aviso flotante estilo Android.
//
//  Píldora oscura centrada en la parte inferior, con fade-in, espera y
//  fade-out. No intercepta clics (se puede seguir tocando el teclado) y se
//  coloca por encima del resto de componentes.
// ============================================================================
class Toast : public juce::Component, private juce::Timer
{
public:
    Toast()
    {
        setInterceptsMouseClicks (false, false);
        setVisible (false);
    }

    void showMessage (const juce::String& message, int holdMilliseconds = 2200)
    {
        message_ = message;
        holdMs_ = holdMilliseconds;
        holdCounter_ = 0;
        alpha_ = 0.0f;
        state_ = State::fadeIn;

        updateBounds();
        setVisible (true);
        toFront (false);
        startTimerHz (30);
        repaint();
    }

    void hideToast()
    {
        stopTimer();
        state_ = State::hidden;
        alpha_ = 0.0f;
        setVisible (false);
    }

    // Recoloca la píldora según el tamaño actual del padre.
    void updateBounds()
    {
        if (auto* parent = getParentComponent())
        {
            const auto font = ui::font (13.0f);
            const auto textWidth = juce::GlyphArrangement::getStringWidth (font, message_);

            const int width = juce::jlimit (ui::px (180),
                                            juce::jmax (ui::px (180), parent->getWidth() - ui::px (40)),
                                            juce::roundToInt (textWidth) + ui::px (52));
            const int height = ui::px (40);

            setBounds ((parent->getWidth() - width) / 2,
                       parent->getHeight() - height - ui::px (28),
                       width, height);
        }
    }

    void paint (juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced (0.5f);
        const auto bg = juce::Colour (0xee20252d);

        g.setColour (bg.withMultipliedAlpha (alpha_));
        g.fillRoundedRectangle (bounds, 10.0f);

        g.setColour (juce::Colour (0xff4a5462).withMultipliedAlpha (alpha_));
        g.drawRoundedRectangle (bounds, 10.0f, 1.0f);

        // Punto de acento verde.
        g.setColour (juce::Colour (0xff7dd3a8).withMultipliedAlpha (alpha_));
        g.fillEllipse (ui::px (16), (getHeight() - ui::px (7)) * 0.5f, ui::px (7), ui::px (7));

        g.setColour (juce::Colour (0xffe6ebf2).withMultipliedAlpha (alpha_));
        g.setFont (ui::font (13.0f));
        g.drawText (message_,
                    getLocalBounds().withTrimmedLeft (ui::px (32)).withTrimmedRight (ui::px (16)),
                    juce::Justification::centredLeft,
                    true);
    }

private:
    enum class State
    {
        hidden,
        fadeIn,
        hold,
        fadeOut
    };

    void timerCallback() override
    {
        switch (state_)
        {
            case State::fadeIn:
                alpha_ += 0.14f;
                if (alpha_ >= 1.0f)
                {
                    alpha_ = 1.0f;
                    holdCounter_ = 0;
                    state_ = State::hold;
                }
                break;

            case State::hold:
                holdCounter_ += 33;   // temporizador a ~30 Hz
                if (holdCounter_ >= holdMs_)
                    state_ = State::fadeOut;
                break;

            case State::fadeOut:
                alpha_ -= 0.09f;
                if (alpha_ <= 0.0f)
                {
                    hideToast();
                    return;
                }
                break;

            case State::hidden:
                return;
        }

        repaint();
    }

    juce::String message_;
    int holdMs_ = 2200;
    int holdCounter_ = 0;
    float alpha_ = 0.0f;
    State state_ = State::hidden;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Toast)
};

} // namespace aeonix
