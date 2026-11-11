#include "ModulationViews.h"

#include "presentation/core/Utf8.h"

namespace aeonix
{

// ============================================================================
//  LfoView
// ============================================================================

LfoView::LfoView (LfoViewModel& viewModel)
    : SectionPanel (tr (TextId::sectionLfo), juce::Colour (0xffa78bfa)),
      rate_ (parameterUiInfo (param::lfoRate),
             viewModel.rate,
             [&viewModel] (float n) { viewModel.setRate (n); },
             getAccentColour()),
      depth_ (parameterUiInfo (param::lfoDepth),
              viewModel.depth,
              [&viewModel] (float n) { viewModel.setDepth (n); },
              getAccentColour()),
      waveform_ (tr (TextId::labelWaveform), tr (TextId::tooltipLfoWaveform), waveformNames())
{
    // Orden de hijos identico al original: rate, depth y despues el combo.
    addAndMakeVisible (rate_);
    addAndMakeVisible (depth_);

    waveform_.getComboBox().setName ("LFO Waveform");
    waveform_.setSelectedIndex (viewModel.waveform.get(), juce::dontSendNotification);
    waveform_.onChange = [&viewModel] (int index)
    {
        viewModel.setWaveform (index);
    };
    addAndMakeVisible (waveform_);

    waveformSubscription_ = viewModel.waveform.subscribe ([this] (const int& index)
    {
        waveform_.setSelectedIndex (index, juce::dontSendNotification);
    });
}

void LfoView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 4);

    waveform_.setBounds (content.removeFromLeft (cell * 2));
    rate_.setBounds (content.removeFromLeft (cell));
    depth_.setBounds (content.removeFromLeft (cell));
}

// ============================================================================
//  UnisonView
// ============================================================================

UnisonView::UnisonView (UnisonViewModel& viewModel)
    : SectionPanel (tr (TextId::sectionUnison), juce::Colour (0xff7dd3a8)),
      detune_ (parameterUiInfo (param::unisonDetune),
               viewModel.detune,
               [&viewModel] (float n) { viewModel.setDetune (n); },
               getAccentColour()),
      width_ (parameterUiInfo (param::unisonWidth),
              viewModel.width,
              [&viewModel] (float n) { viewModel.setWidth (n); },
              getAccentColour()),
      voices_ (tr (TextId::paramVoices), tr (TextId::tipVoices), 1, 8,
               [] (int n) { return juce::String (n); },
               viewModel.voices,
               [&viewModel] (int n) { viewModel.setVoices (n); },
               getAccentColour())
{
    // Orden de hijos identico al original: detune, spread y despues voices.
    addAndMakeVisible (detune_);
    addAndMakeVisible (width_);
    addAndMakeVisible (voices_);
}

void UnisonView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 3);

    voices_.setBounds (content.removeFromLeft (cell));
    detune_.setBounds (content.removeFromLeft (cell));
    width_.setBounds (content.removeFromLeft (cell));
}

} // namespace aeonix
