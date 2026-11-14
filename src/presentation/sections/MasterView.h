#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "presentation/sections/SectionPanel.h"
#include "presentation/core/ViewModels.h"
#include "presentation/patch/ParameterMetadata.h"
#include "presentation/widgets/ComboControl.h"
#include "presentation/widgets/KnobView.h"
#include "presentation/sections/SectionLayout.h"

namespace aeonix
{

// ============================================================================
//  MasterView - master volume and computer keyboard octave.
// ============================================================================
class MasterView final : public SectionPanel
{
public:
    explicit MasterView (SynthViewModel& viewModel);

    void resized() override;

    // Octave change (SynthPanel receives it to notify MainComponent).
    std::function<void (int)> onOctaveChanged;

    // Polyphony change (8/16/32 voices).
    std::function<void (int)> onPolyphonyChanged;

    // Sets the octave without triggering onOctaveChanged.
    void setOctave (int octave);

    // Sets the polyphony without triggering onPolyphonyChanged.
    void setPolyphony (int voices);

private:
    ParameterKnob volume_;
    KnobView octave_;
    ComboControl polyphony_;
};

} // namespace aeonix
