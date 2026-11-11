#include "EffectViews.h"

namespace aeonix
{

// ============================================================================
//  DelayView
// ============================================================================

DelayView::DelayView (FxViewModel& viewModel)
    : SectionPanel (tr (TextId::sectionDelay), juce::Colour (0xffb5d56a)),
      time_ (parameterUiInfo (param::delayTime),
             viewModel.delayTime,
             [&viewModel] (float n) { viewModel.setDelayTime (n); },
             getAccentColour()),
      feedback_ (parameterUiInfo (param::delayFeedback),
                 viewModel.delayFeedback,
                 [&viewModel] (float n) { viewModel.setDelayFeedback (n); },
                 getAccentColour()),
      mix_ (parameterUiInfo (param::delayMix),
            viewModel.delayMix,
            [&viewModel] (float n) { viewModel.setDelayMix (n); },
            getAccentColour())
{
    addAndMakeVisible (time_);
    addAndMakeVisible (feedback_);
    addAndMakeVisible (mix_);
}

void DelayView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 3);
    layOutCells (content, cell, { &time_, &feedback_, &mix_ });
}

// ============================================================================
//  ReverbView
// ============================================================================

ReverbView::ReverbView (FxViewModel& viewModel)
    : SectionPanel (tr (TextId::sectionReverb), juce::Colour (0xffb5d56a)),
      size_ (parameterUiInfo (param::reverbSize),
             viewModel.reverbSize,
             [&viewModel] (float n) { viewModel.setReverbSize (n); },
             getAccentColour()),
      mix_ (parameterUiInfo (param::reverbMix),
            viewModel.reverbMix,
            [&viewModel] (float n) { viewModel.setReverbMix (n); },
            getAccentColour())
{
    addAndMakeVisible (size_);
    addAndMakeVisible (mix_);
}

void ReverbView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 2);
    layOutCells (content, cell, { &size_, &mix_ });
}

} // namespace aeonix
