#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "presentation/core/Observable.h"
#include "presentation/sections/SectionPanel.h"
#include "presentation/core/ViewModels.h"
#include "presentation/patch/ParameterMetadata.h"
#include "presentation/widgets/ComboControl.h"
#include "presentation/widgets/KnobView.h"
#include "presentation/sections/SectionLayout.h"
#include "presentation/sections/Waveforms.h"

namespace aeonix
{

// ============================================================================
//  Views of the synthesis block: oscillator, filter and envelopes.
//
//  Each view is a self-contained SectionPanel: it creates its controls,
//  subscribes to the ViewModels and lays itself out inside its own content.
// ============================================================================
class OscillatorView final : public SectionPanel
{
public:
    explicit OscillatorView (OscillatorViewModel& viewModel);

    void resized() override;

private:
    ComboControl waveform_;
    ParameterKnob level_;
    Observable<int>::Subscription waveformSubscription_;
};

class FilterView final : public SectionPanel
{
public:
    FilterView (FilterViewModel& filterViewModel,
                FilterEnvViewModel& filterEnvViewModel);

    void resized() override;

private:
    ParameterKnob cutoff_;
    ParameterKnob resonance_;
    ParameterKnob envAmount_;
};

class FilterEnvView final : public SectionPanel
{
public:
    explicit FilterEnvView (FilterEnvViewModel& viewModel);

    void resized() override;

private:
    ParameterKnob attack_;
    ParameterKnob decay_;
    ParameterKnob sustain_;
    ParameterKnob release_;
};

class AmpEnvView final : public SectionPanel
{
public:
    explicit AmpEnvView (EnvelopeViewModel& viewModel);

    void resized() override;

private:
    ParameterKnob attack_;
    ParameterKnob decay_;
    ParameterKnob sustain_;
    ParameterKnob release_;
};

} // namespace aeonix
