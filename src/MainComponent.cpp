#include "MainComponent.h"

#include "presentation/core/BuildFlavour.h"

namespace
{
// Mapping of the computer keyboard to MIDI notes (QWERTY-piano layout, like FL Studio).
// Row Z..M = base octave (C4..B4); row Q..U = +1 octave; row I..P = +2 octaves.
int noteForKeyCode (int keyCode, int octaveShift)
{
    constexpr int base = 60;   // C4 = middle C (bottom row Z) with octaveShift = 0

    int relative = -1;
    switch (keyCode)
    {
        case 'Z': relative = 0;  break;
        case 'S': relative = 1;  break;
        case 'X': relative = 2;  break;
        case 'D': relative = 3;  break;
        case 'C': relative = 4;  break;
        case 'V': relative = 5;  break;
        case 'G': relative = 6;  break;
        case 'B': relative = 7;  break;
        case 'H': relative = 8;  break;
        case 'N': relative = 9;  break;
        case 'J': relative = 10; break;
        case 'M': relative = 11; break;

        case 'Q': relative = 12; break;
        case '2': relative = 13; break;
        case 'W': relative = 14; break;
        case '3': relative = 15; break;
        case 'E': relative = 16; break;
        case 'R': relative = 17; break;
        case '5': relative = 18; break;
        case 'T': relative = 19; break;
        case '6': relative = 20; break;
        case 'Y': relative = 21; break;
        case '7': relative = 22; break;
        case 'U': relative = 23; break;

        case 'I': relative = 24; break;
        case '9': relative = 25; break;
        case 'O': relative = 26; break;
        case '0': relative = 27; break;
        case 'P': relative = 28; break;

        default: return -1;
    }

    return base + octaveShift * 12 + relative;
}
}

MainComponent::MainComponent()
    : config_ (aeonix::ConfigManager::getInstance()),
      presetRepository (juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                            .getChildFile (aeonix::build::dataFolder)
                            .getChildFile (aeonix::build::presetsFolder)),
      synthPanel (
        [this] (const std::string& id, float n)
        {
            synthEngine.setParameter (id, n);

            if (presetLoaded_)
                synthPanel.setPresetModified (true);
        },
        [this] (int note, float velocity)       { synthEngine.noteOn (note, velocity); },
        [this] (int note)                       { synthEngine.noteOff (note); },
        presetRepository),
      settingsOverlay_ (deviceManager)
{
    setLookAndFeel (&lookAndFeel_);
    setOpaque (true);
    setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff1b1f27));

    setSize (1180, 752);

    // Computer keyboard as a piano (FL Studio layout).
    addKeyListener (this);
    setWantsKeyboardFocus (true);
    grabKeyboardFocus();

    // ---------------------------------------------------------------- interface
    addAndMakeVisible (titleBar_);
    addAndMakeVisible (panicButton);
#if AEONIX_HAS_DEV_TOOLS
    addAndMakeVisible (snapshotButton);
    addAndMakeVisible (layoutButton);
    addAndMakeVisible (openSnapshotsButton);
