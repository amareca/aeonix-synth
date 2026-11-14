#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "domain/SynthEngine.h"
#include "presentation/chrome/AeonixLookAndFeel.h"
#include "presentation/chrome/SettingsOverlay.h"
#include "presentation/SynthPanel.h"
#include "presentation/core/I18n.h"
#include "presentation/core/BuildFlavour.h"
#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"
#include "presentation/chrome/TitleBar.h"
#include "presentation/chrome/Toast.h"
#include "presentation/presets/PresetManagerOverlay.h"
#include "infrastructure/ConfigManager.h"
#include "infrastructure/PresetRepository.h"

// ==============================================================================
//  MainComponent - main window (container).
//
//  - Audio thread:     synthEngine.render().
//  - MIDI thread:      translates messages to noteOn/off/sustain (lock-free queued)
//                      and queues status text + key feedback.
//  - Message thread:   paints the GUI (AsyncUpdater), NEVER from another thread.
//
//  Contains a SynthPanel (the controls + virtual keyboard) that follows the
//  MVVM pattern: the Views talk to ViewModels, not to the engine.
// ==============================================================================
class MainComponent : public juce::AudioAppComponent,
                      private juce::MidiInputCallback,
                      private juce::AsyncUpdater,
                      private juce::KeyListener
{
public:
    MainComponent();
    ~MainComponent() override;

    // ---- juce::Component ----
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;

    // Custom window: MainWindow connects these callbacks with the DocumentWindow.
    std::function<void()> onWindowMinimise;
    std::function<void()> onWindowToggleMaximise;
    std::function<void()> onWindowClose;
    std::function<void (float, juce::Point<int>)> onWindowRestoreForDrag;

    // Syncs the bar's maximize/restore icon.
    void setWindowMaximised (bool maximised);

    // ---- juce::AudioAppComponent (audio thread) ----
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // ---- juce::MidiInputCallback (MIDI background thread) ----
    void handleIncomingMidiMessage (juce::MidiInput* source,
                                    const juce::MidiMessage& message) override;

    // ---- juce::AsyncUpdater (message thread) ----
    void handleAsyncUpdate() override;

    // ---- juce::KeyListener (computer keyboard as a piano) ----
    bool keyPressed (const juce::KeyPress& key, juce::Component* origin) override;
    bool keyStateChanged (bool isKeyDown, juce::Component* origin) override;

private:
    void panic();
    void showSettings (aeonix::SettingsOverlay::Tab tab);
    void openPresetManager();
    void refreshPresetManager();
    void refreshMidiDevices (bool reopenDevice);
    void setupMidiDevices();
    void openMidiInput (int deviceIndex);

    // Visual theme.
    void applyTheme (aeonix::ThemeId theme);

    // Development tools: F12 = PNG snapshot, F11 = layout report.
#if AEONIX_HAS_DEV_TOOLS
    void saveSnapshot();
    void saveLayoutReport();
    void openSnapshotsFolder();

    // Capture root and folder of the active feature. The feature comes from
    // AEONIX_CAPTURE_TAG or from the visible context (manager, settings or main).
    [[nodiscard]] static juce::File developmentOutputDirectory();
    [[nodiscard]] juce::String captureTag() const;
    [[nodiscard]] juce::String captureFileStem() const;
    [[nodiscard]] juce::File captureDirectory() const;
    [[nodiscard]] bool captureArchiveEnabled() const;
    [[nodiscard]] bool captureMatrixEnabled() const;
    void archiveCapture (const juce::File& file) const;
    [[nodiscard]] static juce::String sanitiseCaptureTag (const juce::String& tag);
#endif
    void openPresetsFolder();

    // Feedback event for the virtual keyboard (MIDI thread -> message thread).
    struct KeyFeedback
    {
        bool noteOn = false;
        int note = 0;
        float velocity = 0.0f;
    };

    // ---- styling (must outlive the children) ----
    aeonix::AeonixLookAndFeel lookAndFeel_;
    aeonix::ThemeId theme_ = aeonix::ThemeId::holoAccents;

    // ---- audio + synthesis ----
    aeonix::ConfigManager& config_;
    aeonix::PresetRepository presetRepository;
    aeonix::SynthEngine synthEngine;
    bool presetLoaded_ = false;
    aeonix::SynthPanel synthPanel;

    // ---- interface ----
    aeonix::TitleBar titleBar_;
    aeonix::SettingsOverlay settingsOverlay_;
    std::unique_ptr<aeonix::PresetManagerOverlay> presetManagerOverlay_;

    juce::TextButton panicButton          { aeonix::tr (aeonix::TextId::panicButton) };
#if AEONIX_HAS_DEV_TOOLS
    juce::TextButton snapshotButton       { aeonix::tr (aeonix::TextId::snapshotButton) };
    juce::TextButton layoutButton         { aeonix::tr (aeonix::TextId::layoutButton) };
    juce::TextButton openSnapshotsButton  { aeonix::tr (aeonix::TextId::openSnapshotsButton) };
#endif
    juce::Label      statusLabel;

    // Floating notice (added as an invisible child and shown on save).
    aeonix::Toast toast_;

    // ---- MIDI ----
    std::unique_ptr<juce::MidiInput> midiInput;

    // ---- pending state (MIDI thread -> message thread) ----
    std::mutex midiMutex_;
    juce::String pendingStatus_;
    std::vector<KeyFeedback> pendingKeyFeedback_;
    float pendingPitchBend_ = 0.0f;
    float pendingModWheel_  = 0.0f;
    bool  hasPendingPitchBend_ = false;
    bool  hasPendingModWheel_  = false;

    // Computer keys currently held (keyCode -> MIDI note).
    std::map<int, int> heldKeys_;

    // Octave offset of the computer keyboard (in octaves).
    int typingOctave_ = 0;

    // Effective polyphony (8/16/32), persisted in config.json.
    int polyphony_ = 16;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
