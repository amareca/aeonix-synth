#pragma once

#include <juce_core/juce_core.h>

#include <map>

namespace aeonix
{

// ============================================================================
//  PresetLibrary - preset library metadata (infrastructure).
//
//  Stores category, favorite and last use in a `library.json` sidecar
//  separate from the presets. Never writes inside each preset's JSON.
// ============================================================================

// Ten closed categories. The keys are stable and not translated; the
// visible labels live in the UI text catalog. The enum order is the order
// of the manager sidebar.
enum class PresetCategory
{
    initiation,
    bass,
    lead,
    pad,
    strings,
    brass,
    keys,
    pluck,
    fx,
    other
};

inline constexpr int presetCategoryCount = 10;

struct PresetMetadata
{
    PresetCategory category = PresetCategory::other;
    bool favorite = false;
    juce::int64 lastUsed = 0;

    // In memory only: the sidecar entry no longer has a file behind it.
    bool orphan = false;
};

class PresetLibrary
{
public:
    explicit PresetLibrary (juce::File sidecarFile);

    // Reads the sidecar. If missing or corrupt it starts from empty groups;
    // the factory categories still come from the static table.
    void load();

    // Atomic write: temporary file + replace.
    bool save() const;

    // Adds the missing entries for the user presets and marks as
    // orphans those that no longer have a file. Deletes nothing on its own.
    void reconcile (const juce::File& userPresetDirectory);

    [[nodiscard]] PresetMetadata metadataFor (const juce::String& name, bool factory) const;

    void setCategory (const juce::String& name, bool factory, PresetCategory category);
    void setFavorite (const juce::String& name, bool factory, bool favorite);
    void markUsed (const juce::String& name, bool factory, juce::int64 timestamp);

    // Renaming moves the key; deleting removes the entry.
    void renameUser (const juce::String& oldName, const juce::String& newName);
    void removeUser (const juce::String& name);

    [[nodiscard]] const std::map<juce::String, PresetMetadata>& factoryEntries() const noexcept
    {
        return factory_;
    }

    [[nodiscard]] const std::map<juce::String, PresetMetadata>& userEntries() const noexcept
    {
        return user_;
    }

    [[nodiscard]] static PresetCategory categoryFromKey (const juce::String& key);
    [[nodiscard]] static juce::String categoryKey (PresetCategory category);

private:
    [[nodiscard]] PresetMetadata& entryFor (const juce::String& name, bool factory);

    juce::File file_;
    std::map<juce::String, PresetMetadata> factory_;
    std::map<juce::String, PresetMetadata> user_;
};

// Category of the factory presets; `other` for any other name.
[[nodiscard]] PresetCategory factoryCategoryFor (const juce::String& name);

} // namespace aeonix