#endif
    addAndMakeVisible (statusLabel);
    addAndMakeVisible (synthPanel);
    addChildComponent (settingsOverlay_);   // hidden until Settings is opened

    presetManagerOverlay_ = std::make_unique<aeonix::PresetManagerOverlay>();
    presetManagerOverlay_->onLoad = [this] (const juce::String& name)
    {
        synthPanel.requestLoadPreset (name, true);
    };
    presetManagerOverlay_->onAudition = [this] (const juce::String& name)
    {
        synthPanel.requestLoadPreset (name, false);
    };
    presetManagerOverlay_->onAuditionChanged = [this] (bool enabled)
    {
        config_.setInt ("presets.audition", enabled ? 1 : 0);
        config_.save();
    };
    presetManagerOverlay_->onSortChanged = [this] (aeonix::PresetManagerOverlay::SortMode mode)
    {
        config_.setString ("presets.sort", aeonix::PresetManagerOverlay::sortKey (mode));
        config_.save();
    };
    presetManagerOverlay_->onOpenFolder = [this] { openPresetsFolder(); };
    presetManagerOverlay_->onFavorite = [this] (const juce::String& name, bool favorite)
    {
        synthPanel.setPresetFavorite (name, favorite);
    };
    presetManagerOverlay_->onSaveCurrent = [this]
    {
        const bool saved = synthPanel.saveCurrentPreset();

        if (saved)
            refreshPresetManager();

        return saved;
    };
    presetManagerOverlay_->onSaveCopy = [this] (const juce::String& name, aeonix::PresetCategory category)
    {
        const bool saved = synthPanel.savePresetCopy (name, category);

        if (saved)
            refreshPresetManager();

        return saved;
    };
    presetManagerOverlay_->onRename = [this] (const juce::String& oldName, const juce::String& newName)
    {
        const bool renamed = synthPanel.renamePreset (oldName, newName);

        if (renamed)
            refreshPresetManager();

        return renamed;
    };
    presetManagerOverlay_->onDelete = [this] (const juce::String& name)
    {
        const bool deleted = synthPanel.deletePreset (name);

        if (deleted)
            refreshPresetManager();

        return deleted;
    };
    presetManagerOverlay_->onClosed = [this]
    {
        const auto name = synthPanel.currentPresetName();

        if (name.isNotEmpty())
        {
            config_.setString ("preset.last", name);
            config_.save();
        }

        grabKeyboardFocus();
    };
    addChildComponent (*presetManagerOverlay_);

    addChildComponent (toast_);             // invisible until a notice is shown

    titleBar_.onSettings = [this] { showSettings (aeonix::SettingsOverlay::Tab::audio); };
    titleBar_.onMidi     = [this] { showSettings (aeonix::SettingsOverlay::Tab::midi); };
    titleBar_.onMinimise = [this] { if (onWindowMinimise) onWindowMinimise(); };
    titleBar_.onToggleMaximise = [this] { if (onWindowToggleMaximise) onWindowToggleMaximise(); };
    titleBar_.onClose    = [this] { if (onWindowClose) onWindowClose(); };
    titleBar_.onRestoreForDrag = [this] (float proportion, juce::Point<int> screenPos)
    {
        if (onWindowRestoreForDrag)
            onWindowRestoreForDrag (proportion, screenPos);
    };

    settingsOverlay_.onMidiDeviceSelected = [this] (int index) { openMidiInput (index); };
    settingsOverlay_.onRefreshMidi = [this] { refreshMidiDevices (true); };
    settingsOverlay_.onThemeChanged = [this] (aeonix::ThemeId theme)
    {
        applyTheme (theme);
        config_.setString ("theme", aeonix::themeKey (theme));
        config_.save();
        statusLabel.setText (aeonix::trf (aeonix::TextId::statusTheme,
                                  { aeonix::themeName (theme) }),
                             juce::dontSendNotification);
    };
    settingsOverlay_.onLanguageChanged = [this] (aeonix::Language language)
    {
        config_.setString ("ui.language", aeonix::languageKey (language));
        config_.save();
        toast_.showMessage (aeonix::tr (aeonix::TextId::toastLanguageChanged), 3000);
    };
    settingsOverlay_.onTextSizeChanged = [this] (aeonix::TextSize size)
    {
        config_.setString ("ui.textSize", aeonix::textSizeKey (size));
        config_.save();
        toast_.showMessage (aeonix::tr (aeonix::TextId::toastTextSizeChanged), 3000);
    };
    settingsOverlay_.onClosed = [this] { grabKeyboardFocus(); };

    panicButton.onClick         = [this] { panic(); };
#if AEONIX_HAS_DEV_TOOLS
    snapshotButton.onClick      = [this] { saveSnapshot(); };
    layoutButton.onClick        = [this] { saveLayoutReport(); };
    openSnapshotsButton.onClick = [this] { openSnapshotsFolder(); };

    snapshotButton.setTooltip (aeonix::tr (aeonix::TextId::tooltipSnapshotButton));
    layoutButton.setTooltip (aeonix::tr (aeonix::TextId::tooltipLayoutButton));
    openSnapshotsButton.setTooltip (aeonix::tr (aeonix::TextId::tooltipOpenSnapshots));
