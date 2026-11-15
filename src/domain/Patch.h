#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Parameter.h"

namespace aeonix
{

// ============================================================================
//  Patch - the "preset": a collection of Parameter (definition + value).
//
//  Holds the synthesizer parameter set and its current normalized
//  value. It is a snapshot of the instrument state.
// ============================================================================
class Patch
{
public:
    Patch() = default;

    void add (Parameter p) { params_.push_back (std::move (p)); }

    // Returns true if the parameter exists.
    [[nodiscard]] bool contains (const std::string& id) const
    {
        return find (id) != nullptr;
    }

    // Normalized value of a parameter (0.0 if it does not exist).
    [[nodiscard]] float get (const std::string& id) const
    {
        const auto* p = find (id);
        return p != nullptr ? p->getNormalised() : 0.0f;
    }

    // Real value (in the parameter range). Returns min if it does not exist.
    [[nodiscard]] float getValue (const std::string& id) const
    {
        const auto* p = find (id);
        return p != nullptr ? p->getValue() : p->getMin();
    }

    // Sets the normalized value. Returns false if the id does not exist.
    bool set (const std::string& id, float normalised)
    {
        auto* p = find (id);
        if (p == nullptr)
            return false;

        p->setNormalised (normalised);
        return true;
    }

    [[nodiscard]] size_t size() const { return params_.size(); }

    // Access to all parameters (for serializing presets, etc.).
    [[nodiscard]] const std::vector<Parameter>& parameters() const { return params_; }

    [[nodiscard]] const Parameter* find (const std::string& id) const
    {
        for (const auto& p : params_)
            if (p.getId() == id)
                return &p;

        return nullptr;
    }

    // Non-const version: returns a mutable pointer (used by the engine to
    // cache the parameter pointers).
    [[nodiscard]] Parameter* find (const std::string& id)
    {
        for (auto& p : params_)
            if (p.getId() == id)
                return &p;

        return nullptr;
    }

private:
    std::vector<Parameter> params_;
};

// Creates the default Patch with the parameters used in Phase 1.
Patch makeDefaultPatch();

} // namespace aeonix
