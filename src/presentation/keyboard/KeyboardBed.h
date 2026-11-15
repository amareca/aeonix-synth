#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "presentation/keyboard/AeonixKeyboard.h"
#include "presentation/theme/Theme.h"

namespace aeonix
{

// ============================================================================
//  KeyboardBed - frame, felt and "Studio Ivory" keyboard.
//
//  The component fills the whole frame and places the keyboard inside it with
//  the same margin as before. It adds 1 extra px at the bottom so the frame
//  shadow is not clipped by the component bounds.
// ============================================================================
class KeyboardBed final : public juce::Component
{
public:
    // Frame margin around the keys. The +1 stops the centred border stroke
    // from being clipped by the component bounds.
    static constexpr int frameLeft   = 7;
    static constexpr int frameTop    = 9;
    static constexpr int frameRight  = 7;
    static constexpr int frameBottom = 10;   // frame (6) + shadow (3) + slack

    explicit KeyboardBed (juce::MidiKeyboardState& state);

    [[nodiscard]] AeonixKeyboard& getKeyboard() noexcept { return keyboard_; }
    [[nodiscard]] const AeonixKeyboard& getKeyboard() const noexcept { return keyboard_; }

    void setTheme (ThemeId theme);

    // Converts the keyboard bounds into those of the frame that contains it.
    static juce::Rectangle<int> bedBoundsFor (juce::Rectangle<int> keys)
    {
        return { keys.getX() - frameLeft,
                 keys.getY() - frameTop,
                 keys.getWidth() + frameLeft + frameRight,
                 keys.getHeight() + frameTop + frameBottom };
    }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    ThemeId theme_ = ThemeId::holoAccents;
    AeonixKeyboard keyboard_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardBed)
};

} // namespace aeonix
