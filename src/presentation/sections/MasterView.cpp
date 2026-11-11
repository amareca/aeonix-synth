#include "MasterView.h"

namespace aeonix
{

namespace
{

juce::StringArray polyphonyOptions()
{
    juce::StringArray options;
    options.add ("8");
    options.add ("16");
    options.add ("32");
    return options;
}

} // namespace

MasterView::MasterView (SynthViewModel& viewModel)
    : SectionPanel (tr (TextId::sectionMaster), juce::Colour (0xffe6c15a)),
      volume_ (parameterUiInfo (param::masterVolume),
               viewModel.masterVolume,
               [&viewModel] (float n) { viewModel.setMasterVolume (n); },
               getAccentColour()),
      octave_ (tr (TextId::labelOctave),
               tr (TextId::tooltipOctave),
               -2.0, 2.0, 1.0,
               [] (double value)
               {
                   const int octave = juce::roundToInt (value);
                   return (octave > 0 ? "+" : "") + juce::String (octave);
               },
               getAccentColour(),
               46,
               true),
      polyphony_ (tr (TextId::labelPolyphony),
                  tr (TextId::tooltipPolyphony),
                  polyphonyOptions())
{
    addAndMakeVisible (volume_);

    octave_.setValue (0.0);
    octave_.onValueChange = [this] (double value)
    {
        if (onOctaveChanged)
            onOctaveChanged (juce::roundToInt (value));
    };
    addAndMakeVisible (octave_);

    polyphony_.setSelectedIndex (1, juce::dontSendNotification);   // 16 por defecto
    polyphony_.onChange = [this] (int index)
    {
        const int voices = index == 0 ? 8 : (index == 1 ? 16 : 32);

        if (onPolyphonyChanged)
            onPolyphonyChanged (voices);
    };
    addAndMakeVisible (polyphony_);
}

void MasterView::setOctave (int octave)
{
    octave_.setValue (octave, juce::dontSendNotification);
}

void MasterView::setPolyphony (int voices)
{
    const int index = voices <= 8 ? 0 : (voices <= 16 ? 1 : 2);
    polyphony_.setSelectedIndex (index, juce::dontSendNotification);
}

void MasterView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 3);
    layOutCells (content, cell, { &volume_, &octave_, &polyphony_ });
}

} // namespace aeonix
