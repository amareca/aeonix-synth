#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "presentation/core/I18n.h"
#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"
#include "presentation/widgets/IconButton.h"

namespace aeonix
{

// ============================================================================
//  PresetBar — barra de presets de una sola fila.
//
//  Etiqueta, selector, anterior, siguiente, carpeta del gestor y LED de
//  cambios. Buscar, guardar, renombrar, borrar y crear viven en el overlay.
// ============================================================================
class PresetBar final : public juce::Component
{
public:
    static constexpr int preferredWidth  = 492;

    static int preferredHeight() { return ui::presetBarHeight(); }

    PresetBar();

    // Intenciones de usuario.
    std::function<void (int)> onSelectIndex;
    std::function<void()> onPrev;
    std::function<void()> onNext;
    std::function<void()> onOpenFolder;

    void setEntries (const juce::StringArray& names, int selectedIndex);
    void setSelectedIndex (int index,
                           juce::NotificationType notification = juce::sendNotificationSync);
    [[nodiscard]] int getSelectedIndex() const;
    [[nodiscard]] juce::String getSelectedName() const;

    // LED de cambios sin guardar.
    void setModified (bool modified);
    [[nodiscard]] bool isModified() const noexcept { return modified_; }

    void setTheme (ThemeId theme);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label    presetLabel_;
    juce::ComboBox presetCombo_;
    juce::TextButton prevButton_;
    juce::TextButton nextButton_;
    IconButton     openPresetsButton_ { IconType::folder,
                                        tr (TextId::tooltipOpenPresetManager) };

    juce::Rectangle<int> ledBounds_;
    ThemeId theme_ = ThemeId::holoAccents;
    bool modified_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};

} // namespace aeonix
