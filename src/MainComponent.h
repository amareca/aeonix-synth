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
//  MainComponent — ventana principal (contenedor).
//
//  - Hilo de audio:    synthEngine.render().
//  - Hilo de MIDI:     traduce mensajes a noteOn/off/sustain (encolado lock-free)
//                      y encola texto de estado + feedback de teclas.
//  - Hilo de mensajes: pinta la GUI (AsyncUpdater), NUNCA desde otro hilo.
//
//  Contiene un SynthPanel (los controles + teclado virtual) que sigue el
//  patron MVVM: las Views hablan con ViewModels, no con el motor.
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

    // Ventana propia: MainWindow conecta estos callbacks con el DocumentWindow.
    std::function<void()> onWindowMinimise;
    std::function<void()> onWindowToggleMaximise;
    std::function<void()> onWindowClose;
    std::function<void (float, juce::Point<int>)> onWindowRestoreForDrag;

    // Sincroniza el icono maximizar/restaurar de la barra.
    void setWindowMaximised (bool maximised);

    // ---- juce::AudioAppComponent (hilo de audio) ----
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // ---- juce::MidiInputCallback (hilo de fondo de MIDI) ----
    void handleIncomingMidiMessage (juce::MidiInput* source,
                                    const juce::MidiMessage& message) override;

    // ---- juce::AsyncUpdater (hilo de mensajes) ----
    void handleAsyncUpdate() override;

    // ---- juce::KeyListener (teclado del ordenador como piano) ----
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

    // Tema visual.
    void applyTheme (aeonix::ThemeId theme);

    // Herramientas de desarrollo: F12 = snapshot PNG, F11 = informe de layout.
#if AEONIX_HAS_DEV_TOOLS
    void saveSnapshot();
    void saveLayoutReport();
    void openSnapshotsFolder();

    // Raiz de capturas y carpeta de la feature activa. La feature sale de
    // AEONIX_CAPTURE_TAG o del contexto visible (gestor, ajustes o main).
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

    // Evento de feedback para el teclado virtual (hilo MIDI -> hilo mensajes).
    struct KeyFeedback
    {
        bool noteOn = false;
        int note = 0;
        float velocity = 0.0f;
    };

    // ---- estilo (debe vivir mas que los hijos) ----
    aeonix::AeonixLookAndFeel lookAndFeel_;
    aeonix::ThemeId theme_ = aeonix::ThemeId::holoAccents;

    // ---- audio + sintesis ----
    aeonix::ConfigManager& config_;
    aeonix::PresetRepository presetRepository;
    aeonix::SynthEngine synthEngine;
    bool presetLoaded_ = false;
    aeonix::SynthPanel synthPanel;

    // ---- interfaz ----
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

    // Aviso flotante (se anade como hijo invisible y se muestra al guardar).
    aeonix::Toast toast_;

    // ---- MIDI ----
    std::unique_ptr<juce::MidiInput> midiInput;

    // ---- estado pendiente (hilo MIDI -> hilo mensajes) ----
    std::mutex midiMutex_;
    juce::String pendingStatus_;
    std::vector<KeyFeedback> pendingKeyFeedback_;
    float pendingPitchBend_ = 0.0f;
    float pendingModWheel_  = 0.0f;
    bool  hasPendingPitchBend_ = false;
    bool  hasPendingModWheel_  = false;

    // Teclas del ordenador actualmente pulsadas (keyCode -> nota MIDI).
    std::map<int, int> heldKeys_;

    // Desplazamiento de octava del teclado del ordenador (en octavas).
    int typingOctave_ = 0;

    // Polifonía efectiva (8/16/32), persistida en config.json.
    int polyphony_ = 16;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
