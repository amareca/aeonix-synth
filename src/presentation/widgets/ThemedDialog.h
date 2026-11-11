#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "presentation/theme/Theme.h"

namespace aeonix
{

struct ThemedDialogButton
{
    juce::String text;
    int result = 0;
    bool destructive = false;
};

// ============================================================================
//  ThemedDialog — dialogo modal con la estetica de la app.
//
//  Scrim + tarjeta centrada sobre el contenido de la ventana, con titulo,
//  mensaje y botones. Sustituye a juw::AlertWindow en la capa de presentacion.
//
//  Los botones se colocan en el orden recibido, de izquierda a derecha:
//  para acciones destructivas se pasa primero la accion y luego Cancelar.
//  Enter y el clic en el scrim equivalen a Cancelar.
// ============================================================================
class ThemedDialog final : public juce::Component
{
public:
    enum class Style
    {
        info,
        warning,
        danger
    };

    static ThemedDialog* show (juce::Component& dialogParent,
                               ThemeId theme,
                               Style style,
                               juce::String title,
                               juce::String message,
                               std::vector<ThemedDialogButton> buttons,
                               std::function<void (int)> onResult);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    bool keyPressed (const juce::KeyPress& key) override;
    void parentSizeChanged() override;

private:
    ThemedDialog (ThemeId theme,
                  Style style,
                  juce::String title,
                  juce::String message,
                  std::vector<ThemedDialogButton> buttons,
                  std::function<void (int)> onResult);

    struct ButtonEntry
    {
        std::unique_ptr<juce::TextButton> button;
        int result = 0;
    };

    void close (int result);
    void applyThemeToButton (juce::TextButton& button, bool destructive) const;

    ThemeId theme_ = ThemeId::holoAccents;
    Style style_ = Style::info;
    juce::String title_;
    juce::String message_;
    std::vector<ButtonEntry> buttons_;
    std::function<void (int)> onResult_;
    juce::Rectangle<int> panelBounds_;
    int cancelResult_ = 0;
    bool closed_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThemedDialog)
};

} // namespace aeonix
