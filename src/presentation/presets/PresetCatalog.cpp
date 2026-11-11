#include "PresetCatalog.h"

#include <algorithm>

#include "domain/Preset.h"

namespace aeonix
{

void PresetCatalog::refresh()
{
    library_.load();
    library_.reconcile (repository_.getDirectory());

    entries_.clear();

    std::vector<PresetEntry> factory;

    for (const auto& preset : makeFactoryPresets())
    {
        const juce::String name (preset.name);
        factory.push_back ({ name, true, library_.metadataFor (name, true) });
    }

    std::sort (factory.begin(), factory.end(),
               [] (const PresetEntry& left, const PresetEntry& right)
               {
                   return left.name.compareIgnoreCase (right.name) < 0;
               });

    entries_.insert (entries_.end(), factory.begin(), factory.end());

    for (const auto& name : repository_.listPresets())
        entries_.push_back ({ name, false, library_.metadataFor (name, false) });
}

bool PresetCatalog::isFactory (const juce::String& name) const
{
    for (const auto& preset : makeFactoryPresets())
        if (name == juce::String (preset.name))
            return true;

    return false;
}

bool PresetCatalog::exists (const juce::String& name) const
{
    return isFactory (name) || repository_.exists (name);
}

std::map<std::string, float> PresetCatalog::load (const juce::String& name) const
{
    for (const auto& preset : makeFactoryPresets())
        if (name == juce::String (preset.name))
            return preset.parameters;

    return repository_.load (name);
}

void PresetCatalog::save (const juce::String& name,
                          const std::map<std::string, float>& params) const
{
    repository_.save (name, params);
}

bool PresetCatalog::remove (const juce::String& name)
{
    if (! repository_.remove (name))
        return false;

    library_.removeUser (name);
    library_.save();
    return true;
}

bool PresetCatalog::rename (const juce::String& oldName,
                            const juce::String& newName)
{
    if (! repository_.rename (oldName, newName))
        return false;

    library_.renameUser (oldName, newName);
    library_.save();
    return true;
}

void PresetCatalog::setFavorite (const juce::String& name, bool favorite)
{
    library_.setFavorite (name, isFactory (name), favorite);
    library_.save();
    updateMetadata (name);
}

void PresetCatalog::setCategory (const juce::String& name, PresetCategory category)
{
    library_.setCategory (name, isFactory (name), category);
    library_.save();
    updateMetadata (name);
}

void PresetCatalog::markUsed (const juce::String& name)
{
    library_.markUsed (name, isFactory (name), juce::Time::currentTimeMillis());
    library_.save();
    updateMetadata (name);
}

void PresetCatalog::updateMetadata (const juce::String& name)
{
    for (auto& entry : entries_)
    {
        if (entry.name == name)
        {
            entry.metadata = library_.metadataFor (name, entry.factory);
            return;
        }
    }
}

} // namespace aeonix