#endif
    panicButton.setTooltip (aeonix::tr (aeonix::TextId::tooltipPanic));

    statusLabel.setJustificationType (juce::Justification::centredLeft);
    statusLabel.setColour (juce::Label::textColourId, juce::Colour (0xff9aa4b2));
    statusLabel.setFont (aeonix::ui::font (12.0f));

    synthPanel.onOctaveChanged = [this] (int octave)
    {
        typingOctave_ = octave;
        config_.setInt ("keyboard.octave", octave);
        config_.save();
    };
    synthPanel.onPolyphonyChanged = [this] (int voices)
    {
        polyphony_ = voices;
        synthEngine.setPolyphony (voices);
        config_.setInt ("polyphony", voices);
        config_.save();
        statusLabel.setText (aeonix::trf (aeonix::TextId::statusPolyphony,
                                          { juce::String (voices) }),
                             juce::dontSendNotification);
    };
    synthPanel.onPresetLoaded = [this] (const juce::String& name)
    {
        presetLoaded_ = true;
        synthPanel.setPresetModified (false);
        config_.setString ("preset.last", name);
        config_.save();

        if (presetManagerOverlay_ != nullptr)
            presetManagerOverlay_->notifyPresetLoaded (name);
    };
    synthPanel.onPresetPreviewed = [this] (const juce::String& name)
    {
        presetLoaded_ = true;
        synthPanel.setPresetModified (false);

        if (presetManagerOverlay_ != nullptr)
            presetManagerOverlay_->setCurrentPreset (name, false);
    };
    synthPanel.onOpenPresetsFolder = [this] { openPresetManager(); };
    synthPanel.onRequestSaveCopy = [this]
    {
        if (presetManagerOverlay_ != nullptr && ! presetManagerOverlay_->isOpen())
            openPresetManager();

        presetManagerOverlay_->beginCopyForm();
    };
    synthPanel.onPresetToast = [this] (const juce::String& message)
    {
        toast_.showMessage (message);
    };
    synthPanel.onPitchBend = [this] (float value)
    {
        synthEngine.setPitchBend (value);
        statusLabel.setText (aeonix::trf (aeonix::TextId::statusPitchBend,
                                  { juce::String (juce::roundToInt (value * 2.0f)) }),
                             juce::dontSendNotification);
    };
    synthPanel.onModWheel = [this] (float value)
    {
        synthEngine.setModWheel (value);
        statusLabel.setText (aeonix::trf (aeonix::TextId::statusModWheel,
                                  { juce::String (juce::roundToInt (value * 100.0f)) }),
                             juce::dontSendNotification);
    };

    // ---------------------------------------------------------------- audio
    // Polyphony: 8/16/32 voices. The saved value is rounded and applied
    // before starting the device so prepare() reserves the capacity.
    {
        const int saved = config_.getInt ("polyphony", 16);
        polyphony_ = saved <= 8 ? 8 : (saved <= 16 ? 16 : 32);
        synthPanel.setPolyphony (polyphony_);
    }

    // Restores the saved audio device (if any).
    std::unique_ptr<juce::XmlElement> audioState;
    const juce::String audioXml = config_.getString ("audio.state");
    if (audioXml.isNotEmpty())
        audioState = juce::XmlDocument::parse (audioXml);

    setAudioChannels (0, 2, audioState.get());   // 0 inputs, 2 outputs (stereo)
    statusLabel.setText (aeonix::tr (aeonix::TextId::statusReady),
                         juce::dontSendNotification);

    // ---------------------------------------------------------------- state
    typingOctave_ = juce::jlimit (-2, 2, config_.getInt ("keyboard.octave", 0));
    synthPanel.setOctave (typingOctave_);

    // ---------------------------------------------------------------- MIDI
    setupMidiDevices();

    // Last used preset (or Default if it no longer exists). Does not count as recent.
    synthPanel.selectPresetByName (config_.getString ("preset.last", "Default"), false);
    presetLoaded_ = true;
    synthPanel.setPresetModified (false);

    // Saved visual theme (Holo Accents by default).
    applyTheme (aeonix::themeFromKey (config_.getString ("theme", "holo.accents")));
}

MainComponent::~MainComponent()
{
    setLookAndFeel (nullptr);

    if (midiInput != nullptr)
        midiInput->stop();

    // Remembers the audio device state for the next session.
    if (auto state = deviceManager.createStateXml())
        config_.setString ("audio.state", state->toString());

    config_.save();

    shutdownAudio();   // mandatory in AudioAppComponent
}

// ==============================================================================
//  Layout
// ==============================================================================

void MainComponent::applyTheme (aeonix::ThemeId theme)
{
    theme_ = theme;

    lookAndFeel_.setTheme (theme);
    synthPanel.setTheme (theme);
    titleBar_.setTheme (theme);
    settingsOverlay_.setTheme (theme);
    settingsOverlay_.setThemeSelection (theme);

    if (presetManagerOverlay_ != nullptr)
        presetManagerOverlay_->setTheme (theme);
    statusLabel.setColour (juce::Label::textColourId, aeonix::themeColours (theme).muted);

    sendLookAndFeelChange();
    repaint();
}

