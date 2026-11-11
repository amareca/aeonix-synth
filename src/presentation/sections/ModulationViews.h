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
//  Vistas del bloque de modulacion: LFO y unison.
// ============================================================================
class LfoView final : public SectionPanel
{
public:
    explicit LfoView (LfoViewModel& viewModel);

    void resized() override;

private:
    ParameterKnob rate_;
    ParameterKnob depth_;
    ComboControl waveform_;
    Observable<int>::Subscription waveformSubscription_;
};

class UnisonView final : public SectionPanel
{
public:
    explicit UnisonView (UnisonViewModel& viewModel);

    void resized() override;

private:
    ParameterKnob detune_;
    ParameterKnob width_;
    DiscreteKnob voices_;
};

} // namespace aeonix
