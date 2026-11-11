#pragma once

#include <map>
#include <string>

#include "presentation/patch/SynthViewModels.h"

namespace aeonix
{

// ============================================================================
//  PatchBinder — puente entre un patch (mapa id -> valor normalizado) y los
//  ViewModels de la interfaz.
//
//  - capture(): lee el estado actual de los ViewModels.
//  - apply():    parte de los valores por defecto y aplica el patch recibido,
//                de modo que los presets antiguos queden completos.
// ============================================================================
class PatchBinder
{
public:
    explicit PatchBinder (SynthViewModels& models) : models_ (models) {}

    [[nodiscard]] std::map<std::string, float> capture() const;
    void apply (const std::map<std::string, float>& params);

private:
    SynthViewModels& models_;
};

} // namespace aeonix
