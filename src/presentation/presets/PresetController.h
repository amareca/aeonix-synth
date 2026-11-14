#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "presentation/patch/PatchBinder.h"
#include "presentation/presets/PresetBar.h"
#include "presentation/presets/PresetCatalog.h"
#include "presentation/widgets/ThemedDialog.h"

namespace aeonix
{

// ============================================================================
//  PresetController - interaction rules for the preset browser.
//
//  Connects PresetBar with PresetCatalog/PatchBinder and manages the
//  asynchronous dialogs (overwrite, delete, rename). The view knows nothing
//  about the repository and the controller knows nothing about the engine.
// ============================================================================
class PresetController
{
public:
    PresetController (PresetBar& bar,
                      PresetCatalog& catalog,
                      PatchBinder& binder,
                      juce::Component& dialogParent,
                      std::function<void (const juce::String&)> onPresetLoaded,
                      std::function<void (const juce::String&)> onPresetPreviewed,
                      std::function<bool()> isDirty,
                      std::function<void (const juce::String&)> notify);
    ~PresetController();

    PresetController (const PresetController&) = delete;
    PresetController& operator= (const PresetController&) = delete;

    // Rebuilds the list (factory + user) and keeps/forces the selection.
    void refresh (const juce::String& selectName = {});

    // Selects and loads a preset by name (when restoring the config).
    void selectByName (const juce::String& name, bool countAsRecent = true);

    // Load requested by the manager. With countAsRecent=false it is an audition:
    // it does not mark "Recientes" nor save the last preset.
    void requestLoadByName (const juce::String& name, bool countAsRecent = true);

    // Preset manager actions (without their own forms).
    bool saveCurrentPreset();
    bool savePresetCopy (const juce::String& name, PresetCategory category);
    bool renamePreset (const juce::String& oldName, const juce::String& newName);
    bool deletePreset (const juce::String& name);

    // Theme for the dialogs (synchronised from SynthPanel).
    void setTheme (ThemeId theme) { theme_ = theme; }

    // Save copy from the guard for changes of a factory preset.
    std::function<void()> onRequestSaveCopy;

private:
    void requestLoadByIndex (int index, bool forceReload = false, bool countAsRecent = true);
    void applyLoad (const juce::String& name, int index, bool countAsRecent = true);
    void restoreSelection();
    void step (int delta);
    [[nodiscard]] int indexOf (const juce::String& name) const;

    void showDialog (ThemedDialog::Style style,
                     juce::String title,
                     juce::String message,
                     std::vector<ThemedDialogButton> buttons,
                     std::function<void (int)> onResult);

    // Brief notice to the user (a Toast in MainComponent).
    void notify (const juce::String& message);

    PresetBar& bar_;
    PresetCatalog& catalog_;
    PatchBinder& binder_;
    juce::Component& dialogParent_;
    std::function<void (const juce::String&)> onPresetLoaded_;
    std::function<void (const juce::String&)> onPresetPreviewed_;
    std::function<bool()> isDirty_;
    std::function<void (const juce::String&)> notify_;
    juce::String loadedName_;
    ThemeId theme_ = ThemeId::holoAccents;
    juce::Component::SafePointer<ThemedDialog> activeDialog_;
};

} // namespace aeonix