void MainComponent::paint (juce::Graphics& g)
{
    const auto colours = aeonix::themeColours (theme_);
    const auto bounds = getLocalBounds();

    if (theme_ == aeonix::ThemeId::prototype)
    {
        g.fillAll (colours.background);
        return;
    }

    if (theme_ == aeonix::ThemeId::holoMinimal)
    {
        juce::ColourGradient gradient (colours.background,
                                       0.0f, 0.0f,
                                       colours.backgroundAlt,
                                       0.0f, static_cast<float> (getHeight()),
                                       false);
        g.setGradientFill (gradient);
        g.fillRect (bounds);
        return;
    }

    g.fillAll (colours.background);

    const auto glowAlpha = theme_ == aeonix::ThemeId::holoGlass ? 0.16f : 0.10f;

    juce::ColourGradient teal (juce::Colour (0xff4ecdc4).withAlpha (glowAlpha),
                               0.0f, 0.0f,
                               juce::Colours::transparentBlack,
                               static_cast<float> (getWidth()) * 0.60f,
                               static_cast<float> (getHeight()) * 0.45f,
                               true);
    g.setGradientFill (teal);
    g.fillRect (bounds);

    juce::ColourGradient violet (juce::Colour (0xffa78bfa).withAlpha (glowAlpha),
                                 static_cast<float> (getWidth()), 0.0f,
                                 juce::Colours::transparentBlack,
                                 static_cast<float> (getWidth()) * 0.35f,
                                 static_cast<float> (getHeight()) * 0.45f,
                                 true);
    g.setGradientFill (violet);
    g.fillRect (bounds);

    if (theme_ == aeonix::ThemeId::holoGlass)
    {
        juce::Random rng (0x5EED);

        for (int i = 0; i < 220; ++i)
        {
            const auto x = rng.nextFloat() * static_cast<float> (getWidth());
            const auto y = rng.nextFloat() * static_cast<float> (getHeight());
            const auto alpha = 0.10f + rng.nextFloat() * 0.35f;
            const auto size = (i % 7 == 0) ? 2.2f : 1.4f;

            g.setColour (juce::Colours::white.withAlpha (alpha));
            g.fillEllipse (x, y, size, size);
        }

        for (int i = 0; i < 28; ++i)
        {
            const auto x = rng.nextFloat() * static_cast<float> (getWidth());
            const auto y = rng.nextFloat() * static_cast<float> (getHeight());
            const auto alpha = 0.25f + rng.nextFloat() * 0.25f;
            const auto colour = (i % 2 == 0) ? juce::Colour (0xff4ecdc4)
                                             : juce::Colour (0xffa78bfa);

            g.setColour (colour.withAlpha (alpha));
            g.fillEllipse (x, y, 2.0f, 2.0f);
        }
    }
}

void MainComponent::paintOverChildren (juce::Graphics& g)
{
    // The window has no native frame: we draw a 1 px border above
    // all the content to separate it from the desktop.
    g.setColour (aeonix::themeColours (theme_).panelBorder.withAlpha (0.55f));
    g.drawRect (getLocalBounds(), 1);
}

void MainComponent::resized()
{
    settingsOverlay_.setBounds (getLocalBounds());

    if (presetManagerOverlay_ != nullptr)
        presetManagerOverlay_->setBounds (getLocalBounds());

    auto area = getLocalBounds();
    titleBar_.setBounds (area.removeFromTop (aeonix::ui::px (aeonix::TitleBar::preferredHeight)));

    auto b = area.reduced (aeonix::ui::px (12));

    auto topBar = b.removeFromTop (aeonix::ui::px (36));
    panicButton.setBounds (topBar.removeFromLeft (aeonix::ui::px (110)));
    topBar.removeFromLeft (aeonix::ui::px (10));

#if AEONIX_HAS_DEV_TOOLS
    auto right = topBar.removeFromRight (aeonix::ui::px (86 + 4 + 70 + 4 + 96));
    snapshotButton.setBounds (right.removeFromLeft (aeonix::ui::px (86)));
    right.removeFromLeft (aeonix::ui::px (4));
    layoutButton.setBounds (right.removeFromLeft (aeonix::ui::px (70)));
    right.removeFromLeft (aeonix::ui::px (4));
    openSnapshotsButton.setBounds (right.removeFromLeft (aeonix::ui::px (96)));
#endif

    topBar.removeFromRight (aeonix::ui::px (12));
    statusLabel.setBounds (topBar);

    b.removeFromTop (aeonix::ui::px (12));
    synthPanel.setBounds (b);

    toast_.updateBounds();
}

// ==============================================================================
//  Audio (real-time thread)
// ==============================================================================

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // Reserves the maximum capacity (32) and applies the configured limit; that way the
    // user can switch 8/16/32 without restarting the audio.
    synthEngine.prepare (sampleRate, samplesPerBlockExpected,
                         aeonix::SynthEngine::maxVoiceCapacity);
    synthEngine.setPolyphony (polyphony_);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    juce::ScopedNoDenormals noDenormals;

    auto* left  = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    auto* right = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

    synthEngine.render (left, right, bufferToFill.numSamples);
}

