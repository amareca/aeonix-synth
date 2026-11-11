#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "presentation/theme/Theme.h"

namespace aeonix
{

// ============================================================================
//  PerformanceWheels — pitch bend (con muelle) y rueda de modulacion.
//
//  Vista pura: emite los valores al exterior y refleja los que llegan del
//  MIDI sin reenviarlos. El orden de las hojas (label/slider por rueda) se
//  conserva para el informe de layout F11.
// ============================================================================
class PerformanceWheels final : public juce::Component
{
public:
    static constexpr int preferredWidth = 94;

    PerformanceWheels();

    std::function<void (float)> onPitchBend;
    std::function<void (float)> onModWheel;

    // Centra el pitch bend y baja la modulacion (enviando notificacion).
    void reset();

    // Refleja un valor venido del MIDI sin reenviarlo.
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
