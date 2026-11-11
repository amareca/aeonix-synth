#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "presentation/keyboard/AeonixKeyboard.h"
#include "presentation/theme/Theme.h"

namespace aeonix
{

// ============================================================================
//  KeyboardBed — marco, fieltro y teclado "Studio Ivory".
//
//  El componente ocupa el marco completo y coloca el teclado dentro con el
//  mismo margen que antes. Incluye 1 px extra abajo para que la sombra del
//  marco no quede recortada por los limites del componente.
// ============================================================================
class KeyboardBed final : public juce::Component
{
public:
    // Margen del marco respecto a las teclas. El +1 evita que el trazo
    // centrado del borde quede recortado por los limites del componente.
    static constexpr int frameLeft   = 7;
    static constexpr int frameTop    = 9;
    static constexpr int frameRight  = 7;
    static constexpr int frameBottom = 10;   // marco (6) + sombra (3) + holgura

    explicit KeyboardBed (juce::MidiKeyboardState& state);

    [[nodiscard]] AeonixKeyboard& getKeyboard() noexcept { return keyboard_; }
    [[nodiscard]] const AeonixKeyboard& getKeyboard() const noexcept { return keyboard_; }

    void setTheme (ThemeId theme);

    // Convierte los bounds del teclado en los del marco que lo contiene.
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
