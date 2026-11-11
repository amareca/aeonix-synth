#include "PresetBar.h"

namespace aeonix
{

PresetBar::PresetBar()
{
    setTitle (tr (TextId::presetBarTitle));
    setDescription (tr (TextId::presetBarDescription));

    presetLabel_.setText (tr (TextId::presetLabel), juce::dontSendNotification);
    presetLabel_.setJustificationType (juce::Justification::centredLeft);
    presetLabel_.setFont (ui::font (11.0f));
    presetLabel_.setName ("Preset");
    addAndMakeVisible (presetLabel_);

    presetCombo_.setName ("Preset");
    presetCombo_.setTitle (tr (TextId::accessibilityPresetCombo));
    presetCombo_.setTooltip (tr (TextId::tooltipPresetCombo));
    presetCombo_.onChange = [this]
    {
        if (onSelectIndex)
            onSelectIndex (getSelectedIndex());
    };
    addAndMakeVisible (presetCombo_);

    prevButton_.setButtonText ("<");
    prevButton_.setName ("PrevPreset");
    prevButton_.setTooltip (tr (TextId::tooltipPrevPreset));
    prevButton_.onClick = [this] { if (onPrev) onPrev(); };
    addAndMakeVisible (prevButton_);

    nextButton_.setButtonText (">");
    nextButton_.setName ("NextPreset");
    nextButton_.setTooltip (tr (TextId::tooltipNextPreset));
    nextButton_.onClick = [this] { if (onNext) onNext(); };
    addAndMakeVisible (nextButton_);

    openPresetsButton_.setName ("OpenPresetsFolder");
    openPresetsButton_.onClick = [this] { if (onOpenFolder) onOpenFolder(); };
    addAndMakeVisible (openPresetsButton_);
}

void PresetBar::setEntries (const juce::StringArray& names, int selectedIndex)
{
    presetCombo_.clear (juce::dontSendNotification);

    for (int i = 0; i < names.size(); ++i)
        presetCombo_.addItem (names[i], i + 1);

    if (selectedIndex >= 0 && selectedIndex < names.size())
        presetCombo_.setSelectedId (selectedIndex + 1, juce::dontSendNotification);
    else
        presetCombo_.setSelectedId (0, juce::dontSendNotification);
}

void PresetBar::setSelectedIndex (int index, juce::NotificationType notification)
{
    if (index < 0 || index >= presetCombo_.getNumItems())
        return;

    presetCombo_.setSelectedId (index + 1, notification);
}

int PresetBar::getSelectedIndex() const
{
    const int id = presetCombo_.getSelectedId();
    return id > 0 ? id - 1 : -1;
}

juce::String PresetBar::getSelectedName() const
{
    return presetCombo_.getText();
}

void PresetBar::setModified (bool modified)
{
    if (modified_ == modified)
        return;

    modified_ = modified;
    repaint();
}

void PresetBar::setTheme (ThemeId theme)
{
    theme_ = theme;
    openPresetsButton_.setTheme (theme);

    const auto colours = themeColours (theme);
    presetLabel_.setColour (juce::Label::textColourId, colours.muted);

    repaint();
}

void PresetBar::paint (juce::Graphics& g)
{
    if (! modified_)
        return;

    g.setColour (juce::Colour (0xffe6c15a));
    g.fillEllipse (ledBounds_.toFloat().withSizeKeepingCentre (ui::px (6), ui::px (6)));
}

void PresetBar::resized()
{
    auto row = getLocalBounds().withSizeKeepingCentre (getWidth(), ui::presetRowHeight());

    presetLabel_.setBounds (row.removeFromLeft (ui::px (48)));
    presetCombo_.setBounds (row.removeFromLeft (ui::px (190)));
    row.removeFromLeft (ui::px (6));
    prevButton_.setBounds (row.removeFromLeft (ui::px (26)));
    row.removeFromLeft (ui::px (4));
    nextButton_.setBounds (row.removeFromLeft (ui::px (26)));
    row.removeFromLeft (ui::px (4));
    openPresetsButton_.setBounds (row.removeFromLeft (ui::px (26)));
    row.removeFromLeft (ui::px (4));
    ledBounds_ = row.removeFromLeft (ui::px (24));
}

} // namespace aeonix
