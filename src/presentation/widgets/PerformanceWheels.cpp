#include "PerformanceWheels.h"

#include "presentation/core/Utf8.h"
#include "presentation/theme/Typography.h"

namespace aeonix
{

PerformanceWheels::PerformanceWheels()
{
    setTitle (tr (TextId::wheelsTitle));
    setDescription (tr (TextId::wheelsDescription));

    // ---- pitch bend ----
    pitchBendLabel_.setText (tr (TextId::labelPitch), juce::dontSendNotification);
    pitchBendLabel_.setJustificationType (juce::Justification::centred);
    pitchBendLabel_.setFont (ui::font (10.0f, juce::Font::bold));
    pitchBendLabel_.setName ("Pitch");
    addAndMakeVisible (pitchBendLabel_);

    pitchBendSlider_.setName ("Pitch");
    pitchBendSlider_.setSliderStyle (juce::Slider::LinearVertical);
    pitchBendSlider_.setRange (-1.0, 1.0, 0.01);
    pitchBendSlider_.setValue (0.0, juce::dontSendNotification);
    pitchBendSlider_.setTextBoxStyle (juce::Slider::TextBoxBelow, false,
                                      ui::px (36), ui::valueBoxHeight());
    pitchBendSlider_.setDoubleClickReturnValue (true, 0.0);
    pitchBendSlider_.setColour (juce::Slider::trackColourId, juce::Colour (0xff5aa9e6));
    pitchBendSlider_.setColour (juce::Slider::thumbColourId, juce::Colour (0xff5aa9e6));
    pitchBendSlider_.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2c333d));
    pitchBendSlider_.setTooltip (tr (TextId::tooltipPitchBend));
    pitchBendSlider_.textFromValueFunction = [] (double value)
    {
        const int semitones = juce::roundToInt (value * 2.0);
        return (semitones > 0 ? "+" : "") + juce::String (semitones) + " st";
    };
    pitchBendSlider_.onValueChange = [this]
    {
        if (onPitchBend)
            onPitchBend (static_cast<float> (pitchBendSlider_.getValue()));
    };
    pitchBendSlider_.onDragEnd = [this]
    {
        // Muelle: al soltar, vuelve al centro.
        pitchBendSlider_.setValue (0.0, juce::sendNotificationSync);
    };
    addAndMakeVisible (pitchBendSlider_);

    // ---- rueda de modulacion ----
    modWheelLabel_.setText (tr (TextId::labelMod), juce::dontSendNotification);
    modWheelLabel_.setJustificationType (juce::Justification::centred);
    modWheelLabel_.setFont (ui::font (10.0f, juce::Font::bold));
    modWheelLabel_.setName ("Mod");
    addAndMakeVisible (modWheelLabel_);

    modWheelSlider_.setName ("Mod");
    modWheelSlider_.setSliderStyle (juce::Slider::LinearVertical);
    modWheelSlider_.setRange (0.0, 1.0, 0.01);
    modWheelSlider_.setValue (0.0, juce::dontSendNotification);
    modWheelSlider_.setTextBoxStyle (juce::Slider::TextBoxBelow, false,
                                     ui::px (36), ui::valueBoxHeight());
    modWheelSlider_.setDoubleClickReturnValue (true, 0.0);
    modWheelSlider_.setColour (juce::Slider::trackColourId, juce::Colour (0xffa78bfa));
    modWheelSlider_.setColour (juce::Slider::thumbColourId, juce::Colour (0xffa78bfa));
    modWheelSlider_.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2c333d));
    modWheelSlider_.setTooltip (tr (TextId::tooltipModWheel));
    modWheelSlider_.textFromValueFunction = [] (double value)
    {
        return juce::String (juce::roundToInt (value * 100.0)) + "%";
    };
    modWheelSlider_.onValueChange = [this]
    {
        if (onModWheel)
            onModWheel (static_cast<float> (modWheelSlider_.getValue()));
    };
    addAndMakeVisible (modWheelSlider_);

    pitchBendSlider_.updateText();
    modWheelSlider_.updateText();
}

void PerformanceWheels::reset()
{
    pitchBendSlider_.setValue (0.0, juce::sendNotificationSync);
    modWheelSlider_.setValue (0.0, juce::sendNotificationSync);
}

void PerformanceWheels::setPitchBendDisplay (float value)
{
    pitchBendSlider_.setValue (juce::jlimit (-1.0f, 1.0f, value),
                               juce::dontSendNotification);
}

void PerformanceWheels::setModWheelDisplay (float value)
{
    modWheelSlider_.setValue (juce::jlimit (0.0f, 1.0f, value),
                              juce::dontSendNotification);
}

void PerformanceWheels::setTheme (ThemeId theme)
{
    const auto colours = themeColours (theme);
    const auto pitchColour = theme == ThemeId::prototype
                                 ? juce::Colour (0xff5aa9e6)
                                 : colours.accent;
    const auto modColour = theme == ThemeId::prototype
                               ? juce::Colour (0xffa78bfa)
                               : holoColour (0.65f);
    const auto wheelBackground = theme == ThemeId::prototype
                                     ? juce::Colour (0xff2c333d)
                                     : colours.panel;

    pitchBendSlider_.setColour (juce::Slider::trackColourId, pitchColour);
    pitchBendSlider_.setColour (juce::Slider::thumbColourId, pitchColour);
    pitchBendSlider_.setColour (juce::Slider::backgroundColourId, wheelBackground);

    modWheelSlider_.setColour (juce::Slider::trackColourId, modColour);
    modWheelSlider_.setColour (juce::Slider::thumbColourId, modColour);
    modWheelSlider_.setColour (juce::Slider::backgroundColourId, wheelBackground);

    repaint();
}

void PerformanceWheels::resized()
{
    auto area = getLocalBounds();

    auto pitchCell = area.removeFromLeft (ui::px (45));
    pitchBendLabel_.setBounds (pitchCell.removeFromTop (ui::labelHeight()));
    pitchBendSlider_.setBounds (pitchCell.reduced (2, 0));

    area.removeFromLeft (ui::px (4));

    auto modCell = area;
    modWheelLabel_.setBounds (modCell.removeFromTop (ui::labelHeight()));
    modWheelSlider_.setBounds (modCell.reduced (2, 0));
}

} // namespace aeonix
