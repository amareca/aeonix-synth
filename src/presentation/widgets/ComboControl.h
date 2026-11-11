#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"

namespace aeonix
{

// ============================================================================
//  ComboControl — etiqueta + ComboBox con el mismo layout que los knobs.
// ============================================================================
class ComboControl final : public juce::Component
{
public:
    ComboControl (juce::String title,
                  juce::String tooltip,
                  const juce::StringArray& items)
    {
        label_.setText (title, juce::dontSendNotification);
        label_.setJustificationType (juce::Justification::centred);
        label_.setFont (ui::font (11.0f));
        label_.setName (title);
        addAndMakeVisible (label_);

        combo_.setName (title);
        combo_.setTitle (title);
        combo_.setTooltip (tooltip);

        for (int i = 0; i < items.size(); ++i)
            combo_.addItem (items[i], i + 1);

        combo_.onChange = [this]
        {
            if (onChange)
                onChange (combo_.getSelectedItemIndex());
        };
        addAndMakeVisible (combo_);
    }

    std::function<void (int)> onChange;

    void setSelectedIndex (int index,
                           juce::NotificationType notification = juce::sendNotificationSync)
    {
        if (index >= 0 && index < combo_.getNumItems())
            combo_.setSelectedItemIndex (index, notification);
    }

    [[nodiscard]] int getSelectedIndex() const
    {
        return combo_.getSelectedItemIndex();
    }

    juce::ComboBox& getComboBox() noexcept { return combo_; }
    juce::Label& getLabel() noexcept { return label_; }

    void setTheme (ThemeId theme)
    {
        juce::ignoreUnused (theme);
        repaint();
    }

    void resized() override
    {
        auto area = getLocalBounds();
        label_.setBounds (area.removeFromTop (ui::labelHeight()));
        combo_.setBounds (area.removeFromTop (ui::comboHeight()).reduced (2, 0));
    }

private:
    juce::Label label_;
    juce::ComboBox combo_;
};

} // namespace aeonix
