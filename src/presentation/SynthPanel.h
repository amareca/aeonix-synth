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
//  SynthPanel - the main view of the synthesizer.
//
//  MVVM: the sliders/combo talk to their ViewModels (never to the engine).
//  It also contains the preset browser (list + save) and the keyboard.
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

    // Computer keyboard octave change (MainComponent receives it).
    std::function<void (int)> onOctaveChanged;

    // Polyphony change (8/16/32 voices), handled by MainComponent.
    std::function<void (int)> onPolyphonyChanged;

    // Fired when a preset is loaded (to persist the last preset).
    std::function<void (const juce::String&)> onPresetLoaded;

    // Audition from the manager: loads without marking "Recientes" or the last preset.
    std::function<void (const juce::String&)> onPresetPreviewed;

    // Opens the user preset folder in the file explorer.
    std::function<void()> onOpenPresetsFolder;
    std::function<void()> onRequestSaveCopy;

    // Brief notice to the user (MainComponent shows it as a Toast).
    std::function<void (const juce::String&)> onPresetToast;

    // On-screen performance controls (pitch bend and mod wheel).
    std::function<void (float)> onPitchBend;
    std::function<void (float)> onModWheel;

    // Re-centres the pitch bend and lowers the modulation wheel.
    void resetPerformanceControls();

    // Mirrors a MIDI-driven movement on the wheels (without forwarding it).
    void setPitchBendDisplay (float value);
    void setModWheelDisplay (float value);

    // Sets the octave without triggering onOctaveChanged (when restoring the config).
    void setOctave (int octave);

    // Sets the polyphony without triggering onPolyphonyChanged (when restoring the config).
    void setPolyphony (int voices);

    // State of the unsaved-changes indicator.
    [[nodiscard]] bool isPresetModified() const;

    // Selects and loads a preset by name (when restoring the config).
    void selectPresetByName (const juce::String& name, bool countAsRecent = true);

    // Load requested by the preset manager; respects the changes guard.
    // countAsRecent=false is an audition: it does not mark "Recientes".
    void requestLoadPreset (const juce::String& name, bool countAsRecent = true);

    // Current catalog, for the preset manager.
    [[nodiscard]] const std::vector<PresetEntry>& presetEntries() const noexcept
    {
        return presetCatalog_.entries();
    }

    [[nodiscard]] juce::String currentPresetName() const { return presetBar_.getSelectedName(); }

    // Preset favourite (library metadata).
    void setPresetFavorite (const juce::String& name, bool favorite);

    // Preset manager actions.
    bool saveCurrentPreset();
    bool savePresetCopy (const juce::String& name, PresetCategory category);
    bool renamePreset (const juce::String& oldName, const juce::String& newName);
    bool deletePreset (const juce::String& name);

    // Saves the current preset (Ctrl+S or Enter in the name editor).
    void savePreset();

    // Visual indicator of unsaved changes in the preset.
    void setPresetModified (bool modified);

    // Visual theme (Holo Accents / Glass / Minimal / Prototipo). The change is
    // made from Ajustes > Apariencia.
    void setTheme (ThemeId theme);

    // Layout debug report (F11): bounds, overlaps and clipped texts.
    [[nodiscard]] juce::String createLayoutReport() const;

    // Physical keyboard feedback (message thread).
    void highlightNoteOn (int note, float velocity);
    void highlightNoteOff (int note);
    void clearAllNotes();

private:
    // juce::MidiKeyboardState::Listener (clicks on the virtual keyboard).
    void handleNoteOn  (juce::MidiKeyboardState*, int, int note, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int, int note, float) override;

    // --- use cases (injected) ---
    std::function<void (const std::string&, float)> setParameter_;
    std::function<void (int, float)> noteOn_;
    std::function<void (int)> noteOff_;

    // --- infrastructure ---
    PresetRepository& presetRepository_;

    // --- viewmodels (declared before the views so that the
    //     subscriptions are destroyed first) ---
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

    // --- section views (each one is its own SectionPanel) ---
    OscillatorView oscView_       { oscVm_ };
    FilterView     filterView_    { filterVm_, filterEnvVm_ };
    FilterEnvView  filterEnvView_ { filterEnvVm_ };
    AmpEnvView     ampEnvView_    { envVm_ };
    LfoView        lfoView_       { lfoVm_ };
    UnisonView     unisonView_    { unisonVm_ };
    DelayView      delayView_     { fxVm_ };
    ReverbView     reverbView_    { fxVm_ };
    MasterView     masterView_    { synthVm_ };

    // --- presets (view + controller) ---
    PresetBar presetBar_;
    PresetCatalog presetCatalog_ { presetRepository_ };
    std::unique_ptr<PresetController> presetController_;

    // --- performance wheels and keyboard (widgets) ---
    PerformanceWheels performanceWheels_;

    // --- tooltips ---
    std::unique_ptr<juce::TooltipWindow> tooltipWindow_;

    // --- virtual keyboard ---
    juce::MidiKeyboardState keyboardState_;
    KeyboardBed keyboardBed_ { keyboardState_ };

    bool applyingFeedback_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthPanel)
};

} // namespace aeonix
