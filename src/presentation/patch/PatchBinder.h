#pragma once

#include <map>
#include <string>

#include "presentation/patch/SynthViewModels.h"

namespace aeonix
{

// ============================================================================
//  PatchBinder - bridge between a patch (id -> normalised value map) and the
//  interface ViewModels.
//
//  - capture(): reads the current state of the ViewModels.
//  - apply():    starts from the default values and applies the received
//                patch, so old presets end up complete.
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
