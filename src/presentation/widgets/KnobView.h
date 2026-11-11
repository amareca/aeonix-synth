#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <string>
#include <utility>

#include "domain/ParameterFormat.h"
#include "presentation/core/Observable.h"
#include "presentation/core/Utf8.h"
#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"
#include "presentation/patch/ParameterMetadata.h"

namespace aeonix
{

// ============================================================================
//  KnobView — etiqueta + slider rotativo con formato de valor real.
//
//  No conoce ViewModels: recibe rango, formato y un callback de cambio. Las
//  clases ParameterKnob/DiscreteKnob anaden el binding observable.
// ============================================================================
class KnobView : public juce::Component
{
public:
    KnobView (juce::String title,
              juce::String tooltip,
              double minValue, double maxValue, double interval,
              std::function<juce::String (double)> formatter,
              juce::Colour accent,
              int textBoxWidth = 52,
              bool inheritLabelColour = false)
    {
        label_.setText (title, juce::dontSendNotification);
        label_.setJustificationType (juce::Justification::centred);
        label_.setFont (ui::font (11.0f));
        label_.setName (title);

        if (! inheritLabelColour)
            label_.setColour (juce::Label::textColourId, juce::Colour (0xffc8ced8));

        addAndMakeVisible (label_);

        slider_.setName (title);
        slider_.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider_.setRange (minValue, maxValue, interval);
        slider_.setTextBoxStyle (juce::Slider::TextBoxBelow, false,
                                 ui::px (textBoxWidth), ui::valueBoxHeight());
        slider_.setTooltip (tooltip);
        slider_.setColour (juce::Slider::rotarySliderFillColourId, accent);
        slider_.setColour (juce::Slider::rotarySliderOutlineColourId,
                           juce::Colour (0xff3a414d));
        slider_.setColour (juce::Slider::thumbColourId, accent);
        slider_.textFromValueFunction = [formatter = std::move (formatter)] (double value)
        {
            return formatter (value);
        };
        slider_.onValueChange = [this]
        {
            if (onValueChange)
                onValueChange (slider_.getValue());
        };
        addAndMakeVisible (slider_);
    }

    std::function<void (double)> onValueChange;

    void setValue (double value,
                   juce::NotificationType notification = juce::dontSendNotification)
    {
        slider_.setValue (value, notification);
        slider_.updateText();
    }

    [[nodiscard]] double getValue() const { return slider_.getValue(); }

    juce::Slider& getSlider() noexcept { return slider_; }
    juce::Label& getLabel() noexcept { return label_; }

    void setTheme (ThemeId theme)
    {
        juce::ignoreUnused (theme);
        repaint();
    }

    void resized() override
    {
        auto area = getLocalBounds();
        label_.setBounds (area.removeFromTop (ui::labelHeight()));
        slider_.setBounds (area.reduced (4, 0));
    }

private:
    juce::Label label_;
    juce::Slider slider_;
};

// ============================================================================
//  ParameterKnob — knob de un parametro normalizado (0..1) con formato real.
// ============================================================================
class ParameterKnob final : public KnobView
{
public:
    ParameterKnob (const ParameterUiInfo& info,
                   Observable<float>& value,
                   std::function<void (float)> setter,
                   juce::Colour accent)
        : KnobView (info.label, info.tooltip, 0.0, 1.0, 0.001,
                    [id = std::string (info.id)] (double v)
                    {
                        return juce::String (formatParameterValue (id,
                                                                  static_cast<float> (v)));
                    },
                    accent),
          subscription_ (value.subscribe ([this] (const float& n)
          {
              setValue (static_cast<double> (n), juce::dontSendNotification);
          }))
    {
        setValue (static_cast<double> (value.get()), juce::dontSendNotification);
        onValueChange = [setter = std::move (setter)] (double v)
        {
            setter (static_cast<float> (v));
        };
    }

private:
    Observable<float>::Subscription subscription_;
};

// ============================================================================
//  DiscreteKnob — knob de un valor entero (p. ej. voces de unison).
// ============================================================================
class DiscreteKnob final : public KnobView
{
public:
    DiscreteKnob (juce::String title,
                  juce::String tooltip,
                  int minValue, int maxValue,
                  std::function<juce::String (int)> formatter,
                  Observable<int>& value,
                  std::function<void (int)> setter,
                  juce::Colour accent,
                  int textBoxWidth = 46)
        : KnobView (title, tooltip, minValue, maxValue, 1.0,
                    [formatter = std::move (formatter)] (double v)
                    {
                        return formatter (juce::roundToInt (v));
                    },
                    accent, textBoxWidth, true),
          subscription_ (value.subscribe ([this] (const int& n)
          {
              setValue (n, juce::dontSendNotification);
          }))
    {
        setValue (value.get(), juce::dontSendNotification);
        onValueChange = [setter = std::move (setter)] (double v)
        {
            setter (juce::roundToInt (v));
        };
    }

private:
    Observable<int>::Subscription subscription_;
};

} // namespace aeonix
