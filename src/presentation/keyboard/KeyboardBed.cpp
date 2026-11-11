#include "KeyboardBed.h"

namespace aeonix
{

KeyboardBed::KeyboardBed (juce::MidiKeyboardState& state)
    : keyboard_ (state, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setTitle (tr (TextId::keyboardTitle));
    setDescription (tr (TextId::keyboardDescription));

    keyboard_.setName ("Keyboard");
    keyboard_.setAvailableRange (36, 84);
    keyboard_.setLowestVisibleKey (36);
    addAndMakeVisible (keyboard_);
}

void KeyboardBed::setTheme (ThemeId theme)
{
    theme_ = theme;
    keyboard_.setTheme (theme);
    repaint();
}

void KeyboardBed::paint (juce::Graphics& g)
{
    const auto keys = keyboard_.getBounds().toFloat();

    if (keys.getWidth() <= 0.0f || keys.getHeight() <= 0.0f)
        return;

    // Fieltro + marco del keybed (el teclado se pinta encima como hijo).
    const auto felt = juce::Rectangle<float> (keys.getX() - 6.0f,
                                              keys.getY() - 8.0f,
                                              keys.getWidth() + 12.0f,
                                              8.0f);

    const auto frame = juce::Rectangle<float> (felt.getX(),
                                               felt.getY(),
                                               felt.getWidth(),
                                               keys.getHeight() + 14.0f);

    const auto colours = themeColours (theme_);
    const auto frameColour = theme_ == ThemeId::holoGlass
                                 ? juce::Colour (0xff18212b)
                                 : juce::Colour (0xff20252d);
    const auto feltColour = theme_ == ThemeId::holoMinimal
                                ? juce::Colour (0xff2a2f38)
                                : juce::Colour (0xff4a2c2a);

    g.setColour (juce::Colour (0x40000000));
    g.fillRoundedRectangle (frame.translated (0.0f, 3.0f), 10.0f);

    g.setColour (frameColour);
    g.fillRoundedRectangle (frame, 10.0f);

    if (theme_ != ThemeId::prototype)
    {
        g.setColour (colours.panelBorder.withAlpha (0.5f));
        g.drawRoundedRectangle (frame, 10.0f, 1.0f);
    }

    juce::Path feltPath;
    feltPath.addRoundedRectangle (felt.getX(), felt.getY(), felt.getWidth(), felt.getHeight(),
                                  4.0f, 4.0f, true, true, false, false);
    g.setColour (feltColour);
    g.fillPath (feltPath);

    g.setColour (juce::Colour (0x18ffffff));
    g.fillRect (felt.getX() + 2.0f, felt.getY() + 1.0f, felt.getWidth() - 4.0f, 1.0f);

    g.setColour (juce::Colour (0xaa000000));
    g.fillRect (felt.getX(), felt.getBottom() - 1.0f, felt.getWidth(), 1.0f);
}

void KeyboardBed::resized()
{
    keyboard_.setBounds (getLocalBounds()
                             .withTrimmedLeft (frameLeft)
                             .withTrimmedTop (frameTop)
                             .withTrimmedRight (frameRight)
                             .withTrimmedBottom (frameBottom));
}

} // namespace aeonix
