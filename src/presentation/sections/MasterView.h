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
//  MasterView — volumen general y octava del teclado del ordenador.
// ============================================================================
class MasterView final : public SectionPanel
{
public:
    explicit MasterView (SynthViewModel& viewModel);

    void resized() override;

    // Cambio de octava (lo recibe SynthPanel para avisar a MainComponent).
    std::function<void (int)> onOctaveChanged;

    // Cambio de polifonía (8/16/32 voces).
    std::function<void (int)> onPolyphonyChanged;

    // Fija la octava sin disparar onOctaveChanged.
    void setOctave (int octave);

    // Fija la polifonía sin disparar onPolyphonyChanged.
    void setPolyphony (int voices);

private:
    ParameterKnob volume_;
    KnobView octave_;
    ComboControl polyphony_;
};

} // namespace aeonix
