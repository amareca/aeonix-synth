#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "presentation/theme/Theme.h"

namespace aeonix
{

// ============================================================================
//  PerformanceWheels - pitch bend (spring-loaded) and modulation wheel.
//
//  Pure view: emits the values outwards and reflects those coming from
//  MIDI without forwarding them. The order of the leaves (label/slider per
//  wheel) is kept for the F11 layout report.
// ============================================================================
class PerformanceWheels final : public juce::Component
{
public:
    static constexpr int preferredWidth = 94;

    PerformanceWheels();

    std::function<void (float)> onPitchBend;
    std::function<void (float)> onModWheel;

    // Centres the pitch bend and lowers the modulation (sending notification).
    void reset();

    // Reflects a value coming from MIDI without forwarding it.
    void setPitchBendDisplay (float value);
    void setModWheelDisplay (float value);

    void setTheme (ThemeId theme);

    void resized() override;

private:
    juce::Label  pitchBendLabel_;
    juce::Slider pitchBendSlider_;
    juce::Label  modWheelLabel_;
    juce::Slider modWheelSlider_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerformanceWheels)
};

} // namespace aeonix
