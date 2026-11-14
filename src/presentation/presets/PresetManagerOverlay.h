#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>
#include <vector>

#include "presentation/core/I18n.h"
#include "presentation/presets/PresetCatalog.h"
#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"
#include "presentation/widgets/IconButton.h"

namespace aeonix
{

// ============================================================================
//  PresetManagerOverlay - preset manager (overlay).
//
//  Covers the whole window with a scrim and a 900x620 card. In this first
//  version it allows opening, searching, browsing the list and loading a
//  preset. The category sidebar and the save actions arrive in the following
//  steps.
// ============================================================================
class PresetManagerOverlay final : public juce::Component
{
public:
    enum class SortMode
    {
        nameAsc,
        nameDesc,
        category,
        origin,
        lastUsed
    };

    PresetManagerOverlay();
    ~PresetManagerOverlay() override;

    // Opens the manager with the current catalog and the active preset.
    void show (const std::vector<PresetEntry>& entries,
               const juce::String& currentName,
               bool modified);

    // Refreshes the list after saving, renaming or deleting without closing the manager.
    void refreshEntries (const std::vector<PresetEntry>& entries,
                         const juce::String& currentName,
                         bool modified);
    void beginCopyForm();
    void hide();

    [[nodiscard]] bool isOpen() const noexcept { return isVisible(); }

    // Audition: loads on selection without closing the manager.
    void setAudition (bool enabled);
    [[nodiscard]] bool isAuditionEnabled() const;

    // List order. Persists in config.json as presets.sort.
    void setSortMode (SortMode mode, juce::NotificationType notification = juce::sendNotification);
    [[nodiscard]] SortMode getSortMode() const noexcept { return sortMode_; }
    [[nodiscard]] static juce::String sortKey (SortMode mode);
    [[nodiscard]] static SortMode sortModeFromKey (const juce::String& key);

    // Refreshes the current preset without touching the selection or the filter.
    void setCurrentPreset (const juce::String& name, bool modified);

    // Notifies an explicit load; closes only if "Cargar"/Enter/double click was requested.
    void notifyPresetLoaded (const juce::String& name);

    void setTheme (ThemeId theme);

    std::function<void (const juce::String&)> onLoad;
    std::function<void (const juce::String&)> onAudition;
    std::function<void (bool)> onAuditionChanged;
    std::function<void()> onOpenFolder;
    std::function<void()> onClosed;
    std::function<void (SortMode)> onSortChanged;
    std::function<void (const juce::String&, bool)> onFavorite;
    std::function<bool()> onSaveCurrent;
    std::function<bool (const juce::String&, PresetCategory)> onSaveCopy;
    std::function<bool (const juce::String&, const juce::String&)> onRename;
    std::function<bool (const juce::String&)> onDelete;

    // ---- juce::Component ----
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    enum class Filter
    {
        all,
        factory,
        user,
        favorites,
        recent,
        category
    };

    enum class FormMode
    {
        none,
        create,
        copy,
        rename,
        remove
    };

    // Flat sidebar row: label, counter and active state.
    class SidebarRow final : public juce::Button
    {
    public:
        SidebarRow();
        void setContent (const juce::String& text, int count, bool active,
                         juce::Colour markColour, bool dimmed = false);
        void setTheme (ThemeId theme);
        void paintButton (juce::Graphics& g, bool highlighted, bool down) override;

    private:
        juce::String text_;
        int count_ = 0;
        bool active_ = false;
        bool dimmed_ = false;
        juce::Colour markColour_ = juce::Colours::transparentBlack;
        ThemeId theme_ = ThemeId::holoAccents;
    };

    class ListModel;

    // Audition toggle with independent fill and border.
    class AuditionButton final : public juce::TextButton
    {
    public:
        void setTheme (ThemeId theme);
        void paintButton (juce::Graphics& g,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    private:
        ThemeId theme_ = ThemeId::holoAccents;
    };

    // Sort button: label and drawn chevron. "locked" dims it when
    // the filter fixes the order (Recientes) without disabling the tooltip.
    class SortButton final : public juce::TextButton
    {
    public:
        void setTheme (ThemeId theme);
        void setLocked (bool locked);
        void setOpen (bool open);
        void paintButton (juce::Graphics& g,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    private:
        ThemeId theme_ = ThemeId::holoAccents;
        bool locked_ = false;
        bool open_ = false;
    };

    // List scrollbar: only the thumb ink changes. The track, the click
    // area and the rest of the app scrollbars stay as they are.
    class ListScrollBarLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void setTheme (ThemeId theme);
        void drawScrollbar (juce::Graphics& g,
                            juce::ScrollBar& bar,
                            int x, int y, int width, int height,
                            bool isScrollbarVertical,
                            int thumbStartPosition,
                            int thumbSize,
                            bool isMouseOver,
                            bool isMouseDown) override;

    private:
        ThemeId theme_ = ThemeId::holoAccents;
    };

