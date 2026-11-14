#pragma once

#include <utility>

#include "presentation/core/ViewModels.h"

namespace aeonix
{

// ============================================================================
//  SynthViewModels - aggregate of the interface ViewModels.
//
//  Groups the eight ViewModels so they can be passed as a single dependency to
//  PatchBinder and the sections, instead of repeating eight parameters.
// ============================================================================
struct SynthViewModels
{
    explicit SynthViewModels (SetParameterFn setParameter)
        : osc (setParameter),
          filter (setParameter),
          filterEnv (setParameter),
          amp (setParameter),
          synth (setParameter),
          fx (setParameter),
          lfo (setParameter),
          unison (setParameter)
    {
    }

    OscillatorViewModel osc;
    FilterViewModel     filter;
    FilterEnvViewModel  filterEnv;
    EnvelopeViewModel   amp;
    SynthViewModel      synth;
    FxViewModel         fx;
    LfoViewModel        lfo;
    UnisonViewModel     unison;
};

} // namespace aeonix
