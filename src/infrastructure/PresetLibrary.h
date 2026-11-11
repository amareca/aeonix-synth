#pragma once

#include <juce_core/juce_core.h>

#include <map>

namespace aeonix
{

// ============================================================================
//  PresetLibrary — metadatos de la biblioteca de presets (infraestructura).
//
//  Guarda categoria, favorito y ultimo uso en un sidecar `library.json`
//  separado de los presets. Nunca escribe dentro del JSON de cada preset.
// ============================================================================

// Ocho categorias cerradas. Las claves son estables y no se traducen; las
// etiquetas visibles viven en el catalogo de textos de la interfaz.
enum class PresetCategory
{
    initiation,
    bass,
    lead,
    pad,
    strings,
    keys,
    fx,
    other
};

inline constexpr int presetCategoryCount = 8;

struct PresetMetadata
{
    PresetCategory category = PresetCategory::other;
    bool favorite = false;
    juce::int64 lastUsed = 0;

    // Solo en memoria: la entrada del sidecar ya no tiene archivo detras.
    bool orphan = false;
};

class PresetLibrary
{
public:
    explicit PresetLibrary (juce::File sidecarFile);

    // Lee el sidecar. Si falta o esta corrupto se parte de grupos vacios; las
    // categorias de fabrica siguen saliendo de la tabla estatica.
    void load();

    // Escritura atomica: archivo temporal + reemplazo.
    bool save() const;

    // Anade las entradas que falten para los presets de usuario y marca como
    // huerfanas las que ya no tienen archivo. No borra nada por su cuenta.
    void reconcile (const juce::File& userPresetDirectory);

    [[nodiscard]] PresetMetadata metadataFor (const juce::String& name, bool factory) const;

    void setCategory (const juce::String& name, bool factory, PresetCategory category);
    void setFavorite (const juce::String& name, bool factory, bool favorite);
    void markUsed (const juce::String& name, bool factory, juce::int64 timestamp);

    // Renombrar mueve la clave; borrar elimina la entrada.
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

// Categoria de los presets de fabrica; `other` para cualquier otro nombre.
[[nodiscard]] PresetCategory factoryCategoryFor (const juce::String& name);

} // namespace aeonix