    // Modal layer for the forms: dims the panel and draws the card.
    class FormShield final : public juce::Component
    {
    public:
        void setTheme (ThemeId theme);
        void setCardBounds (juce::Rectangle<int> card);
        void paint (juce::Graphics& g) override;

    private:
        ThemeId theme_ = ThemeId::holoAccents;
        juce::Rectangle<int> card_;
    };

    void filter();
    void sortVisible();
    void setFilter (Filter filter, PresetCategory category = PresetCategory::other);
    void updateMeta();
    void updateSortButton();
    void showSortMenu();
    void updateSidebar();
    void updateDetail();
    void refreshDetailName();
    void updateActionColours();
    void updateFavoriteButton();
    void toggleFavorite();
    void openCopyForm();
    void openCreateForm();
    void openRenameForm();
    void openDeleteForm();
    void showForm (FormMode mode, const juce::String& target);
    void closeForm();
    void acceptForm();
    void updateFormLayout();
    [[nodiscard]] PresetCategory selectedFormCategory() const;
    void rowSelected (int row);
    void rowClicked (int row, const juce::MouseEvent& e);
    void focusNextInOverlay (bool backwards);
    void loadRow (int row);
    void loadSelected();
    void maybeAudition();
    void updateAuditionButton();
    void paintEmptyState (juce::Graphics& g);
    void paintListHeader (juce::Graphics& g);
    void paintDetail (juce::Graphics& g);
    void paintRow (int row, juce::Graphics& g, int width, int height, bool selected);
    void applyThemeToChildren();

    [[nodiscard]] bool matchesFilter (const PresetEntry& entry) const;
    [[nodiscard]] juce::String categoryName (PresetCategory category) const;
    [[nodiscard]] int visibleRowOf (const juce::String& name) const;
    [[nodiscard]] int filteredCount() const { return static_cast<int> (visible_.size()); }
    [[nodiscard]] const PresetEntry* findEntry (const juce::String& name) const;
    [[nodiscard]] juce::String rowName (int row) const;

    juce::Label titleLabel_;
    juce::TextEditor searchEditor_;
    IconButton closeButton_ { IconType::close, tr (TextId::presetManagerClose) };
    ListScrollBarLookAndFeel listScrollLookAndFeel_;
    juce::ListBox listBox_ { "Presets", nullptr };
    std::unique_ptr<ListModel> model_;
    juce::Label countLabel_;
    SortButton sortButton_;
    juce::Label libraryLabel_;
    juce::Label categoriesLabel_;
    juce::TextButton newButton_;
    SidebarRow allRow_;
    SidebarRow factoryRow_;
    SidebarRow userRow_;
    SidebarRow favoritesRow_;
    SidebarRow recentsRow_;
    SidebarRow categoryRows_[presetCategoryCount];
    juce::Label detailName_;
    juce::String detailFullName_;
    juce::Label detailCurrent_;
    juce::Label footerEsc_;
    AuditionButton auditionButton_;
    juce::TextButton favoriteButton_;
    juce::TextButton loadButton_;
    juce::TextButton saveButton_;
    juce::TextButton saveCopyButton_;
    juce::TextButton renameButton_;
    juce::TextButton deleteButton_;
    IconButton openFolderButton_ { IconType::folder,
                                   tr (TextId::presetManagerOpenFolderTooltip) };

    FormShield formShield_;
    juce::Label formTitle_;
    juce::Label formNameLabel_;
    juce::TextEditor formName_;
    juce::Label formCategoryLabel_;
    juce::ComboBox formCategory_;
    juce::Label formError_;
    juce::TextButton formAccept_;
    juce::TextButton formCancel_;
    FormMode formMode_ = FormMode::none;
    juce::String formTarget_;

    std::vector<PresetEntry> entries_;
    std::vector<int> visible_;
    juce::String selectedName_;
    juce::String currentName_;
    Filter filter_ = Filter::all;
    PresetCategory category_ = PresetCategory::other;
    SortMode sortMode_ = SortMode::origin;
    bool modified_ = false;
    bool updating_ = false;
    bool pendingClose_ = false;

    juce::Rectangle<int> panelBounds_;
    juce::Rectangle<int> sidebarBounds_;
    juce::Rectangle<int> sidebarSeparatorBounds_;
    juce::Rectangle<int> listBounds_;
    juce::Rectangle<int> listHeaderBounds_;
    juce::Rectangle<int> detailBounds_;
    juce::Rectangle<int> detailBadgesBounds_;
    juce::Rectangle<int> detailCardBounds_;
    juce::Rectangle<int> detailStatusDotBounds_;
    juce::Rectangle<int> footerTopLineBounds_;
    juce::Rectangle<int> listFadeBounds_;
    juce::Rectangle<int> formCardBounds_;
    int listContentWidth_ = 0;
    int listCategoryWidth_ = 0;
    int detailSeparatorY_ = 0;
    bool detailCurrentModified_ = false;

    ThemeId theme_ = ThemeId::holoAccents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManagerOverlay)
};

} // namespace aeonix