void MainComponent::releaseResources()
{
    // SynthEngine does not reserve audio resources; nothing to release.
}

// ==============================================================================
//  MIDI (background thread) - the GUI is NOT painted here.
// ==============================================================================

void MainComponent::handleIncomingMidiMessage (juce::MidiInput* source,
                                               const juce::MidiMessage& message)
{
    juce::ignoreUnused (source);

    juce::String text;
    KeyFeedback feedback;
    bool hasFeedback = false;

    if (message.isNoteOn())
    {
        const int note       = message.getNoteNumber();
        const float velocity = static_cast<float> (message.getVelocity()) / 127.0f;

        synthEngine.noteOn (note, velocity);

        if (velocity > 0.0f)
        {
            text = "NoteOn  " + juce::MidiMessage::getMidiNoteName (note, true, true, 4)
                 + "   vel=" + juce::String (message.getVelocity());
            feedback = { true, note, velocity };
        }
        else
        {
            text = "NoteOff " + juce::MidiMessage::getMidiNoteName (note, true, true, 4);
            feedback = { false, note, 0.0f };
        }

        hasFeedback = true;
    }
    else if (message.isNoteOff())
    {
        const int note = message.getNoteNumber();

        synthEngine.noteOff (note);

        text = "NoteOff " + juce::MidiMessage::getMidiNoteName (note, true, true, 4);
        feedback = { false, note, 0.0f };
        hasFeedback = true;
    }
    else if (message.isController() && message.getControllerNumber() == 64)
    {
        synthEngine.setSustain (message.getControllerValue() > 0);

        text = "Sustain CC64 = " + juce::String (message.getControllerValue());
    }
    else if (message.isController() && message.getControllerNumber() == 1)
    {
        const float mod = message.getControllerValue() / 127.0f;
        synthEngine.setModWheel (mod);

        {
            const std::lock_guard<std::mutex> lock (midiMutex_);
            pendingModWheel_ = mod;
            hasPendingModWheel_ = true;
        }

        text = "Mod Wheel CC1 = " + juce::String (message.getControllerValue());
    }
    else if (message.isPitchWheel())
    {
        // 0..16383 with centre at 8192 -> -1..+1 (fixed range +-2 semitones).
        const float bend = (message.getPitchWheelValue() - 8192) / 8192.0f;
        synthEngine.setPitchBend (bend);

        {
            const std::lock_guard<std::mutex> lock (midiMutex_);
            pendingPitchBend_ = bend;
            hasPendingPitchBend_ = true;
        }

        text = "Pitch Bend " + juce::String (bend, 2);
    }
    else
    {
        text = message.getDescription();
    }

    {
        const std::lock_guard<std::mutex> lock (midiMutex_);
        pendingStatus_ = text;
        if (hasFeedback)
            pendingKeyFeedback_.push_back (feedback);
    }

    triggerAsyncUpdate();
}

// ==============================================================================
//  GUI (message thread, via AsyncUpdater)
// ==============================================================================

void MainComponent::handleAsyncUpdate()
{
    juce::String status;
    std::vector<KeyFeedback> feedback;
    float pitchBend = 0.0f;
    float modWheel = 0.0f;
    bool updatePitchBend = false;
    bool updateModWheel = false;
    {
        const std::lock_guard<std::mutex> lock (midiMutex_);
        status = pendingStatus_;
        pendingStatus_.clear();
        feedback.swap (pendingKeyFeedback_);

        updatePitchBend = hasPendingPitchBend_;
        updateModWheel = hasPendingModWheel_;
        pitchBend = pendingPitchBend_;
        modWheel = pendingModWheel_;
        hasPendingPitchBend_ = false;
        hasPendingModWheel_ = false;
    }

    if (status.isNotEmpty())
        statusLabel.setText (status, juce::dontSendNotification);

    if (updatePitchBend)
        synthPanel.setPitchBendDisplay (pitchBend);

    if (updateModWheel)
        synthPanel.setModWheelDisplay (modWheel);

    for (const auto& f : feedback)
    {
        if (f.noteOn)
            synthPanel.highlightNoteOn (f.note, f.velocity);
        else
            synthPanel.highlightNoteOff (f.note);
    }
}

// ==============================================================================
//  Computer keyboard (KeyListener)
// ==============================================================================

