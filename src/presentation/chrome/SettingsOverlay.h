#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <functional>

#include "presentation/core/I18n.h"
#include "presentation/core/Utf8.h"
#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"
#include "presentation/widgets/IconButton.h"

namespace aeonix
{

// ============================================================================
//  SettingsOverlay — panel de ajustes modal dibujado por la app.
//
//  Se abre desde los iconos de la barra de titulo y se pinta sobre toda la
//  interfaz (scrim + tarjeta centrada), sin depender de dialogos nativos.
//
//  Pestanas:
//    - Audio:      AudioDeviceSelectorComponent embebido en un viewport.
//    - MIDI:       dispositivo de entrada (se guarda en la configuracion).
//    - Apariencia: tema visual.
//
//  Se cierra con el boton X, con Escape o haciendo clic fuera de la tarjeta.
// ============================================================================
class SettingsOverlay final : public juce::Component
{
public:
    enum class Tab
    {
        audio = 0,
        midi = 1,
        appearance = 2
    };

    explicit SettingsOverlay (juce::AudioDeviceManager& deviceManager)
        : audioSelector_ (deviceManager, 0, 0, 0, 2, false, false, true, false)
    {
        setVisible (false);
        setWantsKeyboardFocus (true);
        setTitle (tr (TextId::settingsTitle));
        setDescription (tr (TextId::settingsDescription));

        audioSelector_.setSize (600, 420);
        audioViewport_.setViewedComponent (&audioSelector_, false);
        audioViewport_.setScrollBarsShown (true, false);
        addAndMakeVisible (audioViewport_);

        titleLabel_.setText (tr (TextId::settingsTitle), juce::dontSendNotification);
        titleLabel_.setFont (ui::font (18.0f, juce::Font::bold));
        titleLabel_.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (titleLabel_);

        closeButton_.onClick = [this] { hide(); };
        addAndMakeVisible (closeButton_);

        const TextId tabIds[] = { TextId::settingsTabAudio,
                                  TextId::settingsTabMidi,
                                  TextId::settingsTabAppearance };

        for (int i = 0; i < numTabs; ++i)
        {
            const auto tabName = tr (tabIds[i]);
            tabButtons_[i].setButtonText (tabName);
            tabButtons_[i].setTooltip (trf (TextId::settingsTabTooltip, { tabName }));
            tabButtons_[i].setWantsKeyboardFocus (false);
            tabButtons_[i].onClick = [this, i] { setTab (static_cast<Tab> (i)); };
            addAndMakeVisible (tabButtons_[i]);
        }

        // ---- pestana MIDI ----
        midiLabel_.setText (tr (TextId::settingsMidiDevice), juce::dontSendNotification);
        midiLabel_.setFont (ui::font (12.0f));
        midiLabel_.setJustificationType (juce::Justification::centredLeft);
        addChildComponent (midiLabel_);

        midiCombo_.setTooltip (tr (TextId::settingsMidiDeviceTooltip));
        midiCombo_.onChange = [this]
        {
            const auto index = midiCombo_.getSelectedItemIndex();

            if (index >= 0 && onMidiDeviceSelected)
                onMidiDeviceSelected (index);
        };
        addChildComponent (midiCombo_);

        refreshButton_.setButtonText (tr (TextId::settingsRefresh));
        refreshButton_.setTooltip (tr (TextId::settingsRefreshTooltip));
        refreshButton_.onClick = [this] { if (onRefreshMidi) onRefreshMidi(); };
        addChildComponent (refreshButton_);

        midiHint_.setText (tr (TextId::settingsMidiHint),
                           juce::dontSendNotification);
        midiHint_.setFont (ui::font (11.0f));
        midiHint_.setJustificationType (juce::Justification::centredLeft);
        addChildComponent (midiHint_);

        // ---- pestana Apariencia ----
        appearanceLabel_.setText (tr (TextId::settingsThemeLabel), juce::dontSendNotification);
        appearanceLabel_.setFont (ui::font (12.0f));
        appearanceLabel_.setJustificationType (juce::Justification::centredLeft);
        addChildComponent (appearanceLabel_);

        for (int i = 0; i < 4; ++i)
            themeCombo_.addItem (themeName (themeFromIndex (i)), i + 1);

        themeCombo_.setTooltip (tr (TextId::settingsThemeTooltip));
        themeCombo_.setName ("Theme");
        themeCombo_.setTitle (tr (TextId::settingsThemeLabel));
        themeCombo_.onChange = [this]
        {
            if (onThemeChanged)
                onThemeChanged (themeFromIndex (themeCombo_.getSelectedId() - 1));
        };
        addChildComponent (themeCombo_);

        appearanceHint_.setText (tr (TextId::settingsAppearanceHint),
                                 juce::dontSendNotification);
        appearanceHint_.setFont (ui::font (11.0f));
        appearanceHint_.setJustificationType (juce::Justification::centredLeft);
        addChildComponent (appearanceHint_);

        languageLabel_.setText (tr (TextId::settingsLanguageLabel),
                                juce::dontSendNotification);
        languageLabel_.setFont (ui::font (12.0f));
        languageLabel_.setJustificationType (juce::Justification::centredLeft);
        addChildComponent (languageLabel_);

        languageCombo_.addItem (languageDisplayName (Language::spanish), 1);
        languageCombo_.addItem (languageDisplayName (Language::english), 2);
        languageCombo_.setTooltip (tr (TextId::settingsLanguageTooltip));
        languageCombo_.setName ("Language");
        languageCombo_.setTitle (tr (TextId::settingsLanguageLabel));
        languageCombo_.onChange = [this]
        {
            if (onLanguageChanged)
                onLanguageChanged (languageCombo_.getSelectedId() == 1
                                       ? Language::spanish
                                       : Language::english);
        };
        addChildComponent (languageCombo_);

        languageHint_.setText (tr (TextId::settingsLanguageHint),
                               juce::dontSendNotification);
        languageHint_.setFont (ui::font (11.0f));
        languageHint_.setJustificationType (juce::Justification::centredLeft);
        addChildComponent (languageHint_);

        textSizeLabel_.setText (tr (TextId::settingsTextSizeLabel), juce::dontSendNotification);
        textSizeLabel_.setFont (ui::font (12.0f));
        textSizeLabel_.setJustificationType (juce::Justification::centredLeft);
        addChildComponent (textSizeLabel_);

        for (int i = 0; i < 3; ++i)
            textSizeCombo_.addItem (ui::textSizeName (static_cast<TextSize> (i)), i + 1);

        textSizeCombo_.setTooltip (tr (TextId::settingsTextSizeTooltip));
        textSizeCombo_.setName ("TextSize");
        textSizeCombo_.setTitle (tr (TextId::settingsTextSizeLabel));
        textSizeCombo_.onChange = [this]
        {
            const int index = textSizeCombo_.getSelectedId() - 1;

            if (index >= 0 && onTextSizeChanged)
                onTextSizeChanged (static_cast<TextSize> (index));
        };
        addChildComponent (textSizeCombo_);

        textSizeHint_.setText (tr (TextId::settingsTextSizeHint), juce::dontSendNotification);
        textSizeHint_.setFont (ui::font (11.0f));
        textSizeHint_.setJustificationType (juce::Justification::centredLeft);
        addChildComponent (textSizeHint_);

        // ---- pie ----
        versionLabel_.setText (tr (TextId::settingsVersion), juce::dontSendNotification);
        versionLabel_.setFont (ui::font (10.5f));
        versionLabel_.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (versionLabel_);

        escLabel_.setText (tr (TextId::settingsEsc), juce::dontSendNotification);
        escLabel_.setFont (ui::font (10.5f));
        escLabel_.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (escLabel_);

        setTab (Tab::audio);
        applyThemeToChildren();
    }

