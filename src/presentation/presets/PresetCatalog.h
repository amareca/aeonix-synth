#pragma once

#include <juce_core/juce_core.h>

#include <map>
#include <string>
#include <vector>

#include "infrastructure/PresetLibrary.h"
#include "infrastructure/PresetRepository.h"

namespace aeonix
{

struct PresetEntry
{
    juce::String name;
    bool factory = false;
    PresetMetadata metadata;
};

// ============================================================================
//  PresetCatalog — reglas de los presets (fabrica + usuario).
//
//  Centraliza la lista, la proteccion de los presets de fabrica y la
//  carga/guardado/borrado/renombrado. No conoce la UI ni los dialogos.
// ============================================================================
class PresetCatalog
{
public:
    explicit PresetCatalog (PresetRepository& repository)
        : repository_ (repository),
          library_ (repository.getDirectory().getParentDirectory().getChildFile ("library.json"))
    {
    }

    void refresh();

    [[nodiscard]] const std::vector<PresetEntry>& entries() const noexcept
    {
        return entries_;
    }

    [[nodiscard]] bool isFactory (const juce::String& name) const;
    [[nodiscard]] bool exists (const juce::String& name) const;

    [[nodiscard]] std::map<std::string, float> load (const juce::String& name) const;
    void save (const juce::String& name, const std::map<std::string, float>& params) const;
    bool remove (const juce::String& name);
    bool rename (const juce::String& oldName, const juce::String& newName);

    // Metadatos de la biblioteca (categoria, favorito y ultimo uso).
    void setFavorite (const juce::String& name, bool favorite);
    void setCategory (const juce::String& name, PresetCategory category);
    void markUsed (const juce::String& name);

    [[nodiscard]] const PresetLibrary& library() const noexcept { return library_; }

private:
    void updateMetadata (const juce::String& name);

    PresetRepository& repository_;
    PresetLibrary library_;
    std::vector<PresetEntry> entries_;
};

} // namespace aeonix
