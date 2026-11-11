#include "PresetController.h"

#include "presentation/core/Utf8.h"

namespace aeonix
{

PresetController::PresetController (PresetBar& bar,
                                    PresetCatalog& catalog,
                                    PatchBinder& binder,
                                    juce::Component& dialogParent,
                                    std::function<void (const juce::String&)> onPresetLoaded,
                                    std::function<bool()> isDirty,
                                    std::function<void (const juce::String&)> notify)
    : bar_ (bar),
      catalog_ (catalog),
      binder_ (binder),
      dialogParent_ (dialogParent),
      onPresetLoaded_ (std::move (onPresetLoaded)),
      isDirty_ (std::move (isDirty)),
      notify_ (std::move (notify))
{
    bar_.onSelectIndex = [this] (int index) { requestLoadByIndex (index); };
    bar_.onPrev        = [this] { step (-1); };
    bar_.onNext        = [this] { step (1); };
}

PresetController::~PresetController()
{
    // La barra puede sobrevivir al controlador: no dejar callbacks colgando.
    bar_.onSelectIndex = nullptr;
    bar_.onPrev        = nullptr;
    bar_.onNext        = nullptr;
}

int PresetController::indexOf (const juce::String& name) const
{
    const auto& entries = catalog_.entries();

    for (int i = 0; i < static_cast<int> (entries.size()); ++i)
        if (entries[static_cast<size_t> (i)].name == name)
            return i;

    return -1;
}

void PresetController::showDialog (ThemedDialog::Style style,
                                   juce::String title,
                                   juce::String message,
                                   std::vector<ThemedDialogButton> buttons,
                                   std::function<void (int)> onResult)
{
    if (activeDialog_ != nullptr)
        return;

    activeDialog_ = ThemedDialog::show (dialogParent_, theme_, style,
                                        std::move (title), std::move (message),
                                        std::move (buttons),
                                        [this, callback = std::move (onResult)] (int result) mutable
                                        {
                                            activeDialog_ = nullptr;

                                            if (callback)
                                                callback (result);
                                        });
}

void PresetController::notify (const juce::String& message)
{
    if (notify_)
        notify_ (message);
}

void PresetController::refresh (const juce::String& selectName)
{
    catalog_.refresh();

    const auto& entries = catalog_.entries();
    const auto wanted = selectName.isNotEmpty() ? selectName : bar_.getSelectedName();

    juce::StringArray names;
    int selected = -1;

    for (int i = 0; i < static_cast<int> (entries.size()); ++i)
    {
        const auto& entry = entries[static_cast<size_t> (i)];
        names.add (entry.name);

        if (entry.name == wanted)
            selected = i;
    }

    if (selected < 0 && ! entries.empty())
        selected = 0;

    bar_.setEntries (names, selected);
}

void PresetController::selectByName (const juce::String& name, bool countAsRecent)
{
    const auto wanted = name.isNotEmpty() ? name : juce::String ("Default");
    const int index = indexOf (wanted);

    if (index < 0)
    {
        auto params = catalog_.load (wanted);

        if (params.empty())
            return;

        binder_.apply (params);
        loadedName_ = wanted;

        if (countAsRecent)
            catalog_.markUsed (wanted);

        if (onPresetLoaded_)
            onPresetLoaded_ (wanted);

        return;
    }

    applyLoad (wanted, index, countAsRecent);
}

void PresetController::requestLoadByName (const juce::String& name)
{
    const int index = indexOf (name);

    if (index >= 0)
    {
        requestLoadByIndex (index, true);
        return;
    }

    auto params = catalog_.load (name);

    if (params.empty())
        return;

    binder_.apply (params);
    loadedName_ = name;
    catalog_.markUsed (name);

    if (onPresetLoaded_)
        onPresetLoaded_ (name);
}

void PresetController::requestLoadByIndex (int index, bool forceReload, bool countAsRecent)
{
    const auto& entries = catalog_.entries();

    if (index < 0 || index >= static_cast<int> (entries.size()))
        return;

    const auto name = entries[static_cast<size_t> (index)].name;

    if (! forceReload && name == loadedName_)
        return;

    if (activeDialog_ != nullptr)
    {
        restoreSelection();
        return;
    }

    if (isDirty_ && isDirty_())
    {
        const bool factory = catalog_.isFactory (loadedName_);
        const auto message = factory
                                 ? trf (TextId::dialogUnsavedFactoryMessage, { loadedName_, name })
                                 : trf (TextId::dialogUnsavedNamedMessage, { loadedName_, name });

        showDialog (ThemedDialog::Style::warning,
                    tr (TextId::dialogUnsavedTitle),
                    message,
                    { { factory ? tr (TextId::presetManagerSaveCopy)
                               : tr (TextId::saveButton), 1, false },
                      { tr (TextId::buttonDiscard), 2, true },
                      { tr (TextId::buttonCancel), 0, false } },
                    [this, name, index, countAsRecent, factory] (int result)
                    {
                        if (result == 1)
                        {
                            if (factory)
                            {
                                // Fabrica: no se sobrescribe; se abre el
                                // formulario de copia y se cancela la carga.
                                if (onRequestSaveCopy)
                                    onRequestSaveCopy();

                                restoreSelection();
                                return;
                            }

                            saveCurrentPreset();
                            applyLoad (name, index, countAsRecent);
                        }
                        else if (result == 2)
                        {
                            applyLoad (name, index, countAsRecent);
                        }
                        else
                        {
                            restoreSelection();
                        }
                    });
        return;
    }

    applyLoad (name, index, countAsRecent);
}

void PresetController::applyLoad (const juce::String& name, int index, bool countAsRecent)
{
    auto params = catalog_.load (name);

    if (params.empty())
    {
        restoreSelection();
        return;
    }

    binder_.apply (params);
    bar_.setSelectedIndex (index, juce::dontSendNotification);
    loadedName_ = name;

    if (countAsRecent)
        catalog_.markUsed (name);

    if (onPresetLoaded_)
        onPresetLoaded_ (name);
}

void PresetController::restoreSelection()
{
    const int index = indexOf (loadedName_);

    if (index >= 0)
        bar_.setSelectedIndex (index, juce::dontSendNotification);
}

void PresetController::step (int delta)
{
    const int count = static_cast<int> (catalog_.entries().size());

    if (count == 0)
        return;

    int current = bar_.getSelectedIndex();
    if (current < 0)
        current = delta > 0 ? -1 : 0;

    const int next = ((current + delta) % count + count) % count;

    requestLoadByIndex (next);
}

bool PresetController::saveCurrentPreset()
{
    const auto name = loadedName_;

    if (name.isEmpty() || catalog_.isFactory (name) || ! catalog_.exists (name))
        return false;

    catalog_.save (name, binder_.capture());
    refresh (name);
    loadedName_ = name;

    notify (trf (TextId::toastPresetSaved, { name }));

    if (onPresetLoaded_)
        onPresetLoaded_ (name);

    return true;
}

bool PresetController::savePresetCopy (const juce::String& name, PresetCategory category)
{
    const auto stored = juce::File::createLegalFileName (name.trim());

    if (stored.isEmpty() || catalog_.isFactory (stored) || catalog_.exists (stored))
        return false;

    catalog_.save (stored, binder_.capture());
    catalog_.setCategory (stored, category);
    refresh (stored);
    loadedName_ = stored;

    notify (trf (TextId::toastPresetSaved, { stored }));

    if (onPresetLoaded_)
        onPresetLoaded_ (stored);

    return true;
}

bool PresetController::renamePreset (const juce::String& oldName, const juce::String& newName)
{
    const auto stored = juce::File::createLegalFileName (newName.trim());

    if (stored.isEmpty() || stored == oldName)
        return false;

    if (catalog_.isFactory (oldName) || catalog_.isFactory (stored) || catalog_.exists (stored))
        return false;

    const bool wasLoaded = oldName == loadedName_;

    if (! catalog_.rename (oldName, stored))
        return false;

    if (wasLoaded)
    {
        refresh (stored);
        loadedName_ = stored;

        if (onPresetLoaded_)
            onPresetLoaded_ (stored);
    }
    else
    {
        refresh (loadedName_);
    }

    notify (trf (TextId::toastPresetRenamed, { stored }));

    return true;
}

bool PresetController::deletePreset (const juce::String& name)
{
    if (name.isEmpty() || catalog_.isFactory (name))
        return false;

    const bool wasLoaded = name == loadedName_;
    const int index = indexOf (name);

    if (! catalog_.remove (name))
        return false;

    if (wasLoaded)
    {
        refresh();

        const auto& remaining = catalog_.entries();

        if (! remaining.empty())
        {
            const int selected = juce::jlimit (0, static_cast<int> (remaining.size()) - 1,
                                               index < 0 ? 0 : index);
            applyLoad (remaining[static_cast<size_t> (selected)].name, selected);
        }
    }
    else
    {
        refresh (loadedName_);
    }

    notify (trf (TextId::toastPresetDeleted, { name }));

    return true;
}

} // namespace aeonix
