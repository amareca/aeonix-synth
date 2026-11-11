#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "presentation/sections/SectionPanel.h"
#include "presentation/core/ViewModels.h"
#include "presentation/patch/ParameterMetadata.h"
#include "presentation/widgets/KnobView.h"
#include "presentation/sections/SectionLayout.h"

namespace aeonix
{

// ============================================================================
//  Vistas del bloque de efectos: delay y reverb.
// ============================================================================
class DelayView final : public SectionPanel
{
public:
    explicit DelayView (FxViewModel& viewModel);

    void resized() override;

private:
    ParameterKnob time_;
    ParameterKnob feedback_;
    ParameterKnob mix_;
};

class ReverbView final : public SectionPanel
{
public:
    explicit ReverbView (FxViewModel& viewModel);

    void resized() override;

private:
    ParameterKnob size_;
    ParameterKnob mix_;
};

} // namespace aeonix
