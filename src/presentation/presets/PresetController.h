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
//  PresetController — reglas de interaccion del navegador de presets.
//
//  Conecta PresetBar con PresetCatalog/PatchBinder y gestiona los dialogos
//  asincronos (sobrescribir, borrar, renombrar). La vista no conoce el
//  repositorio y el controlador no conoce el motor.
// ============================================================================
class PresetController
{
public:
    PresetController (PresetBar& bar,
                      PresetCatalog& catalog,
                      PatchBinder& binder,
                      juce::Component& dialogParent,
                      std::function<void (const juce::String&)> onPresetLoaded,
                      std::function<bool()> isDirty,
                      std::function<void (const juce::String&)> notify);
    ~PresetController();

    PresetController (const PresetController&) = delete;
    PresetController& operator= (const PresetController&) = delete;

    // Reconstruye la lista (fabrica + usuario) y conserva/forza la seleccion.
    void refresh (const juce::String& selectName = {});

    // Selecciona y carga un preset por nombre (al restaurar la config).
    void selectByName (const juce::String& name, bool countAsRecent = true);

    // Carga pedida por el gestor: respeta la guardia de cambios.
    void requestLoadByName (const juce::String& name);

    // Acciones del gestor de presets (sin formularios propios).
    bool saveCurrentPreset();
    bool savePresetCopy (const juce::String& name, PresetCategory category);
    bool renamePreset (const juce::String& oldName, const juce::String& newName);
    bool deletePreset (const juce::String& name);

    // Tema para los dialogos (se sincroniza desde SynthPanel).
    void setTheme (ThemeId theme) { theme_ = theme; }

    // Guardar copia desde la guardia de cambios de un preset de fabrica.
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

    // Aviso breve al usuario (un Toast en MainComponent).
    void notify (const juce::String& message);

    PresetBar& bar_;
    PresetCatalog& catalog_;
    PatchBinder& binder_;
    juce::Component& dialogParent_;
    std::function<void (const juce::String&)> onPresetLoaded_;
    std::function<bool()> isDirty_;
    std::function<void (const juce::String&)> notify_;
    juce::String loadedName_;
    ThemeId theme_ = ThemeId::holoAccents;
    juce::Component::SafePointer<ThemedDialog> activeDialog_;
};

} // namespace aeonix