bool MainComponent::keyPressed (const juce::KeyPress& key, juce::Component*)
{
    const int keyCode = key.getKeyCode();
    const auto mods = key.getModifiers();

    // On Windows/JUCE, Ctrl+S arrives as control character 0x13 (19), not
    // as 'S'. We normalize Ctrl/Cmd+letter to its letter before comparing.
    int shortcutKey = keyCode;
    if (mods.isCommandDown() && keyCode >= 1 && keyCode <= 26)
        shortcutKey = 'A' + keyCode - 1;

    // Development tools (they do not affect the musical keyboard).
    // F11/F12 may require Fn on some laptops; Ctrl+Shift is the
    // universal alternative. They also work with Settings open so as to
    // capture that feature. F5-F8 switch themes and F10 saves PNG + report.
#if AEONIX_HAS_DEV_TOOLS
    const bool cmdShift = mods.isCommandDown() && mods.isShiftDown();
    const bool isSnapshotShortcut = keyCode == juce::KeyPress::F12Key
                                 || (cmdShift && (shortcutKey == 'S' || shortcutKey == 's'));
    const bool isLayoutShortcut = keyCode == juce::KeyPress::F11Key
                               || (cmdShift && (shortcutKey == 'L' || shortcutKey == 'l'));
    const bool isThemeShortcut = keyCode >= juce::KeyPress::F5Key
                              && keyCode <= juce::KeyPress::F8Key;

    if (isThemeShortcut)
    {
        applyTheme (static_cast<aeonix::ThemeId> (keyCode - juce::KeyPress::F5Key));
        config_.setString ("theme", aeonix::themeKey (theme_));
        config_.save();
        return true;
    }

    if (keyCode == juce::KeyPress::F10Key)
    {
        saveSnapshot();
        saveLayoutReport();
        return true;
    }

    if (isSnapshotShortcut)
    {
        saveSnapshot();
        return true;
    }

    if (isLayoutShortcut)
    {
        saveLayoutReport();
        return true;
    }
#endif

    // With settings open, the keyboard must not touch the synthesizer.
    if (settingsOverlay_.isOpen())
    {
        if (keyCode == juce::KeyPress::escapeKey)
            settingsOverlay_.hide();

        return true;
    }

    // Global settings shortcut (Ctrl+, on Windows/Linux; Cmd+, on macOS).
    if (keyCode == ',' && mods.isCommandDown())
    {
        showSettings (aeonix::SettingsOverlay::Tab::audio);
        return true;
    }

    const bool isSaveShortcut = mods.isCommandDown() && ! mods.isShiftDown()
                             && (shortcutKey == 'S' || shortcutKey == 's');

    if (isSaveShortcut)
    {
        synthPanel.savePreset();
        return true;
    }

    const int note = noteForKeyCode (keyCode, typingOctave_);

    if (note < 0)
        return false;   // not a mapped key

    if (heldKeys_.count (keyCode))
        return true;    // auto-repeat: ignore

    heldKeys_[keyCode] = note;

    synthEngine.noteOn (note, 1.0f);
    synthPanel.highlightNoteOn (note, 1.0f);
    return true;
}

bool MainComponent::keyStateChanged (bool isKeyDown, juce::Component*)
{
    if (isKeyDown)
        return false;

    // Some key was released: free the mapped keys that are no longer held.
    for (auto it = heldKeys_.begin(); it != heldKeys_.end(); )
    {
        if (!juce::KeyPress::isKeyCurrentlyDown (it->first))
        {
            synthEngine.noteOff (it->second);
            synthPanel.highlightNoteOff (it->second);
            it = heldKeys_.erase (it);
        }
        else
        {
            ++it;
        }
    }

    return false;
}

// ==============================================================================
//  UI actions (message thread)
// ==============================================================================

void MainComponent::panic()
{
    synthEngine.allNotesOff();
    synthPanel.resetPerformanceControls();
    synthPanel.clearAllNotes();
    statusLabel.setText (aeonix::tr (aeonix::TextId::statusPanic), juce::dontSendNotification);
}

// ==============================================================================
//  Development tools
// ==============================================================================

#if AEONIX_HAS_DEV_TOOLS

juce::File MainComponent::developmentOutputDirectory()
{
    auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                   .getChildFile ("Aeonix")
                   .getChildFile ("snapshots");

    dir.createDirectory();
    return dir;
}

juce::String MainComponent::sanitiseCaptureTag (const juce::String& tag)
{
    juce::String result;

    for (auto character : tag.trim().toLowerCase())
    {
        if ((character >= 'a' && character <= 'z')
            || (character >= '0' && character <= '9')
            || character == '-' || character == '_')
        {
            result << character;
        }
        else if (character == ' ' || character == '.')
        {
            result << '-';
        }
    }

    return result.substring (0, 40);
}

juce::String MainComponent::captureTag() const
{
    const auto fromEnvironment = sanitiseCaptureTag (
        juce::SystemStats::getEnvironmentVariable ("AEONIX_CAPTURE_TAG", {}));

    if (fromEnvironment.isNotEmpty())
        return fromEnvironment;

    if (presetManagerOverlay_ != nullptr && presetManagerOverlay_->isOpen())
        return "presets";

    if (settingsOverlay_.isVisible())
        return "settings";

    return "main";
}

