#include "SynthesisViews.h"

namespace aeonix
{

// ============================================================================
//  OscillatorView
// ============================================================================

OscillatorView::OscillatorView (OscillatorViewModel& viewModel)
    : SectionPanel (tr (TextId::sectionOscillator), juce::Colour (0xff5aa9e6)),
      waveform_ (tr (TextId::labelWaveform), tr (TextId::tooltipOscWaveform), waveformNames()),
      level_ (parameterUiInfo (param::oscLevel),
              viewModel.level,
              [&viewModel] (float n) { viewModel.setLevel (n); },
              getAccentColour())
{
    waveform_.setSelectedIndex (viewModel.waveform.get(), juce::dontSendNotification);
    waveform_.onChange = [&viewModel] (int index)
    {
        viewModel.setWaveform (index);
    };
    addAndMakeVisible (waveform_);

    // Sincroniza el combo cuando la forma de onda cambie (por ejemplo, al
    // cargar un preset), igual que hacen los knobs con sus observables.
    waveformSubscription_ = viewModel.waveform.subscribe ([this] (const int& index)
    {
        waveform_.setSelectedIndex (index, juce::dontSendNotification);
    });

    addAndMakeVisible (level_);
}

void OscillatorView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 3);

    waveform_.setBounds (content.removeFromLeft (cell * 2));
    level_.setBounds (content.removeFromLeft (cell));
}

// ============================================================================
//  FilterView
// ============================================================================

FilterView::FilterView (FilterViewModel& filterViewModel,
                        FilterEnvViewModel& filterEnvViewModel)
    : SectionPanel (tr (TextId::sectionFilter), juce::Colour (0xff4ecdc4)),
      cutoff_ (parameterUiInfo (param::filterCutoff),
               filterViewModel.cutoff,
               [&filterViewModel] (float n) { filterViewModel.setCutoff (n); },
               getAccentColour()),
      resonance_ (parameterUiInfo (param::filterResonance),
                  filterViewModel.resonance,
                  [&filterViewModel] (float n) { filterViewModel.setResonance (n); },
                  getAccentColour()),
      envAmount_ (parameterUiInfo (param::filterEnvAmount),
                  filterEnvViewModel.amount,
                  [&filterEnvViewModel] (float n) { filterEnvViewModel.setAmount (n); },
                  getAccentColour())
{
    addAndMakeVisible (cutoff_);
    addAndMakeVisible (resonance_);
    addAndMakeVisible (envAmount_);
}

void FilterView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 3);
    layOutCells (content, cell, { &cutoff_, &resonance_, &envAmount_ });
}

// ============================================================================
//  FilterEnvView
// ============================================================================

FilterEnvView::FilterEnvView (FilterEnvViewModel& viewModel)
    : SectionPanel (tr (TextId::sectionFilterEnv), juce::Colour (0xfff6a54a)),
      attack_ (parameterUiInfo (param::filterAttack),
               viewModel.attack,
               [&viewModel] (float n) { viewModel.setAttack (n); },
               getAccentColour()),
      decay_ (parameterUiInfo (param::filterDecay),
              viewModel.decay,
              [&viewModel] (float n) { viewModel.setDecay (n); },
              getAccentColour()),
      sustain_ (parameterUiInfo (param::filterSustain),
                viewModel.sustain,
                [&viewModel] (float n) { viewModel.setSustain (n); },
                getAccentColour()),
      release_ (parameterUiInfo (param::filterRelease),
                viewModel.release,
                [&viewModel] (float n) { viewModel.setRelease (n); },
                getAccentColour())
{
    addAndMakeVisible (attack_);
    addAndMakeVisible (decay_);
    addAndMakeVisible (sustain_);
    addAndMakeVisible (release_);
}

void FilterEnvView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 4);
    layOutCells (content, cell, { &attack_, &decay_, &sustain_, &release_ });
}

// ============================================================================
//  AmpEnvView
// ============================================================================

AmpEnvView::AmpEnvView (EnvelopeViewModel& viewModel)
    : SectionPanel (tr (TextId::sectionAmpEnv), juce::Colour (0xffe8836b)),
      attack_ (parameterUiInfo (param::ampAttack),
               viewModel.attack,
               [&viewModel] (float n) { viewModel.setAttack (n); },
               getAccentColour()),
      decay_ (parameterUiInfo (param::ampDecay),
              viewModel.decay,
              [&viewModel] (float n) { viewModel.setDecay (n); },
              getAccentColour()),
      sustain_ (parameterUiInfo (param::ampSustain),
                viewModel.sustain,
                [&viewModel] (float n) { viewModel.setSustain (n); },
                getAccentColour()),
      release_ (parameterUiInfo (param::ampRelease),
                viewModel.release,
                [&viewModel] (float n) { viewModel.setRelease (n); },
                getAccentColour())
{
    addAndMakeVisible (attack_);
    addAndMakeVisible (decay_);
    addAndMakeVisible (sustain_);
    addAndMakeVisible (release_);
}

void AmpEnvView::resized()
{
    auto content = getContentBounds();
    const int cell = sectionCellWidth (content, 4);
    layOutCells (content, cell, { &attack_, &decay_, &sustain_, &release_ });
}

} // namespace aeonix