    // ---- callbacks ----
    std::function<void (int)> onMidiDeviceSelected;
    std::function<void()> onRefreshMidi;
    std::function<void (ThemeId)> onThemeChanged;
    std::function<void (Language)> onLanguageChanged;
    std::function<void (TextSize)> onTextSizeChanged;
    std::function<void()> onClosed;

    // ---- estado ----
    void show (Tab tab)
    {
        setTab (tab);
        setVisible (true);
        toFront (false);
        resized();
        grabKeyboardFocus();
    }

    void hide()
    {
        if (! isVisible())
            return;

        setVisible (false);

        if (onClosed)
            onClosed();
    }

    [[nodiscard]] bool isOpen() const noexcept { return isVisible(); }

    void setTheme (ThemeId theme)
    {
        theme_ = theme;
        closeButton_.setTheme (theme);
        applyThemeToChildren();
        updateTabColours();
        repaint();
    }

    void setThemeSelection (ThemeId theme)
    {
        themeCombo_.setSelectedId (themeToIndex (theme) + 1, juce::dontSendNotification);
    }

    void setLanguageSelection (Language language)
    {
        languageCombo_.setSelectedId (language == Language::spanish ? 1 : 2,
                                      juce::dontSendNotification);
    }

    void setTextSizeSelection (TextSize size)
    {
        textSizeCombo_.setSelectedId (static_cast<int> (size) + 1,
                                      juce::dontSendNotification);
    }

