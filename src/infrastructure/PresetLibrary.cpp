#include "PresetLibrary.h"

#include <utility>

namespace aeonix
{
namespace
{

juce::int64 readLastUsed (const juce::var& value)
{
    if (value.isInt() || value.isInt64() || value.isDouble())
        return static_cast<juce::int64> (value);

    return 0;
}

void readGroup (const juce::var& group,
                std::map<juce::String, PresetMetadata>& target,
                bool factory)
{
    auto* object = group.getDynamicObject();

    if (object == nullptr)
        return;

    for (const auto& entry : object->getProperties())
    {
        const auto name = entry.name.toString();
        PresetMetadata meta;

        if (auto* values = entry.value.getDynamicObject())
        {
            meta.category = values->hasProperty ("category")
                              ? PresetLibrary::categoryFromKey (values->getProperty ("category").toString())
                              : (factory ? factoryCategoryFor (name) : PresetCategory::other);
            meta.favorite = static_cast<bool> (values->getProperty ("favorite"));
            meta.lastUsed = readLastUsed (values->getProperty ("lastUsed"));
        }
        else if (factory)
        {
            meta.category = factoryCategoryFor (name);
        }

        target[name] = meta;
    }
}

juce::var writeGroup (const std::map<juce::String, PresetMetadata>& group)
{
    juce::DynamicObject::Ptr object = new juce::DynamicObject();

    for (const auto& [name, meta] : group)
    {
        juce::DynamicObject::Ptr entry = new juce::DynamicObject();
        entry->setProperty ("category", PresetLibrary::categoryKey (meta.category));
        entry->setProperty ("favorite", meta.favorite);

        if (meta.lastUsed > 0)
            entry->setProperty ("lastUsed", meta.lastUsed);

        object->setProperty (name, juce::var (entry.get()));
    }

    return juce::var (object.get());
}

} // namespace

PresetLibrary::PresetLibrary (juce::File sidecarFile)
    : file_ (std::move (sidecarFile))
{
}

void PresetLibrary::load()
{
    factory_.clear();
    user_.clear();

    if (! file_.existsAsFile())
        return;

    const auto root = juce::JSON::parse (file_);
    auto* object = root.getDynamicObject();

    if (object == nullptr)
        return;

    readGroup (object->getProperty ("factory"), factory_, true);
    readGroup (object->getProperty ("user"), user_, false);
}

bool PresetLibrary::save() const
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty ("version", 1);
    root->setProperty ("factory", writeGroup (factory_));
    root->setProperty ("user", writeGroup (user_));

    const auto json = juce::JSON::toString (juce::var (root.get()), false);
    const auto temporary = file_.getSiblingFile (file_.getFileName() + ".tmp");

    if (! temporary.replaceWithText (json))
        return false;

    if (! temporary.replaceFileIn (file_))
    {
        temporary.deleteFile();
        return false;
    }

    return true;
}

void PresetLibrary::reconcile (const juce::File& userPresetDirectory)
{
    juce::StringArray names;

    for (const auto& file : userPresetDirectory.findChildFiles (juce::File::findFiles, false, "*.json"))
        names.add (file.getFileNameWithoutExtension());

    for (const auto& name : names)
        if (user_.find (name) == user_.end())
            user_[name] = PresetMetadata();

    for (auto& entry : user_)
        entry.second.orphan = ! names.contains (entry.first);
}

PresetMetadata PresetLibrary::metadataFor (const juce::String& name, bool factory) const
{
    const auto& group = factory ? factory_ : user_;
    const auto it = group.find (name);

    if (it != group.end())
        return it->second;

    PresetMetadata meta;

    if (factory)
        meta.category = factoryCategoryFor (name);

    return meta;
}

void PresetLibrary::setCategory (const juce::String& name, bool factory, PresetCategory category)
{
    entryFor (name, factory).category = category;
}

void PresetLibrary::setFavorite (const juce::String& name, bool factory, bool favorite)
{
    entryFor (name, factory).favorite = favorite;
}

void PresetLibrary::markUsed (const juce::String& name, bool factory, juce::int64 timestamp)
{
    entryFor (name, factory).lastUsed = timestamp;
}

void PresetLibrary::renameUser (const juce::String& oldName, const juce::String& newName)
{
    if (oldName == newName)
        return;

    const auto it = user_.find (oldName);

    if (it == user_.end())
        return;

    const auto meta = it->second;
    user_.erase (it);
    user_[newName] = meta;
}

void PresetLibrary::removeUser (const juce::String& name)
{
    user_.erase (name);
}

PresetCategory PresetLibrary::categoryFromKey (const juce::String& key)
{
    if (key == "iniciacion") return PresetCategory::initiation;
    if (key == "bajo")       return PresetCategory::bass;
    if (key == "lead")       return PresetCategory::lead;
    if (key == "pad")        return PresetCategory::pad;
    if (key == "cuerdas")    return PresetCategory::strings;
    if (key == "teclas")     return PresetCategory::keys;
    if (key == "fx")         return PresetCategory::fx;

    return PresetCategory::other;
}

juce::String PresetLibrary::categoryKey (PresetCategory category)
{
    switch (category)
    {
        case PresetCategory::initiation: return "iniciacion";
        case PresetCategory::bass:       return "bajo";
        case PresetCategory::lead:       return "lead";
        case PresetCategory::pad:        return "pad";
        case PresetCategory::strings:    return "cuerdas";
        case PresetCategory::keys:       return "teclas";
        case PresetCategory::fx:         return "fx";
        case PresetCategory::other:      return "otros";
    }

    return "otros";
}

PresetMetadata& PresetLibrary::entryFor (const juce::String& name, bool factory)
{
    auto& group = factory ? factory_ : user_;
    auto it = group.find (name);

    if (it == group.end())
    {
        PresetMetadata meta;

        if (factory)
            meta.category = factoryCategoryFor (name);

        it = group.emplace (name, meta).first;
    }

    return it->second;
}

PresetCategory factoryCategoryFor (const juce::String& name)
{
    if (name == "Default") return PresetCategory::initiation;

    // La biblioteca v1 usa el sufijo de familia en el nombre visible.
    if (name.endsWith (" Pad"))     return PresetCategory::pad;
    if (name.endsWith (" Pluck"))   return PresetCategory::keys;
    if (name.endsWith (" Lead"))    return PresetCategory::lead;
    if (name.endsWith (" Bass"))    return PresetCategory::bass;
    if (name.endsWith (" Brass"))   return PresetCategory::other;
    if (name.endsWith (" Piano"))   return PresetCategory::keys;
    if (name.endsWith (" Strings")) return PresetCategory::strings;

    return PresetCategory::other;
}

} // namespace aeonix
