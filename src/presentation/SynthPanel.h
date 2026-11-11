#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <functional>
#include <memory>
#include <string>

#include "presentation/theme/Theme.h"
#include "infrastructure/PresetRepository.h"
#include "presentation/patch/PatchBinder.h"
#include "presentation/presets/PresetBar.h"
#include "presentation/presets/PresetCatalog.h"
#include "presentation/presets/PresetController.h"
#include "presentation/sections/EffectViews.h"
#include "presentation/sections/MasterView.h"
#include "presentation/sections/ModulationViews.h"
#include "presentation/sections/SynthesisViews.h"
#include "presentation/keyboard/KeyboardBed.h"
#include "presentation/widgets/PerformanceWheels.h"

namespace aeonix
{

// ============================================================================
//  SynthPanel — la vista principal del sintetizador.
//
//  MVVM: los sliders/combo hablan con sus ViewModels (nunca con el motor).
//  Ademas contiene el navegador de presets (lista + guardar) y el teclado.
// ============================================================================
class SynthPanel : public juce::Component,
                   private juce::MidiKeyboardState::Listener
{
public:
    SynthPanel (std::function<void (const std::string&, float)> setParameter,
                std::function<void (int, float)> noteOn,
                std::function<void (int)> noteOff,
                PresetRepository& presetRepository);
    ~SynthPanel() override;

    void resized() override;

    // Cambio de octava del teclado del ordenador (lo recibe MainComponent).
    std::function<void (int)> onOctaveChanged;

    // Cambio de polifonía (8/16/32 voces), gestionado por MainComponent.
    std::function<void (int)> onPolyphonyChanged;

    // Se dispara al cargar un preset (para persistir el ultimo preset).
    std::function<void (const juce::String&)> onPresetLoaded;

    // Abre la carpeta de presets de usuario en el explorador.
    std::function<void()> onOpenPresetsFolder;
    std::function<void()> onRequestSaveCopy;

    // Aviso breve al usuario (lo muestra MainComponent como Toast).
    std::function<void (const juce::String&)> onPresetToast;

    // Controles de performance en pantalla (pitch bend y mod wheel).
    std::function<void (float)> onPitchBend;
    std::function<void (float)> onModWheel;

    // Vuelve a centrar el pitch bend y bajar la rueda de modulacion.
    void resetPerformanceControls();

    // Refleja en las ruedas un movimiento venido del MIDI (sin reenviarlo).
    void setPitchBendDisplay (float value);
    void setModWheelDisplay (float value);

    // Fija la octava sin disparar onOctaveChanged (al restaurar la config).
    void setOctave (int octave);

    // Fija la polifonía sin disparar onPolyphonyChanged (al restaurar la config).
    void setPolyphony (int voices);

    // Estado del indicador de cambios sin guardar.
    [[nodiscard]] bool isPresetModified() const;

    // Selecciona y carga un preset por nombre (al restaurar la config).
    void selectPresetByName (const juce::String& name, bool countAsRecent = true);

    // Carga pedida por el gestor de presets; respeta la guardia de cambios.
    void requestLoadPreset (const juce::String& name);

    // Catalogo actual, para el gestor de presets.
    [[nodiscard]] const std::vector<PresetEntry>& presetEntries() const noexcept
    {
        return presetCatalog_.entries();
    }

    [[nodiscard]] juce::String currentPresetName() const { return presetBar_.getSelectedName(); }

    // Favorito del preset (metadatos de la biblioteca).
    void setPresetFavorite (const juce::String& name, bool favorite);

    // Acciones del gestor de presets.
    bool saveCurrentPreset();
    bool savePresetCopy (const juce::String& name, PresetCategory category);
    bool renamePreset (const juce::String& oldName, const juce::String& newName);
    bool deletePreset (const juce::String& name);

    // Guarda el preset actual (Ctrl+S o Enter en el editor de nombre).
    void savePreset();

    // Indicador visual de cambios sin guardar en el preset.
    void setPresetModified (bool modified);

    // Tema visual (Holo Accents / Glass / Minimal / Prototipo). El cambio se
    // hace desde Ajustes › Apariencia.
    void setTheme (ThemeId theme);

    // Informe de depuracion del layout (F11): bounds, solapes y textos cortados.
    [[nodiscard]] juce::String createLayoutReport() const;

    // Feedback del teclado fisico (hilo de mensajes).
    void highlightNoteOn (int note, float velocity);
    void highlightNoteOff (int note);
    void clearAllNotes();

private:
    // juce::MidiKeyboardState::Listener (clicks en el teclado virtual).
    void handleNoteOn  (juce::MidiKeyboardState*, int, int note, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int, int note, float) override;

    // --- casos de uso (inyectados) ---
    std::function<void (const std::string&, float)> setParameter_;
    std::function<void (int, float)> noteOn_;
    std::function<void (int)> noteOff_;

    // --- infraestructura ---
    PresetRepository& presetRepository_;

    // --- viewmodels (se declaran antes que las vistas para que las
    //     suscripciones se destruyan primero) ---
    SynthViewModels viewModels_;
    OscillatorViewModel& oscVm_ = viewModels_.osc;
    FilterViewModel&     filterVm_ = viewModels_.filter;
    FilterEnvViewModel&  filterEnvVm_ = viewModels_.filterEnv;
    EnvelopeViewModel&   envVm_ = viewModels_.amp;
    SynthViewModel&      synthVm_ = viewModels_.synth;
    FxViewModel&         fxVm_ = viewModels_.fx;
    LfoViewModel&        lfoVm_ = viewModels_.lfo;
    UnisonViewModel&     unisonVm_ = viewModels_.unison;
    PatchBinder          patchBinder_ { viewModels_ };

    // --- vistas de seccion (cada una es su propio SectionPanel) ---
    OscillatorView oscView_       { oscVm_ };
    FilterView     filterView_    { filterVm_, filterEnvVm_ };
    FilterEnvView  filterEnvView_ { filterEnvVm_ };
    AmpEnvView     ampEnvView_    { envVm_ };
    LfoView        lfoView_       { lfoVm_ };
    UnisonView     unisonView_    { unisonVm_ };
    DelayView      delayView_     { fxVm_ };
    ReverbView     reverbView_    { fxVm_ };
    MasterView     masterView_    { synthVm_ };

    // --- presets (vista + controlador) ---
    PresetBar presetBar_;
    PresetCatalog presetCatalog_ { presetRepository_ };
    std::unique_ptr<PresetController> presetController_;

    // --- ruedas de performance y teclado (widgets) ---
    PerformanceWheels performanceWheels_;

    // --- tooltips ---
    std::unique_ptr<juce::TooltipWindow> tooltipWindow_;

    // --- teclado virtual ---
    juce::MidiKeyboardState keyboardState_;
    KeyboardBed keyboardBed_ { keyboardState_ };

    bool applyingFeedback_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthPanel)
};

} // namespace aeonix