    void refreshMidiDevices (const juce::StringArray& names, int selectedIndex)
    {
        midiCombo_.clear (juce::dontSendNotification);

        if (names.isEmpty())
        {
            midiCombo_.addItem (tr (TextId::settingsNoMidi), 1);
            midiCombo_.setSelectedId (1, juce::dontSendNotification);
            midiCombo_.setEnabled (false);
            return;
        }

        midiCombo_.setEnabled (true);

        for (int i = 0; i < names.size(); ++i)
            midiCombo_.addItem (names[i], i + 1);

        if (selectedIndex >= 0 && selectedIndex < names.size())
            midiCombo_.setSelectedItemIndex (selectedIndex, juce::dontSendNotification);
    }

    // ---- juce::Component ----
    void paint (juce::Graphics& g) override
    {
        const auto colours = themeColours (theme_);

        g.fillAll (juce::Colours::black.withAlpha (0.58f));

        const auto panel = panelBounds_.toFloat();

        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (panel.translated (0.0f, 8.0f), 12.0f);

        g.setColour (colours.panel);
        g.fillRoundedRectangle (panel, 12.0f);

        g.setColour (colours.panelBorder);
        g.drawRoundedRectangle (panel, 12.0f, 1.0f);

        const auto line = juce::Rectangle<int> (panelBounds_.getX() + ui::px (20),
                                                panelBounds_.getY() + ui::px (headerHeight),
                                                panelBounds_.getWidth() - ui::px (40), 2);

        if (colours.gradientLines)
        {
            juce::ColourGradient gradient (holoColour (0.0f),
                                           static_cast<float> (line.getX()), 0.0f,
                                           holoColour (1.0f),
                                           static_cast<float> (line.getRight()), 0.0f,
                                           false);
            g.setGradientFill (gradient);
            g.fillRect (line);
        }
        else
        {
            g.setColour (theme_ == ThemeId::prototype ? colours.accent : colours.panelBorder);
            g.fillRect (line);
        }
    }

    void resized() override
    {
        const auto full = getLocalBounds();
        const int panelW = juce::jmin (ui::px (720), juce::jmax (ui::px (560), full.getWidth() - ui::px (48)));
        const int panelH = juce::jmin (ui::px (620), juce::jmax (ui::px (480), full.getHeight() - ui::px (48)));

        panelBounds_ = full.withSizeKeepingCentre (panelW, panelH);

        auto p = panelBounds_.reduced (ui::px (20));

        auto header = p.removeFromTop (ui::px (headerHeight) - ui::px (20));
        closeButton_.setBounds (header.removeFromRight (ui::px (32))
                                      .withSizeKeepingCentre (ui::px (32), ui::px (24)));
        titleLabel_.setBounds (header);

        p.removeFromTop (ui::px (10));

        auto tabs = p.removeFromTop (ui::px (30));
        auto x = tabs.getX();
        const int tabWidth = ui::px (112);

        for (auto& button : tabButtons_)
        {
            button.setBounds (x, tabs.getY(), tabWidth, tabs.getHeight());
            x += tabWidth + ui::px (8);
        }

        p.removeFromTop (ui::px (14));

        auto footer = p.removeFromBottom (ui::px (22));
        versionLabel_.setBounds (footer.removeFromLeft (ui::px (300)));
        escLabel_.setBounds (footer.removeFromRight (ui::px (160)));

        p.removeFromBottom (ui::px (8));
        contentBounds_ = p;

        // ---- Audio ----
        audioViewport_.setBounds (contentBounds_);
        audioSelector_.setSize (juce::jmax (200, contentBounds_.getWidth() - 14),
                                juce::jmax (380, audioSelector_.getHeight()));

        // ---- MIDI ----
        auto midi = contentBounds_.reduced (ui::px (2), ui::px (6));
        midiLabel_.setBounds (midi.removeFromTop (ui::px (20)));
        midi.removeFromTop (ui::px (8));

        auto midiRow = midi.removeFromTop (ui::px (28));
        midiCombo_.setBounds (midiRow.removeFromLeft (juce::jmin (ui::px (340), midiRow.getWidth() - ui::px (130))));
        midiRow.removeFromLeft (ui::px (10));
        refreshButton_.setBounds (midiRow.removeFromLeft (ui::px (110)));

        midi.removeFromTop (ui::px (10));
        midiHint_.setBounds (midi.removeFromTop (ui::px (20)));

        // ---- Apariencia ----
        auto look = contentBounds_.reduced (ui::px (2), ui::px (6));
        appearanceLabel_.setBounds (look.removeFromTop (ui::px (20)));
        look.removeFromTop (ui::px (8));
        themeCombo_.setBounds (look.removeFromTop (ui::px (28)).removeFromLeft (ui::px (240)));
        look.removeFromTop (ui::px (10));
        appearanceHint_.setBounds (look.removeFromTop (ui::px (20)));
        look.removeFromTop (ui::px (14));
        languageLabel_.setBounds (look.removeFromTop (ui::px (20)));
        look.removeFromTop (ui::px (8));
        languageCombo_.setBounds (look.removeFromTop (ui::px (28)).removeFromLeft (ui::px (240)));
        look.removeFromTop (ui::px (10));
        languageHint_.setBounds (look.removeFromTop (ui::px (20)));
        look.removeFromTop (ui::px (14));
        textSizeLabel_.setBounds (look.removeFromTop (ui::px (20)));
        look.removeFromTop (ui::px (8));
        textSizeCombo_.setBounds (look.removeFromTop (ui::px (28)).removeFromLeft (ui::px (240)));
        look.removeFromTop (ui::px (10));
        textSizeHint_.setBounds (look.removeFromTop (ui::px (20)));
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (! panelBounds_.contains (e.getPosition()))
            hide();
    }

