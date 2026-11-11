#include "ThemedDialog.h"

#include "presentation/theme/Typography.h"

namespace aeonix
{

namespace
{

juce::Component& overlayHost (juce::Component& dialogParent)
{
    if (auto* top = dialogParent.getTopLevelComponent())
    {
        if (auto* document = dynamic_cast<juce::DocumentWindow*> (top))
        {
            if (auto* content = document->getContentComponent())
                return *content;
        }

        return *top;
    }

    return dialogParent;
}

} // namespace

ThemedDialog* ThemedDialog::show (juce::Component& dialogParent,
                                  ThemeId theme,
                                  Style style,
                                  juce::String title,
                                  juce::String message,
                                  std::vector<ThemedDialogButton> buttons,
                                  std::function<void (int)> onResult)
{
    auto& host = overlayHost (dialogParent);

    auto* dialog = new ThemedDialog (theme, style,
                                     std::move (title), std::move (message),
                                     std::move (buttons), std::move (onResult));

    host.addAndMakeVisible (dialog);
    dialog->setBounds (host.getLocalBounds());
    dialog->toFront (false);
    dialog->grabKeyboardFocus();
    return dialog;
}

ThemedDialog::ThemedDialog (ThemeId theme,
                            Style style,
                            juce::String title,
                            juce::String message,
                            std::vector<ThemedDialogButton> buttons,
                            std::function<void (int)> onResult)
    : theme_ (theme),
      style_ (style),
      title_ (std::move (title)),
      message_ (std::move (message)),
      onResult_ (std::move (onResult))
{
    setWantsKeyboardFocus (true);
    setTitle (title_);
    setDescription (message_);

    for (auto& spec : buttons)
    {
        auto button = std::make_unique<juce::TextButton> (spec.text);
        button->setName (spec.text);
        applyThemeToButton (*button, spec.destructive);

        const int result = spec.result;
        button->onClick = [this, result] { close (result); };

        addAndMakeVisible (*button);
        buttons_.push_back ({ std::move (button), result });

        if (result == 0)
            cancelResult_ = 0;
    }
}

void ThemedDialog::applyThemeToButton (juce::TextButton& button, bool destructive) const
{
    const auto colours = themeColours (theme_);

    button.setColour (juce::TextButton::buttonColourId,
                      destructive ? juce::Colour (0xff4a2422) : colours.panel);
    button.setColour (juce::TextButton::buttonOnColourId,
                      destructive ? juce::Colour (0xffb34a42) : colours.panelBorder);
    button.setColour (juce::TextButton::textColourOffId,
                      destructive ? juce::Colours::white : colours.text);
    button.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

void ThemedDialog::paint (juce::Graphics& g)
{
    const auto colours = themeColours (theme_);

    g.fillAll (juce::Colours::black.withAlpha (0.62f));

    const auto panel = panelBounds_.toFloat();

    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (panel.translated (0.0f, 8.0f), 12.0f);

    g.setColour (colours.panel);
    g.fillRoundedRectangle (panel, 12.0f);

    g.setColour (colours.panelBorder);
    g.drawRoundedRectangle (panel, 12.0f, 1.0f);

    const auto accent = style_ == Style::danger
                            ? juce::Colour (0xffc4433c)
                            : (style_ == Style::warning ? juce::Colour (0xffe6c15a)
                                                        : colours.accent);

    g.setColour (accent);
    g.fillRoundedRectangle (panel.getX() + ui::px (20), panel.getY() + ui::px (19),
                            ui::px (3), ui::px (16), 1.5f);

    g.setColour (colours.text);
    g.setFont (ui::font (16.0f, juce::Font::bold));
    g.drawText (title_,
                juce::Rectangle<int> (panelBounds_.getX() + ui::px (32),
                                      panelBounds_.getY() + ui::px (14),
                                      panelBounds_.getWidth() - ui::px (52),
                                      ui::px (28)),
                juce::Justification::centredLeft, true);

    const auto line = juce::Rectangle<int> (panelBounds_.getX() + ui::px (20),
                                            panelBounds_.getY() + ui::px (58),
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
        g.setColour (accent.withAlpha (0.85f));
        g.fillRect (line);
    }

    g.setColour (colours.text.withAlpha (0.92f));
    g.setFont (ui::font (13.0f));
    g.drawFittedText (message_,
                      juce::Rectangle<int> (panelBounds_.getX() + ui::px (20),
                                            panelBounds_.getY() + ui::px (72),
                                            panelBounds_.getWidth() - ui::px (40),
                                            panelBounds_.getHeight() - ui::px (72) - ui::px (68)),
                      juce::Justification::centredLeft, 4);
}

void ThemedDialog::resized()
{
    const int panelWidth  = juce::jmin (ui::px (460), juce::jmax (ui::px (300), getWidth() - ui::px (40)));
    const int panelHeight = juce::jmin (ui::px (230), juce::jmax (ui::px (190), getHeight() - ui::px (40)));

    panelBounds_ = getLocalBounds().withSizeKeepingCentre (panelWidth, panelHeight);

    const int buttonWidth  = ui::px (96);
    const int buttonHeight = ui::px (28);
    const int gap          = ui::px (8);

    const int count = static_cast<int> (buttons_.size());
    const int totalWidth = count * buttonWidth + juce::jmax (0, count - 1) * gap;

    int x = panelBounds_.getRight() - ui::px (20) - totalWidth;
    const int y = panelBounds_.getBottom() - ui::px (20) - buttonHeight;

    for (auto& entry : buttons_)
    {
        entry.button->setBounds (x, y, buttonWidth, buttonHeight);
        x += buttonWidth + gap;
    }
}

void ThemedDialog::mouseDown (const juce::MouseEvent& e)
{
    if (! panelBounds_.contains (e.getPosition()))
        close (cancelResult_);
}

bool ThemedDialog::keyPressed (const juce::KeyPress& key)
{
    if (key.getKeyCode() == juce::KeyPress::escapeKey
        || key.getKeyCode() == juce::KeyPress::returnKey)
    {
        close (cancelResult_);
        return true;
    }

    // Modal: consumir el resto para que el teclado QWERTY no toque el sintetizador.
    return true;
}

void ThemedDialog::parentSizeChanged()
{
    if (auto* parent = getParentComponent())
        setBounds (parent->getLocalBounds());
}

void ThemedDialog::close (int result)
{
    if (closed_)
        return;

    closed_ = true;
    setVisible (false);

    auto callback = std::move (onResult_);
    auto* parent = getParentComponent();

    if (parent != nullptr)
    {
        parent->removeChildComponent (this);
        parent->grabKeyboardFocus();
    }

    if (callback)
        callback (result);

    juce::MessageManager::callAsync ([this]
    {
        delete this;
    });
}

} // namespace aeonix
