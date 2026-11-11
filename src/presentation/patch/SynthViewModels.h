#pragma once

#include <utility>

#include "presentation/core/ViewModels.h"

namespace aeonix
{

// ============================================================================
//  SynthViewModels — agregado de los ViewModels de la interfaz.
//
//  Agrupa los ocho ViewModels para pasarlos como una sola dependencia al
//  PatchBinder y a las secciones, en lugar de repetir ocho parametros.
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