    bool keyPressed (const juce::KeyPress& key) override
    {
        if (key.getKeyCode() == juce::KeyPress::escapeKey)
        {
            hide();
            return true;
        }

        return false;
    }

private:
    static constexpr int numTabs      = 3;
    static constexpr int tabWidth     = 112;
    static constexpr int headerHeight = 58;

    void setTab (Tab tab)
    {
        tab_ = tab;

        const bool audio      = tab == Tab::audio;
        const bool midi       = tab == Tab::midi;
        const bool appearance = tab == Tab::appearance;

        audioViewport_.setVisible (audio);

        midiLabel_.setVisible (midi);
        midiCombo_.setVisible (midi);
        refreshButton_.setVisible (midi);
        midiHint_.setVisible (midi);

        appearanceLabel_.setVisible (appearance);
        themeCombo_.setVisible (appearance);
        appearanceHint_.setVisible (appearance);
        languageLabel_.setVisible (appearance);
        languageCombo_.setVisible (appearance);
        languageHint_.setVisible (appearance);
        textSizeLabel_.setVisible (appearance);
        textSizeCombo_.setVisible (appearance);
        textSizeHint_.setVisible (appearance);

        updateTabColours();
        resized();
        repaint();
    }

    void updateTabColours()
    {
        const auto colours = themeColours (theme_);

        for (int i = 0; i < numTabs; ++i)
        {
            const bool selected = static_cast<int> (tab_) == i;

            tabButtons_[i].setColour (juce::TextButton::buttonColourId,
                                      selected ? colours.panelBorder : colours.panel);
            tabButtons_[i].setColour (juce::TextButton::buttonOnColourId,
                                      selected ? colours.accent : colours.panelBorder);
            tabButtons_[i].setColour (juce::TextButton::textColourOffId,
                                      selected ? juce::Colours::white : colours.muted);
        }
    }

    void applyThemeToChildren()
    {
        const auto colours = themeColours (theme_);

        titleLabel_.setColour (juce::Label::textColourId, colours.text);
        midiLabel_.setColour (juce::Label::textColourId, colours.text);
        appearanceLabel_.setColour (juce::Label::textColourId, colours.text);
        languageLabel_.setColour (juce::Label::textColourId, colours.text);
        textSizeLabel_.setColour (juce::Label::textColourId, colours.text);
        midiHint_.setColour (juce::Label::textColourId, colours.muted);
        appearanceHint_.setColour (juce::Label::textColourId, colours.muted);
        languageHint_.setColour (juce::Label::textColourId, colours.muted);
        textSizeHint_.setColour (juce::Label::textColourId, colours.muted);
        versionLabel_.setColour (juce::Label::textColourId, colours.muted);
        escLabel_.setColour (juce::Label::textColourId, colours.muted);
    }

    Tab tab_ = Tab::audio;
    ThemeId theme_ = ThemeId::holoAccents;
    juce::Rectangle<int> panelBounds_ {};
    juce::Rectangle<int> contentBounds_ {};

    juce::AudioDeviceSelectorComponent audioSelector_;
    juce::Viewport audioViewport_;

    juce::Label titleLabel_;
    IconButton closeButton_ { IconType::close, tr (TextId::settingsCloseTooltip) };

    juce::TextButton tabButtons_[numTabs];

    juce::Label midiLabel_;
    juce::ComboBox midiCombo_;
    juce::TextButton refreshButton_;
    juce::Label midiHint_;

    juce::Label appearanceLabel_;
    juce::ComboBox themeCombo_;
    juce::Label appearanceHint_;

    juce::Label languageLabel_;
    juce::ComboBox languageCombo_;
    juce::Label languageHint_;

    juce::Label textSizeLabel_;
    juce::ComboBox textSizeCombo_;
    juce::Label textSizeHint_;

    juce::Label versionLabel_;
    juce::Label escLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsOverlay)
};

} // namespace aeonix