juce::String MainComponent::captureFileStem() const
{
    if (! captureMatrixEnabled())
        return {};

    const auto size = juce::String (getWidth()) + "x" + juce::String (getHeight());
    auto theme = aeonix::themeKey (theme_);
    theme = theme.replace (".", "-");

    return "matrix-" + aeonix::languageKey (aeonix::getLanguage())
         + "-" + theme
         + "-" + aeonix::textSizeKey (aeonix::ui::getTextSize())
         + "-" + size;
}

juce::File MainComponent::captureDirectory() const
{
    auto dir = developmentOutputDirectory().getChildFile (captureTag());
    dir.createDirectory();
    return dir;
}

bool MainComponent::captureMatrixEnabled() const
{
    const auto value = juce::SystemStats::getEnvironmentVariable ("AEONIX_CAPTURE_MATRIX", {}).trim();

    return value == "1"
        || value.equalsIgnoreCase ("true")
        || value.equalsIgnoreCase ("yes");
}

bool MainComponent::captureArchiveEnabled() const
{
    const auto value = juce::SystemStats::getEnvironmentVariable ("AEONIX_CAPTURE_ARCHIVE", {}).trim();

    return value == "1"
        || value.equalsIgnoreCase ("true")
        || value.equalsIgnoreCase ("yes");
}

void MainComponent::archiveCapture (const juce::File& file) const
{
    if (! captureArchiveEnabled() || ! file.existsAsFile())
        return;

    const auto now = juce::Time::getCurrentTime();
    auto dir = captureDirectory()
                   .getChildFile ("archive")
                   .getChildFile (now.formatted ("%Y-%m-%d"));

    dir.createDirectory();

    auto target = dir.getChildFile (now.formatted ("%H%M%S") + "-" + file.getFileName());
    target.deleteFile();
    file.copyFileTo (target);
}

void MainComponent::saveSnapshot()
{
    // We hide the Toast so it does not appear in the PNG.
    toast_.hideToast();

    // It is captured before touching the status bar so the PNG comes out clean.
    const auto image = createComponentSnapshot (getLocalBounds(), true, 1.0f);

    if (image.isNull())
    {
        statusLabel.setText (aeonix::tr (aeonix::TextId::statusSnapshotNoRender),
                             juce::dontSendNotification);
        toast_.showMessage (aeonix::tr (aeonix::TextId::toastSnapshotNoRender), 3000);
        return;
    }

    const auto size = juce::String (getWidth()) + "x" + juce::String (getHeight());
    const auto stem = captureMatrixEnabled() ? captureFileStem() : "snapshot-" + size;
    auto file = captureDirectory().getChildFile (stem + ".png");

    juce::MemoryOutputStream buffer;
    juce::PNGImageFormat png;

    if (png.writeImageToStream (image, buffer)
        && file.replaceWithData (buffer.getData(), buffer.getDataSize()))
    {
        archiveCapture (file);
        statusLabel.setText (aeonix::tr (aeonix::TextId::statusSnapshotSaved),
                             juce::dontSendNotification);
        toast_.showMessage (aeonix::trf (aeonix::TextId::toastSnapshotSaved,
                                 { file.getFileName() }));
        return;
    }

    statusLabel.setText (aeonix::tr (aeonix::TextId::statusSnapshotError),
                         juce::dontSendNotification);
    toast_.showMessage (aeonix::tr (aeonix::TextId::toastSnapshotError), 3000);
}

void MainComponent::saveLayoutReport()
{
    const auto size = juce::String (getWidth()) + "x" + juce::String (getHeight());
    const auto stem = captureMatrixEnabled() ? captureFileStem() : "layout-" + size;
    auto file = captureDirectory().getChildFile (stem + ".txt");

    juce::String report;
    report << "Feature: " << captureTag() << "\n";
    report << "Theme: " << aeonix::themeKey (theme_) << "\n";
    report << "Language: " << aeonix::languageKey (aeonix::getLanguage()) << "\n";
    report << "Text size: " << aeonix::textSizeKey (aeonix::ui::getTextSize()) << "\n";
    report << "MainComponent: " << getWidth() << " x " << getHeight() << "\n";
    report << "LookAndFeel: AeonixLookAndFeel\n\n";
    report << synthPanel.createLayoutReport();

    if (file.replaceWithText (report))
    {
        archiveCapture (file);
        statusLabel.setText (aeonix::tr (aeonix::TextId::statusLayoutSaved),
                             juce::dontSendNotification);
        toast_.showMessage (aeonix::trf (aeonix::TextId::toastLayoutSaved,
                                 { file.getFileName() }));
    }
    else
    {
        statusLabel.setText (aeonix::tr (aeonix::TextId::statusLayoutError),
                             juce::dontSendNotification);
        toast_.showMessage (aeonix::tr (aeonix::TextId::toastLayoutError), 3000);
    }
}

void MainComponent::openSnapshotsFolder()
{
    auto dir = developmentOutputDirectory();
    dir.revealToUser();
    toast_.showMessage (aeonix::tr (aeonix::TextId::toastSnapshotsFolder));
}

#endif

void MainComponent::openPresetsFolder()
{
    auto dir = presetRepository.getDirectory();
    dir.createDirectory();
    dir.startAsProcess();   // opens the presets folder directly

    statusLabel.setText (aeonix::tr (aeonix::TextId::statusPresetsFolder), juce::dontSendNotification);
    toast_.showMessage (aeonix::tr (aeonix::TextId::toastPresetsFolder));
}

void MainComponent::openPresetManager()
{
    presetManagerOverlay_->setBounds (getLocalBounds());
    presetManagerOverlay_->setTheme (theme_);
    presetManagerOverlay_->setAudition (config_.getInt ("presets.audition", 0) != 0);
    presetManagerOverlay_->setSortMode (
        aeonix::PresetManagerOverlay::sortModeFromKey (config_.getString ("presets.sort", "origin")),
        juce::dontSendNotification);
    presetManagerOverlay_->show (synthPanel.presetEntries(),
                                 synthPanel.currentPresetName(),
                                 synthPanel.isPresetModified());
}

void MainComponent::refreshPresetManager()
{
    if (presetManagerOverlay_ != nullptr)
        presetManagerOverlay_->refreshEntries (synthPanel.presetEntries(),
                                               synthPanel.currentPresetName(),
                                               synthPanel.isPresetModified());
}

void MainComponent::showSettings (aeonix::SettingsOverlay::Tab tab)
{
    // Refreshes the list in case a device has been connected/disconnected.
    refreshMidiDevices (false);

    settingsOverlay_.setTheme (theme_);
    settingsOverlay_.setThemeSelection (theme_);
    settingsOverlay_.setLanguageSelection (aeonix::getLanguage());
    settingsOverlay_.setTextSizeSelection (aeonix::ui::getTextSize());
    settingsOverlay_.show (tab);
}

void MainComponent::setWindowMaximised (bool maximised)
{
    titleBar_.setMaximised (maximised);
}

// ==============================================================================
//  MIDI: enumeration and opening
// ==============================================================================

void MainComponent::setupMidiDevices()
{
    refreshMidiDevices (true);
}

void MainComponent::refreshMidiDevices (bool reopenDevice)
{
    const auto inputs = juce::MidiInput::getAvailableDevices();

    juce::StringArray names;
    for (const auto& device : inputs)
        names.add (device.name);

    // Priority: remembered device -> Oxygen -> first available.
    const juce::String savedId = config_.getString ("midi.deviceId");

    int selected = -1;
    for (int i = 0; i < inputs.size(); ++i)
        if (savedId.isNotEmpty() && inputs[i].identifier == savedId)
        {
            selected = i;
            break;
        }

    if (selected < 0)
        for (int i = 0; i < inputs.size(); ++i)
            if (inputs[i].name.containsIgnoreCase ("oxygen"))
            {
                selected = i;
                break;
            }

    if (selected < 0 && inputs.size() > 0)
        selected = 0;

    settingsOverlay_.refreshMidiDevices (names, selected);

    if (reopenDevice && selected >= 0)
        openMidiInput (selected);
}

void MainComponent::openMidiInput (int deviceIndex)
{
    const auto inputs = juce::MidiInput::getAvailableDevices();

    if (deviceIndex < 0 || deviceIndex >= inputs.size())
        return;

    if (midiInput != nullptr)
        midiInput->stop();

    midiInput = juce::MidiInput::openDevice (inputs[deviceIndex].identifier, this);

    if (midiInput != nullptr)
    {
        midiInput->start();

        config_.setString ("midi.deviceId", inputs[deviceIndex].identifier);
        config_.setString ("midi.deviceName", inputs[deviceIndex].name);
        config_.save();

        statusLabel.setText (aeonix::trf (aeonix::TextId::statusMidi,
                                  { inputs[deviceIndex].name }),
                             juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText (aeonix::trf (aeonix::TextId::statusMidiError,
                                  { inputs[deviceIndex].name }),
                             juce::dontSendNotification);
    }
}
