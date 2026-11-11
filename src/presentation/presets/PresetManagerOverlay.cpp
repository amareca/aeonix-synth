#include "PresetManagerOverlay.h"

#include <algorithm>

namespace
{
constexpr int recentLimit = 20;

juce::Colour categoryColour (aeonix::PresetCategory category,
                             const aeonix::ThemeColours& colours)
{
    switch (category)
    {
        case aeonix::PresetCategory::initiation: return juce::Colour (0xff4ecdc4);
        case aeonix::PresetCategory::bass:       return juce::Colour (0xff5aa9e6);
        case aeonix::PresetCategory::lead:       return juce::Colour (0xfff6a54a);
        case aeonix::PresetCategory::pad:        return juce::Colour (0xffa78bfa);
        case aeonix::PresetCategory::strings:    return juce::Colour (0xff7dd3a8);
        case aeonix::PresetCategory::keys:       return juce::Colour (0xffe6c15a);
        case aeonix::PresetCategory::fx:         return juce::Colour (0xffe8836b);
        case aeonix::PresetCategory::other:      return colours.muted;
    }

    return colours.muted;
}

juce::Colour badgeTextColour (juce::Colour colour, const aeonix::ThemeColours& colours)
{
    return colour.interpolatedWith (colours.text, 0.30f);
}

int badgeWidth (const juce::String& text)
{
    return juce::roundToInt (juce::GlyphArrangement::getStringWidth (aeonix::ui::font (10.0f), text))
         + 2 * aeonix::ui::px (7);
}

void paintBadge (juce::Graphics& g,
                 juce::Rectangle<int> bounds,
                 const juce::String& text,
                 juce::Colour colour,
                 const aeonix::ThemeColours& colours,
                 bool filled)
{
    constexpr float radius = 4.0f;

    if (filled)
    {
        g.setColour (colour.withAlpha (0.11f));
        g.fillRoundedRectangle (bounds.toFloat(), radius);
    }

    g.setColour (colour.withAlpha (filled ? 0.42f : 0.45f));
    g.drawRoundedRectangle (bounds.toFloat(), radius, 1.0f);

    g.setColour (badgeTextColour (colour, colours));
    g.setFont (aeonix::ui::font (10.0f));
    g.drawFittedText (text, bounds.reduced (aeonix::ui::px (6), 0),
                      juce::Justification::centred, 1);
}
}


namespace aeonix
{

// ============================================================================
//  Fila del lateral
// ============================================================================

PresetManagerOverlay::SidebarRow::SidebarRow()
    : juce::Button ("")
{
    setWantsKeyboardFocus (true);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void PresetManagerOverlay::SidebarRow::setContent (const juce::String& text,
                                                   int count,
                                                   bool active,
                                                   juce::Colour markColour,
                                                   bool dimmed)
{
    text_ = text;
    count_ = count;
    active_ = active;
    dimmed_ = dimmed;
    markColour_ = markColour;
    setName (text);
    setTitle (text);
    setDescription (text + " (" + juce::String (count_) + ")");
    repaint();
}

void PresetManagerOverlay::SidebarRow::setTheme (ThemeId theme)
{
    theme_ = theme;
    repaint();
}

void PresetManagerOverlay::SidebarRow::paintButton (juce::Graphics& g,
                                                    bool highlighted,
                                                    bool down)
{
    const auto colours = themeColours (theme_);
    const auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    if (active_)
    {
        g.setColour (colours.accent.withAlpha (0.14f));
        g.fillRoundedRectangle (bounds, 5.0f);
        g.setColour (colours.accent);
        g.fillRect (0, 1, ui::px (2), juce::jmax (1, getHeight() - 2));
    }
    else if (highlighted || down)
    {
        g.setColour (colours.panelBorder.withAlpha (0.30f));
        g.fillRoundedRectangle (bounds, 5.0f);
    }

    if (hasKeyboardFocus (false))
    {
        g.setColour (colours.accent.withAlpha (0.90f));
        g.drawRoundedRectangle (bounds, 5.0f, 1.0f);
    }

    auto area = getLocalBounds().reduced (ui::px (8), 0);
    auto count = area.removeFromRight (ui::px (32));
    auto dotBox = area.removeFromLeft (ui::px (12));
    area.removeFromLeft (ui::px (6));

    const auto dotSize = static_cast<float> (ui::px (5));
    g.setColour (markColour_.withMultipliedAlpha (dimmed_ ? 0.45f : 1.0f));
    g.fillEllipse (dotBox.toFloat().withSizeKeepingCentre (dotSize, dotSize));

    g.setFont (ui::font (11.0f, active_ ? juce::Font::bold : juce::Font::plain));
    g.setColour (dimmed_ ? colours.muted.withAlpha (0.55f)
                         : (active_ ? colours.text : colours.muted));
    g.drawFittedText (text_, area, juce::Justification::centredLeft, 1);

    g.setFont (ui::font (10.0f));
    g.setColour (dimmed_ ? colours.muted.withAlpha (0.45f)
                         : (active_ ? colours.text : colours.muted.withAlpha (0.9f)));
    g.drawFittedText (juce::String (count_), count, juce::Justification::centredRight, 1);
}

// ============================================================================
//  Formularios
// ============================================================================

void PresetManagerOverlay::FormShield::setTheme (ThemeId theme)
{
    theme_ = theme;
    repaint();
}

void PresetManagerOverlay::FormShield::setCardBounds (juce::Rectangle<int> card)
{
    card_ = card;
    repaint();
}

void PresetManagerOverlay::FormShield::paint (juce::Graphics& g)
{
    const auto colours = themeColours (theme_);

    g.fillAll (juce::Colours::black.withAlpha (0.55f));

    g.setColour (colours.backgroundAlt);
    g.fillRoundedRectangle (card_.toFloat(), 10.0f);

    g.setColour (colours.panelBorder);
    g.drawRoundedRectangle (card_.toFloat(), 10.0f, 1.0f);
}

// ============================================================================
//  Modelo de la lista
// ============================================================================

class PresetManagerOverlay::ListModel final : public juce::ListBoxModel
{
public:
    explicit ListModel (PresetManagerOverlay& owner)
        : owner_ (owner)
    {
    }

    int getNumRows() override
    {
        return owner_.filteredCount();
    }

    void paintListBoxItem (int rowNumber, juce::Graphics& g,
                           int width, int height, bool rowIsSelected) override
    {
        owner_.paintRow (rowNumber, g, width, height, rowIsSelected);
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        owner_.rowSelected (lastRowSelected);
    }

    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override
    {
        owner_.loadRow (row);
    }

    void listBoxItemClicked (int row, const juce::MouseEvent& e) override
    {
        owner_.rowClicked (row, e);
    }

    juce::String getNameForRow (int rowNumber) override
    {
        return owner_.rowName (rowNumber);
    }

private:
    PresetManagerOverlay& owner_;
};

// ============================================================================
//  Construccion
// ============================================================================

PresetManagerOverlay::~PresetManagerOverlay() = default;

PresetManagerOverlay::PresetManagerOverlay()
{
    setVisible (false);
    setWantsKeyboardFocus (true);
    setTitle (tr (TextId::presetManagerTitle));
    setDescription (tr (TextId::presetManagerDescription));

    titleLabel_.setText (tr (TextId::presetManagerTitle), juce::dontSendNotification);
    titleLabel_.setFont (ui::font (16.0f, juce::Font::bold));
    titleLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel_);

    searchEditor_.setName ("PresetManagerSearch");
    searchEditor_.setTitle (tr (TextId::presetManagerSearch));
    searchEditor_.setTooltip (tr (TextId::presetManagerSearch));
    ui::styleSingleLineEditor (searchEditor_);
    searchEditor_.onTextChange = [this] { filter(); };
    searchEditor_.onReturnKey = [this] { loadSelected(); };
    searchEditor_.onEscapeKey = [this]
    {
        if (searchEditor_.getText().isNotEmpty())
            searchEditor_.clear();
        else
            hide();
    };
    addAndMakeVisible (searchEditor_);

    closeButton_.onClick = [this] { hide(); };
    addAndMakeVisible (closeButton_);

    countLabel_.setFont (ui::font (11.0f));
    countLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (countLabel_);

    sortLabel_.setFont (ui::font (11.0f));
    sortLabel_.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (sortLabel_);

    newButton_.setButtonText (tr (TextId::presetManagerNew));
    newButton_.setTitle (tr (TextId::presetManagerNew));
    newButton_.setName ("NewPreset");
    newButton_.onClick = [this] { openCreateForm(); };
    addAndMakeVisible (newButton_);

    libraryLabel_.setText (tr (TextId::presetManagerLibrary), juce::dontSendNotification);
    libraryLabel_.setFont (ui::font (11.0f, juce::Font::bold));
    libraryLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (libraryLabel_);

    categoriesLabel_.setText (tr (TextId::presetManagerCategories), juce::dontSendNotification);
    categoriesLabel_.setFont (ui::font (11.0f, juce::Font::bold));
    categoriesLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (categoriesLabel_);

    SidebarRow* libraryRows[] = { &allRow_, &factoryRow_, &userRow_, &favoritesRow_, &recentsRow_ };

    for (auto* row : libraryRows)
        addAndMakeVisible (*row);

    allRow_.onClick       = [this] { setFilter (Filter::all); };
    factoryRow_.onClick   = [this] { setFilter (Filter::factory); };
    userRow_.onClick      = [this] { setFilter (Filter::user); };
    favoritesRow_.onClick = [this] { setFilter (Filter::favorites); };
    recentsRow_.onClick   = [this] { setFilter (Filter::recent); };

    for (int i = 0; i < presetCategoryCount; ++i)
    {
        const auto category = static_cast<PresetCategory> (i);
        categoryRows_[i].onClick = [this, category] { setFilter (Filter::category, category); };
        addAndMakeVisible (categoryRows_[i]);
    }

    model_ = std::make_unique<ListModel> (*this);
    listBox_.setModel (model_.get());
    listBox_.setRowHeight (ui::px (28));
    listBox_.setMultipleSelectionEnabled (false);
    listBox_.setOutlineThickness (0);
    listBox_.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    listBox_.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    listBox_.setName ("PresetManagerList");
    listBox_.setTitle (tr (TextId::presetManagerListAccessible));
    addAndMakeVisible (listBox_);

    detailName_.setFont (ui::font (15.0f, juce::Font::bold));
    detailName_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (detailName_);

    detailCurrent_.setFont (ui::font (11.0f));
    detailCurrent_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (detailCurrent_);

    favoriteButton_.setTooltip (tr (TextId::presetManagerFavoriteTooltip));
    favoriteButton_.setName ("Favorite");
    favoriteButton_.setTitle (tr (TextId::presetManagerFavorite));
    favoriteButton_.onClick = [this] { toggleFavorite(); };
    favoriteButton_.setEnabled (false);
    addAndMakeVisible (favoriteButton_);

    loadButton_.setButtonText (tr (TextId::presetManagerLoad));
    loadButton_.setTooltip (tr (TextId::presetManagerLoadTooltip));
    loadButton_.setName ("Load");
    loadButton_.onClick = [this] { loadSelected(); };
    loadButton_.setEnabled (false);
    addAndMakeVisible (loadButton_);

    openFolderButton_.setButtonText (tr (TextId::presetManagerOpenFolder));
    openFolderButton_.setTooltip (tr (TextId::presetManagerOpenFolderTooltip));
    openFolderButton_.setName ("OpenPresetsFolder");
    openFolderButton_.onClick = [this] { if (onOpenFolder) onOpenFolder(); };
    addAndMakeVisible (openFolderButton_);

    saveButton_.setButtonText (tr (TextId::saveButton));
    saveButton_.setTitle (tr (TextId::saveButton));
    saveButton_.setName ("Save");
    saveButton_.onClick = [this] { if (onSaveCurrent) onSaveCurrent(); };
    addAndMakeVisible (saveButton_);

    saveCopyButton_.setButtonText (tr (TextId::presetManagerSaveCopy));
    saveCopyButton_.setTitle (tr (TextId::presetManagerSaveCopy));
    saveCopyButton_.setName ("SaveCopy");
    saveCopyButton_.onClick = [this] { openCopyForm(); };
    addAndMakeVisible (saveCopyButton_);

    renameButton_.setButtonText (tr (TextId::renameButton));
    renameButton_.setTitle (tr (TextId::renameButton));
    renameButton_.setName ("Rename");
    renameButton_.onClick = [this] { openRenameForm(); };
    addAndMakeVisible (renameButton_);

    deleteButton_.setButtonText (tr (TextId::deleteButton));
    deleteButton_.setTitle (tr (TextId::deleteButton));
    deleteButton_.setName ("Delete");
    deleteButton_.onClick = [this] { openDeleteForm(); };
    addAndMakeVisible (deleteButton_);

    footerEsc_.setText (tr (TextId::presetManagerEsc), juce::dontSendNotification);
    footerEsc_.setFont (ui::font (11.0f));
    footerEsc_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (footerEsc_);

    // ---- formularios ----
    addChildComponent (formShield_);

    formTitle_.setFont (ui::font (14.0f, juce::Font::bold));
    formTitle_.setJustificationType (juce::Justification::centredLeft);
    addChildComponent (formTitle_);

    formNameLabel_.setText (tr (TextId::presetManagerFormName), juce::dontSendNotification);
    formNameLabel_.setFont (ui::font (11.0f));
    formNameLabel_.setJustificationType (juce::Justification::centredLeft);
    addChildComponent (formNameLabel_);

    formName_.setName ("PresetFormName");
    formName_.setTitle (tr (TextId::presetManagerFormName));
    ui::styleSingleLineEditor (formName_);
    formName_.onReturnKey = [this] { acceptForm(); };
    formName_.onEscapeKey = [this] { closeForm(); };
    addChildComponent (formName_);

    formCategoryLabel_.setText (tr (TextId::presetManagerFormCategory), juce::dontSendNotification);
    formCategoryLabel_.setFont (ui::font (11.0f));
    formCategoryLabel_.setJustificationType (juce::Justification::centredLeft);
    addChildComponent (formCategoryLabel_);

    formCategory_.setName ("PresetFormCategory");
    formCategory_.setTitle (tr (TextId::presetManagerFormCategory));
    formCategory_.setTextWhenNothingSelected (tr (TextId::presetManagerFormCategoryPlaceholder));

    for (int i = 0; i < presetCategoryCount; ++i)
        formCategory_.addItem (categoryName (static_cast<PresetCategory> (i)), i + 1);

    addChildComponent (formCategory_);

    formError_.setFont (ui::font (11.0f));
    formError_.setJustificationType (juce::Justification::centredLeft);
    addChildComponent (formError_);

    formAccept_.setName ("FormAccept");
    formAccept_.onClick = [this] { acceptForm(); };
    addChildComponent (formAccept_);

    formCancel_.setButtonText (tr (TextId::buttonCancel));
    formCancel_.setName ("FormCancel");
    formCancel_.onClick = [this] { closeForm(); };
    addChildComponent (formCancel_);

    applyThemeToChildren();
    updateDetail();
}

// ============================================================================
//  Apertura y cierre
// ============================================================================

void PresetManagerOverlay::show (const std::vector<PresetEntry>& entries,
                                 const juce::String& currentName,
                                 bool modified)
{
    if (formMode_ != FormMode::none)
        closeForm();

    entries_ = entries;
    currentName_ = currentName;
    selectedName_ = currentName;
    modified_ = modified;
    filter_ = Filter::all;
    category_ = PresetCategory::other;

    updateSidebar();

    searchEditor_.clear();
    filter();

    setVisible (true);
    toFront (false);
    resized();
    searchEditor_.grabKeyboardFocus();
}

void PresetManagerOverlay::beginCopyForm()
{
    openCopyForm();
}

void PresetManagerOverlay::refreshEntries (const std::vector<PresetEntry>& entries,
                                           const juce::String& currentName,
                                           bool modified)
{
    entries_ = entries;
    currentName_ = currentName;
    modified_ = modified;

    if (findEntry (currentName_) != nullptr)
        selectedName_ = currentName_;
    else if (findEntry (selectedName_) == nullptr)
        selectedName_ = entries_.empty() ? juce::String() : entries_.front().name;

    updateSidebar();
    filter();
}

void PresetManagerOverlay::hide()
{
    if (! isVisible())
        return;

    setVisible (false);

    if (onClosed)
        onClosed();
}

void PresetManagerOverlay::setTheme (ThemeId theme)
{
    theme_ = theme;
    closeButton_.setTheme (theme);
    applyThemeToChildren();
    updateSidebar();
    repaint();
}

// ============================================================================
//  Busqueda, seleccion y detalle
// ============================================================================

void PresetManagerOverlay::filter()
{
    visible_.clear();

    const auto query = searchEditor_.getText().trim();

    for (int i = 0; i < static_cast<int> (entries_.size()); ++i)
    {
        const auto& entry = entries_[static_cast<size_t> (i)];

        if (! matchesFilter (entry))
            continue;

        if (query.isNotEmpty() && ! entry.name.containsIgnoreCase (query))
            continue;

        visible_.push_back (i);
    }

    if (filter_ == Filter::recent)
    {
        std::stable_sort (visible_.begin(), visible_.end(),
                          [this] (int left, int right)
                          {
                              return entries_[static_cast<size_t> (left)].metadata.lastUsed
                                   > entries_[static_cast<size_t> (right)].metadata.lastUsed;
                          });

        if (static_cast<int> (visible_.size()) > recentLimit)
            visible_.resize (static_cast<size_t> (recentLimit));
    }

    updating_ = true;
    listBox_.updateContent();
    listBox_.selectRow (visibleRowOf (selectedName_), juce::dontSendNotification);
    updating_ = false;

    updateMeta();
    updateDetail();
    repaint();
}

void PresetManagerOverlay::updateMeta()
{
    countLabel_.setText (filteredCount() == 1
                             ? tr (TextId::presetManagerCountOne)
                             : trf (TextId::presetManagerCount,
                                    { juce::String (filteredCount()) }),
                         juce::dontSendNotification);

    sortLabel_.setText (filter_ == Filter::recent ? tr (TextId::presetManagerSortRecent)
                                                  : tr (TextId::presetManagerSort),
                        juce::dontSendNotification);
}

void PresetManagerOverlay::setFilter (Filter mode, PresetCategory category)
{
    filter_ = mode;
    category_ = category;
    updateSidebar();
    filter();
}

void PresetManagerOverlay::updateSidebar()
{
    int factory = 0;
    int user = 0;
    int favorites = 0;
    int recent = 0;
    int categories[presetCategoryCount] = {};

    for (const auto& entry : entries_)
    {
        if (entry.factory)
            ++factory;
        else
            ++user;

        if (entry.metadata.favorite)
            ++favorites;

        if (entry.metadata.lastUsed > 0)
            ++recent;

        ++categories[static_cast<int> (entry.metadata.category)];
    }

    const auto colours = themeColours (theme_);
    const auto neutral = colours.muted.withAlpha (0.65f);

    allRow_.setContent (tr (TextId::presetManagerAll),
                        static_cast<int> (entries_.size()),
                        filter_ == Filter::all, neutral);
    factoryRow_.setContent (tr (TextId::presetManagerFactory), factory,
                            filter_ == Filter::factory, neutral);
    userRow_.setContent (tr (TextId::presetManagerUser), user,
                         filter_ == Filter::user, neutral);
    favoritesRow_.setContent (tr (TextId::presetManagerFavorites), favorites,
                              filter_ == Filter::favorites, neutral);
    recentsRow_.setContent (tr (TextId::presetManagerRecents),
                            juce::jmin (recent, recentLimit),
                            filter_ == Filter::recent, neutral);

    for (int i = 0; i < presetCategoryCount; ++i)
    {
        const auto category = static_cast<PresetCategory> (i);

        categoryRows_[i].setContent (categoryName (category),
                                     categories[i],
                                     filter_ == Filter::category && category_ == category,
                                     categoryColour (category, colours),
                                     categories[i] == 0);
    }
}

bool PresetManagerOverlay::matchesFilter (const PresetEntry& entry) const
{
    switch (filter_)
    {
        case Filter::all:       return true;
        case Filter::factory:   return entry.factory;
        case Filter::user:      return ! entry.factory;
        case Filter::favorites: return entry.metadata.favorite;
        case Filter::recent:    return entry.metadata.lastUsed > 0;
        case Filter::category:  return entry.metadata.category == category_;
    }

    return true;
}

juce::String PresetManagerOverlay::categoryName (PresetCategory category) const
{
    switch (category)
    {
        case PresetCategory::initiation: return tr (TextId::categoryInitiation);
        case PresetCategory::bass:       return tr (TextId::categoryBass);
        case PresetCategory::lead:       return tr (TextId::categoryLead);
        case PresetCategory::pad:        return tr (TextId::categoryPad);
        case PresetCategory::strings:    return tr (TextId::categoryStrings);
        case PresetCategory::keys:       return tr (TextId::categoryKeys);
        case PresetCategory::fx:         return tr (TextId::categoryFx);
        case PresetCategory::other:      return tr (TextId::categoryOther);
    }

    return tr (TextId::categoryOther);
}

void PresetManagerOverlay::updateDetail()
{
    const auto* entry = findEntry (selectedName_);
    const bool hasEntry = entry != nullptr;
    const bool userPreset = hasEntry && ! entry->factory;
    const bool isCurrent = hasEntry && entry->name == currentName_;
    const bool visibleSelection = hasEntry && visibleRowOf (selectedName_) >= 0;

    const bool saveWasVisible = saveButton_.isVisible();

    loadButton_.setEnabled (visibleSelection);
    saveButton_.setVisible (userPreset);
    saveButton_.setEnabled (userPreset && isCurrent && modified_);
    saveCopyButton_.setEnabled (hasEntry);
    renameButton_.setEnabled (userPreset);
    deleteButton_.setEnabled (userPreset);
    favoriteButton_.setEnabled (hasEntry);

    detailCurrentModified_ = isCurrent && modified_;

    if (saveWasVisible != userPreset)
        resized();

    if (! hasEntry)
    {
        detailName_.setText (tr (TextId::presetManagerNoSelection), juce::dontSendNotification);
        detailCurrent_.setText ({}, juce::dontSendNotification);
        updateActionColours();
        updateFavoriteButton();
        repaint();
        return;
    }

    detailName_.setText (entry->name, juce::dontSendNotification);
    detailCurrent_.setText (isCurrent
                                ? (modified_
                                       ? tr (TextId::presetManagerCurrent) + " "
                                             + juce::String::charToString (0x00B7) + " "
                                             + tr (TextId::presetManagerModified)
                                       : tr (TextId::presetManagerCurrent))
                                : juce::String(),
                            juce::dontSendNotification);
    updateActionColours();
    updateFavoriteButton();
    repaint();
}

void PresetManagerOverlay::updateActionColours()
{
    const auto colours = themeColours (theme_);
    const bool loadEnabled = loadButton_.isEnabled();
    const bool deleteEnabled = deleteButton_.isEnabled();

    auto styleSecondary = [&colours] (juce::TextButton& button, bool enabled)
    {
        button.setColour (juce::TextButton::buttonColourId, colours.panel);
        button.setColour (juce::TextButton::buttonOnColourId, colours.panelBorder);
        button.setColour (juce::TextButton::textColourOffId,
                          enabled ? colours.text : colours.muted.withAlpha (0.55f));
        button.setColour (juce::TextButton::textColourOnId,
                          enabled ? colours.text : colours.muted.withAlpha (0.55f));
    };

    styleSecondary (saveButton_, saveButton_.isEnabled());
    styleSecondary (saveCopyButton_, saveCopyButton_.isEnabled());
    styleSecondary (renameButton_, renameButton_.isEnabled());

    loadButton_.setColour (juce::TextButton::buttonColourId,
                           loadEnabled ? colours.accent : colours.panel);
    loadButton_.setColour (juce::TextButton::buttonOnColourId,
                           loadEnabled ? colours.accent.brighter (0.12f) : colours.panelBorder);
    loadButton_.setColour (juce::TextButton::textColourOffId,
                           loadEnabled ? colours.background : colours.muted);
    loadButton_.setColour (juce::TextButton::textColourOnId,
                           loadEnabled ? colours.background : colours.muted);

    deleteButton_.setColour (juce::TextButton::buttonColourId,
                             deleteEnabled ? juce::Colour (0xff4a2422) : colours.panel);
    deleteButton_.setColour (juce::TextButton::buttonOnColourId,
                             deleteEnabled ? juce::Colour (0xffb34a42) : colours.panelBorder);
    deleteButton_.setColour (juce::TextButton::textColourOffId,
                             deleteEnabled ? juce::Colours::white : colours.muted);
    deleteButton_.setColour (juce::TextButton::textColourOnId,
                             deleteEnabled ? juce::Colours::white : colours.muted);
}

void PresetManagerOverlay::updateFavoriteButton()
{
    const auto* entry = findEntry (selectedName_);
    const auto colours = themeColours (theme_);
    const auto amber = juce::Colour (0xffe6c15a);

    favoriteButton_.setEnabled (entry != nullptr);
    favoriteButton_.setVisible (true);

    if (entry == nullptr)
    {
        favoriteButton_.setButtonText (juce::String::charToString (0x2606)
                                       + " " + tr (TextId::presetManagerAddFavorite));
        favoriteButton_.setColour (juce::TextButton::buttonColourId, colours.panel);
        favoriteButton_.setColour (juce::TextButton::buttonOnColourId, colours.panelBorder);
        favoriteButton_.setColour (juce::TextButton::textColourOffId, colours.muted);
        return;
    }

    if (entry->metadata.favorite)
    {
        favoriteButton_.setButtonText (juce::String::charToString (0x2605)
                                       + " " + tr (TextId::presetManagerFavorite));
        favoriteButton_.setColour (juce::TextButton::buttonColourId, amber.withAlpha (0.12f));
        favoriteButton_.setColour (juce::TextButton::buttonOnColourId, amber.withAlpha (0.55f));
        favoriteButton_.setColour (juce::TextButton::textColourOffId, colours.text);
    }
    else
    {
        favoriteButton_.setButtonText (juce::String::charToString (0x2606)
                                       + " " + tr (TextId::presetManagerAddFavorite));
        favoriteButton_.setColour (juce::TextButton::buttonColourId, colours.panel);
        favoriteButton_.setColour (juce::TextButton::buttonOnColourId, colours.panelBorder);
        favoriteButton_.setColour (juce::TextButton::textColourOffId, colours.muted);
    }
}

void PresetManagerOverlay::toggleFavorite()
{
    const auto* entry = findEntry (selectedName_);

    if (entry == nullptr)
        return;

    const bool favorite = ! entry->metadata.favorite;

    for (auto& candidate : entries_)
        if (candidate.name == selectedName_)
            candidate.metadata.favorite = favorite;

    updateSidebar();
    filter();

    if (onFavorite)
        onFavorite (selectedName_, favorite);
}

void PresetManagerOverlay::openCopyForm()
{
    if (findEntry (selectedName_) != nullptr)
        showForm (FormMode::copy, selectedName_);
}

void PresetManagerOverlay::openCreateForm()
{
    showForm (FormMode::create, {});
}

void PresetManagerOverlay::openRenameForm()
{
    const auto* entry = findEntry (selectedName_);

    if (entry != nullptr && ! entry->factory)
        showForm (FormMode::rename, selectedName_);
}

void PresetManagerOverlay::openDeleteForm()
{
    const auto* entry = findEntry (selectedName_);

    if (entry != nullptr && ! entry->factory)
        showForm (FormMode::remove, selectedName_);
}

void PresetManagerOverlay::showForm (FormMode mode, const juce::String& target)
{
    formMode_ = mode;
    formTarget_ = target;
    formError_.setText ({}, juce::dontSendNotification);
    formName_.clear();
    formCategory_.setSelectedId (0, juce::dontSendNotification);

    const auto* entry = findEntry (target);
    const bool create = mode == FormMode::create;
    const bool copy = mode == FormMode::copy;
    const bool rename = mode == FormMode::rename;
    const bool withName = create || copy || rename;
    const bool withCategory = create || copy;

    formTitle_.setVisible (true);
    formAccept_.setVisible (true);
    formCancel_.setVisible (true);
    formError_.setVisible (true);
    formNameLabel_.setVisible (withName);
    formName_.setVisible (withName);
    formCategoryLabel_.setVisible (withCategory);
    formCategory_.setVisible (withCategory);

    if (create)
    {
        formTitle_.setText (tr (TextId::presetManagerFormNewTitle), juce::dontSendNotification);
        formName_.clear();
        formCategory_.setSelectedId (static_cast<int> (PresetCategory::other) + 1,
                                     juce::dontSendNotification);
        formAccept_.setButtonText (tr (TextId::saveButton));
    }
    else if (copy)
    {
        formTitle_.setText (tr (TextId::presetManagerFormCopyTitle), juce::dontSendNotification);
        formName_.clear();

        if (entry != nullptr)
            formCategory_.setSelectedId (static_cast<int> (entry->metadata.category) + 1,
                                         juce::dontSendNotification);

        formAccept_.setButtonText (tr (TextId::saveButton));
    }
    else if (rename)
    {
        formTitle_.setText (tr (TextId::presetManagerFormRenameTitle), juce::dontSendNotification);
        formName_.setText (target, juce::dontSendNotification);
        formAccept_.setButtonText (tr (TextId::renameButton));
    }
    else
    {
        formTitle_.setText (tr (TextId::dialogDeleteTitle), juce::dontSendNotification);
        formError_.setText (trf (TextId::dialogDeleteMessage, { target }),
                            juce::dontSendNotification);
        formAccept_.setButtonText (tr (TextId::buttonDelete));
    }

    formShield_.setVisible (true);
    resized();

    if (withName)
    {
        formName_.grabKeyboardFocus();

        if (rename)
            formName_.selectAll();
    }
    else
    {
        formCancel_.grabKeyboardFocus();
    }
}

void PresetManagerOverlay::closeForm()
{
    formMode_ = FormMode::none;
    formTarget_.clear();
    formShield_.setVisible (false);
    formTitle_.setVisible (false);
    formNameLabel_.setVisible (false);
    formName_.setVisible (false);
    formCategoryLabel_.setVisible (false);
    formCategory_.setVisible (false);
    formError_.setVisible (false);
    formAccept_.setVisible (false);
    formCancel_.setVisible (false);

    resized();

    if (isVisible())
        searchEditor_.grabKeyboardFocus();
}

void PresetManagerOverlay::acceptForm()
{
    if (formMode_ == FormMode::none)
        return;

    const auto name = formName_.getText().trim();
    const auto stored = juce::File::createLegalFileName (name);

    if (formMode_ == FormMode::copy || formMode_ == FormMode::create)
    {
        if (stored.isEmpty())
        {
            formError_.setText (tr (TextId::presetManagerFormNameError), juce::dontSendNotification);
            return;
        }

        if (formCategory_.getSelectedId() <= 0)
        {
            formError_.setText (tr (TextId::presetManagerFormCategoryError), juce::dontSendNotification);
            return;
        }

        if (findEntry (stored) != nullptr)
        {
            formError_.setText (tr (TextId::presetManagerFormExistsError), juce::dontSendNotification);
            return;
        }

        if (! onSaveCopy || ! onSaveCopy (name, selectedFormCategory()))
        {
            formError_.setText (tr (TextId::presetManagerFormError), juce::dontSendNotification);
            return;
        }

        closeForm();
        return;
    }

    if (formMode_ == FormMode::rename)
    {
        if (stored.isEmpty())
        {
            formError_.setText (tr (TextId::presetManagerFormNameError), juce::dontSendNotification);
            return;
        }

        if (stored == formTarget_)
        {
            closeForm();
            return;
        }

        if (findEntry (stored) != nullptr)
        {
            formError_.setText (tr (TextId::presetManagerFormExistsError), juce::dontSendNotification);
            return;
        }

        if (! onRename || ! onRename (formTarget_, name))
        {
            formError_.setText (tr (TextId::presetManagerFormError), juce::dontSendNotification);
            return;
        }

        closeForm();
        return;
    }

    if (! onDelete || ! onDelete (formTarget_))
    {
        formError_.setText (tr (TextId::presetManagerFormError), juce::dontSendNotification);
        return;
    }

    closeForm();
}

PresetCategory PresetManagerOverlay::selectedFormCategory() const
{
    return static_cast<PresetCategory> (formCategory_.getSelectedId() - 1);
}

void PresetManagerOverlay::rowSelected (int row)
{
    if (updating_ || row < 0 || row >= filteredCount())
        return;

    selectedName_ = entries_[static_cast<size_t> (visible_[static_cast<size_t> (row)])].name;
    updateDetail();
    repaint();
}

void PresetManagerOverlay::rowClicked (int row, const juce::MouseEvent& e)
{
    if (row < 0 || row >= filteredCount())
        return;

    // La estrella ocupa los primeros 24 px tras el margen izquierdo de la fila.
    const int starLeft = ui::px (8);
    const int starRight = starLeft + ui::px (24);

    if (e.x >= starLeft && e.x <= starRight)
    {
        selectedName_ = entries_[static_cast<size_t> (visible_[static_cast<size_t> (row)])].name;
        updateDetail();
        toggleFavorite();
    }
}

void PresetManagerOverlay::focusNextInOverlay (bool backwards)
{
    std::vector<juce::Component*> focusables;

    if (formMode_ != FormMode::none)
    {
        if (formName_.isVisible())
            focusables.push_back (&formName_);

        if (formCategory_.isVisible())
            focusables.push_back (&formCategory_);

        focusables.push_back (&formAccept_);
        focusables.push_back (&formCancel_);
    }
    else
    {
        focusables.push_back (&searchEditor_);
        focusables.push_back (&listBox_);

        SidebarRow* rows[] = { &allRow_, &factoryRow_, &userRow_, &favoritesRow_, &recentsRow_ };

        for (auto* row : rows)
            focusables.push_back (row);

        for (auto& row : categoryRows_)
            focusables.push_back (&row);

        auto addAction = [&focusables] (juce::Component& component)
        {
            if (component.isVisible() && component.isEnabled())
                focusables.push_back (&component);
        };

        addAction (favoriteButton_);
        addAction (loadButton_);
        addAction (saveButton_);
        addAction (saveCopyButton_);
        addAction (renameButton_);
        addAction (deleteButton_);
        focusables.push_back (&openFolderButton_);
    }

    const auto* current = juce::Component::getCurrentlyFocusedComponent();
    int index = -1;

    for (int i = 0; i < static_cast<int> (focusables.size()); ++i)
        if (focusables[static_cast<size_t> (i)] == current)
            index = i;

    const int count = static_cast<int> (focusables.size());

    if (count == 0)
        return;

    const int next = backwards
                         ? (index <= 0 ? count - 1 : index - 1)
                         : (index + 1) % count;

    focusables[static_cast<size_t> (next)]->grabKeyboardFocus();
}

void PresetManagerOverlay::loadRow (int row)
{
    if (row < 0 || row >= filteredCount())
        return;

    selectedName_ = entries_[static_cast<size_t> (visible_[static_cast<size_t> (row)])].name;
    updateDetail();
    loadSelected();
}

void PresetManagerOverlay::loadSelected()
{
    const auto name = selectedName_;

    if (findEntry (name) == nullptr || visibleRowOf (name) < 0)
        return;

    hide();

    if (onLoad)
        onLoad (name);
}

int PresetManagerOverlay::visibleRowOf (const juce::String& name) const
{
    if (name.isEmpty())
        return -1;

    for (int row = 0; row < filteredCount(); ++row)
        if (entries_[static_cast<size_t> (visible_[static_cast<size_t> (row)])].name == name)
            return row;

    return -1;
}

const PresetEntry* PresetManagerOverlay::findEntry (const juce::String& name) const
{
    for (const auto& entry : entries_)
        if (entry.name == name)
            return &entry;

    return nullptr;
}

juce::String PresetManagerOverlay::rowName (int row) const
{
    if (row < 0 || row >= filteredCount())
        return {};

    const auto& entry = entries_[static_cast<size_t> (visible_[static_cast<size_t> (row)])];
    const auto origin = entry.factory ? tr (TextId::presetManagerFactory)
                                      : tr (TextId::presetManagerUser);

    return entry.name + " - " + categoryName (entry.metadata.category) + " - " + origin;
}

// ============================================================================
//  Pintura
// ============================================================================

void PresetManagerOverlay::paint (juce::Graphics& g)
{
    const auto colours = themeColours (theme_);

    g.fillAll (juce::Colours::black.withAlpha (0.58f));

    const auto panel = panelBounds_.toFloat();

    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (panel.translated (0.0f, 8.0f), 12.0f);

    g.setColour (colours.panel);
    g.fillRoundedRectangle (panel, 12.0f);

    g.setColour (colours.panelBorder);
    g.drawRoundedRectangle (panel, 12.0f, 1.0f);

    g.setColour (colours.panelBorder.withAlpha (0.55f));

    const auto bodyTop = sidebarBounds_.getY();
    const auto bodyBottom = sidebarBounds_.getBottom();
    const auto lineTop = bodyTop - ui::px (6);

    g.drawLine (static_cast<float> (sidebarBounds_.getX()),
                static_cast<float> (lineTop),
                static_cast<float> (detailBounds_.getRight()),
                static_cast<float> (lineTop), 1.0f);

    g.drawLine (static_cast<float> (sidebarBounds_.getRight()) + 0.5f,
                static_cast<float> (bodyTop),
                static_cast<float> (sidebarBounds_.getRight()) + 0.5f,
                static_cast<float> (bodyBottom), 1.0f);

    g.drawLine (static_cast<float> (listBounds_.getRight()) + 0.5f,
                static_cast<float> (bodyTop),
                static_cast<float> (listBounds_.getRight()) + 0.5f,
                static_cast<float> (bodyBottom), 1.0f);

    if (! sidebarSeparatorBounds_.isEmpty())
    {
        g.setColour (colours.panelBorder.withAlpha (0.50f));
        g.fillRect (sidebarSeparatorBounds_);
    }

    paintListHeader (g);
    paintDetail (g);
    paintEmptyState (g);
}

void PresetManagerOverlay::paintOverChildren (juce::Graphics& g)
{
    if (formMode_ != FormMode::none || ! searchEditor_.getText().isEmpty())
        return;

    // Misma caja que usa TextEditor para el texto: viewport (bounds menos
    // borde) recortado por los indents. Asi el placeholder comparte x y linea
    // base con el caret y con lo que se escribe.
    const auto border = searchEditor_.getBorder();
    auto area = searchEditor_.getBounds()
                    .withTrimmedLeft (border.getLeft() + searchEditor_.getLeftIndent())
                    .withTrimmedRight (border.getRight())
                    .withTrimmedTop (border.getTop() + searchEditor_.getTopIndent())
                    .withTrimmedBottom (border.getBottom());

    g.setColour (themeColours (theme_).muted);
    g.setFont (searchEditor_.getFont());
    g.drawFittedText (tr (TextId::presetManagerSearch), area,
                      juce::Justification::centredLeft, 1);
}

void PresetManagerOverlay::paintListHeader (juce::Graphics& g)
{
    if (listHeaderBounds_.isEmpty() || listContentWidth_ <= 0)
        return;

    const auto colours = themeColours (theme_);
    auto area = listHeaderBounds_
                    .withWidth (juce::jmin (listHeaderBounds_.getWidth(), listContentWidth_))
                    .reduced (ui::px (8), 0);

    area.removeFromLeft (ui::px (24));
    auto origin = area.removeFromRight (ui::px (48));
    auto category = area.removeFromRight (listCategoryWidth_);
    auto name = area;

    g.setColour (colours.muted);
    g.setFont (ui::font (10.0f));
    g.drawFittedText (tr (TextId::presetManagerColumnName).toUpperCase(),
                      name, juce::Justification::centredLeft, 1);
    g.drawFittedText (tr (TextId::presetManagerColumnCategory).toUpperCase(),
                      category, juce::Justification::centredLeft, 1);
    g.drawFittedText (tr (TextId::presetManagerColumnOrigin).toUpperCase(),
                      origin, juce::Justification::centredRight, 1);

    // Raya de la cabecera: mismo ancho que las filas y canal de scrollbar
    // reservado, con 4 px de aire antes del primer preset.
    g.setColour (colours.panelBorder.withAlpha (0.50f));
    g.fillRect (listHeaderBounds_.getX(),
                listHeaderBounds_.getBottom() - 1,
                juce::jmin (listHeaderBounds_.getWidth(), listContentWidth_),
                1);
}

void PresetManagerOverlay::paintDetail (juce::Graphics& g)
{
    const auto* entry = findEntry (selectedName_);

    if (entry == nullptr)
        return;

    const auto colours = themeColours (theme_);

    if (! detailBadgesBounds_.isEmpty())
    {
        auto area = detailBadgesBounds_;
        const auto categoryText = categoryName (entry->metadata.category);
        const int categoryW = juce::jmin (area.getWidth(),
                                          juce::jmax (ui::px (40), badgeWidth (categoryText)));
        auto categoryArea = area.removeFromLeft (categoryW)
                                .withSizeKeepingCentre (categoryW, ui::px (20));

        paintBadge (g, categoryArea, categoryText,
                    categoryColour (entry->metadata.category, colours), colours, true);

        area.removeFromLeft (ui::px (5));

        const auto originText = entry->factory ? tr (TextId::presetManagerFactory)
                                               : tr (TextId::presetManagerUser);
        const int originW = juce::jmin (area.getWidth(),
                                        juce::jmax (ui::px (42), badgeWidth (originText)));
        auto originArea = area.removeFromLeft (originW)
                              .withSizeKeepingCentre (originW, ui::px (20));

        paintBadge (g, originArea, originText,
                    entry->factory ? colours.muted : colours.accent, colours, false);
    }

    if (! detailCardBounds_.isEmpty())
    {
        auto card = detailCardBounds_;

        g.setColour (colours.backgroundAlt.withAlpha (0.35f));
        g.fillRoundedRectangle (card.toFloat(), 6.0f);
        g.setColour (colours.panelBorder.withAlpha (0.45f));
        g.drawRoundedRectangle (card.toFloat(), 6.0f, 1.0f);

        auto inner = card.reduced (ui::px (8), ui::px (6));
        auto title = inner.removeFromTop (ui::px (14));

        g.setColour (colours.text.withAlpha (0.85f));
        g.setFont (ui::font (10.0f, juce::Font::bold));
        g.drawFittedText (entry->factory ? tr (TextId::presetManagerFactoryCardTitle)
                                         : tr (TextId::presetManagerUserCardTitle),
                          title, juce::Justification::centredLeft, 1);

        g.setColour (colours.muted);
        g.setFont (ui::font (9.5f));
        g.drawFittedText (entry->factory ? tr (TextId::presetManagerFactoryCardBody)
                                         : tr (TextId::presetManagerUserCardBody),
                          inner, juce::Justification::topLeft, 2);
    }

    if (detailCurrentModified_ && ! detailStatusDotBounds_.isEmpty())
    {
        g.setColour (juce::Colour (0xffe6c15a));
        g.fillEllipse (detailStatusDotBounds_.toFloat()
                           .withSizeKeepingCentre (ui::px (5), ui::px (5)));
    }

    if (detailSeparatorY_ > 0)
    {
        const auto lineArea = detailBounds_.reduced (ui::px (12), 0);
        g.setColour (colours.panelBorder.withAlpha (0.50f));
        g.fillRect (lineArea.getX(), detailSeparatorY_, lineArea.getWidth(), 1);
    }
}

void PresetManagerOverlay::paintEmptyState (juce::Graphics& g)
{
    if (! visible_.empty())
        return;

    const auto colours = themeColours (theme_);
    const auto listArea = listBox_.getBounds();

    if (listArea.isEmpty())
        return;

    const bool searchActive = searchEditor_.getText().trim().isNotEmpty();
    const bool useSearchText = searchActive || filter_ == Filter::all;

    auto block = listArea.reduced (ui::px (16), 0)
                         .withSizeKeepingCentre (listArea.getWidth() - ui::px (32), ui::px (46));

    auto top = block.removeFromTop (ui::px (24));

    g.setColour (colours.text);
    g.setFont (ui::font (13.0f, juce::Font::bold));
    g.drawFittedText (tr (useSearchText ? TextId::presetManagerNoResults
                                        : TextId::presetManagerEmptyFilter),
                      top, juce::Justification::centred, 1);

    g.setColour (colours.muted);
    g.setFont (ui::font (11.0f));
    g.drawFittedText (tr (useSearchText ? TextId::presetManagerNoResultsHint
                                        : TextId::presetManagerEmptyFilterHint),
                      block, juce::Justification::centred, 1);
}

void PresetManagerOverlay::paintRow (int row, juce::Graphics& g,
                                     int width, int height, bool selected)
{
    if (row < 0 || row >= filteredCount())
        return;

    const auto colours = themeColours (theme_);
    const auto& entry = entries_[static_cast<size_t> (visible_[static_cast<size_t> (row)])];
    const int rowWidth = listContentWidth_ > 0
                             ? juce::jmin (width, listContentWidth_)
                             : width;

    if (selected)
    {
        g.setColour (colours.accent.withAlpha (0.18f));
        g.fillRect (0, 0, rowWidth, height);
    }

    auto area = juce::Rectangle<int> (0, 0, rowWidth, height).reduced (ui::px (8), 0);
    auto star = area.removeFromLeft (ui::px (24));
    auto origin = area.removeFromRight (ui::px (48));
    auto category = area.removeFromRight (listCategoryWidth_);
    auto name = area;

    g.setFont (ui::font (11.0f));
    g.setColour (entry.metadata.favorite ? juce::Colour (0xffe6c15a)
                                         : colours.muted.withAlpha (0.55f));
    g.drawFittedText (juce::String::charToString (entry.metadata.favorite ? 0x2605 : 0x2606),
                      star, juce::Justification::centred, 1);

    g.setFont (ui::font (12.0f));
    g.setColour (selected ? colours.text : colours.text.withAlpha (0.92f));
    g.drawFittedText (entry.name, name, juce::Justification::centredLeft, 1);

    if (listCategoryWidth_ > 0)
    {
        const auto categoryText = categoryName (entry.metadata.category);
        const int pillW = juce::jmin (category.getWidth(),
                                      juce::jmax (ui::px (36), badgeWidth (categoryText)));
        auto pill = category.removeFromLeft (pillW)
                            .withSizeKeepingCentre (pillW, ui::px (17));

        paintBadge (g, pill, categoryText,
                    categoryColour (entry.metadata.category, colours), colours, true);
    }

    g.setColour (entry.factory ? colours.muted : colours.accent);
    g.fillEllipse (origin.toFloat().withSizeKeepingCentre (5.0f, 5.0f));
}

// ============================================================================
//  Layout
// ============================================================================

void PresetManagerOverlay::resized()
{
    const auto full = getLocalBounds();
    const int panelW = juce::jmin (ui::px (900), juce::jmax (ui::px (560), full.getWidth() - ui::px (48)));
    const int panelH = juce::jmin (ui::px (620), juce::jmax (ui::px (480), full.getHeight() - ui::px (48)));

    panelBounds_ = full.withSizeKeepingCentre (panelW, panelH);

    auto p = panelBounds_.reduced (ui::px (20));

    auto header = p.removeFromTop (ui::px (44));
    closeButton_.setBounds (header.removeFromRight (ui::px (28))
                                  .withSizeKeepingCentre (ui::px (28), ui::px (24)));
    header.removeFromRight (ui::px (10));
    searchEditor_.setBounds (header.removeFromRight (ui::px (320))
                                   .withSizeKeepingCentre (ui::px (320), ui::px (26)));
    header.removeFromRight (ui::px (12));
    titleLabel_.setBounds (header);

    auto meta = p.removeFromTop (ui::px (28));
    countLabel_.setBounds (meta.removeFromLeft (ui::px (220)));
    sortLabel_.setBounds (meta.removeFromRight (ui::px (80)));

    auto footer = p.removeFromBottom (ui::px (24));
    footerEsc_.setBounds (footer.removeFromLeft (ui::px (160)));
    openFolderButton_.setBounds (footer.removeFromRight (ui::px (120)));
    footer.removeFromRight (ui::px (8));

    auto body = p;

    sidebarBounds_ = body.removeFromLeft (juce::jmax (ui::px (120),
                                                      juce::jmin (ui::px (208), body.getWidth() / 3)));
    listBounds_ = body.removeFromLeft (juce::jmax (ui::px (220),
                                                   juce::jmin (ui::px (404), body.getWidth() - ui::px (160))));
    detailBounds_ = body;

    auto listArea = listBounds_;
    listHeaderBounds_ = listArea.removeFromTop (ui::px (22));
    listContentWidth_ = juce::jmax (ui::px (120), listArea.getWidth() - ui::px (9));
    listArea.removeFromTop (ui::px (4));
    listBox_.setBounds (listArea);

    const auto headerFont = ui::font (10.0f);
    float categoryTextWidth = 0.0f;

    for (int i = 0; i < presetCategoryCount; ++i)
        categoryTextWidth = juce::jmax (
            categoryTextWidth,
            juce::GlyphArrangement::getStringWidth (
                headerFont, categoryName (static_cast<PresetCategory> (i))));

    listCategoryWidth_ = juce::jlimit (ui::px (66), ui::px (104),
                                       juce::roundToInt (categoryTextWidth) + 2 * ui::px (7));

    const bool compactSidebar = ui::getTextSize() == TextSize::extraLarge;
    auto side = sidebarBounds_.reduced (ui::px (8), ui::px (8));
    newButton_.setBounds (side.removeFromTop (ui::px (30)));
    side.removeFromTop (ui::px (compactSidebar ? 6 : 8));
    libraryLabel_.setBounds (side.removeFromTop (ui::px (compactSidebar ? 20 : 22)));
    side.removeFromTop (ui::px (compactSidebar ? 3 : 4));

    auto placeRow = [&side] (SidebarRow& row)
    {
        row.setBounds (side.removeFromTop (ui::px (23)));
    };

    placeRow (allRow_);
    placeRow (factoryRow_);
    placeRow (userRow_);
    placeRow (favoritesRow_);
    placeRow (recentsRow_);

    auto separatorArea = side.removeFromTop (ui::px (compactSidebar ? 6 : 8));
    sidebarSeparatorBounds_ = separatorArea.withSizeKeepingCentre (separatorArea.getWidth(), 1);
    categoriesLabel_.setBounds (side.removeFromTop (ui::px (compactSidebar ? 18 : 20)));
    side.removeFromTop (ui::px (compactSidebar ? 1 : 2));

    for (auto& row : categoryRows_)
        placeRow (row);

    auto detail = detailBounds_.reduced (ui::px (12), ui::px (8));
    detailName_.setBounds (detail.removeFromTop (ui::px (24)));
    detail.removeFromTop (ui::px (3));
    detailBadgesBounds_ = detail.removeFromTop (ui::px (20));
    detail.removeFromTop (ui::px (6));
    favoriteButton_.setBounds (detail.removeFromTop (ui::px (26)));
    detail.removeFromTop (ui::px (8));
    detailCardBounds_ = detail.removeFromTop (ui::px (54));
    detail.removeFromTop (ui::px (6));

    auto status = detail.removeFromTop (ui::px (18));
    detailStatusDotBounds_ = status.removeFromLeft (ui::px (10));
    detailCurrent_.setBounds (status);

    const int loadH = ui::px (30);
    const int buttonH = ui::px (26);
    const int gap = ui::px (6);
    const int actionSeparatorGap = ui::px (10);
    const bool showSave = saveButton_.isVisible();
    const int blockH = loadH + gap + (showSave ? buttonH + gap : 0)
                     + buttonH + gap + buttonH + actionSeparatorGap + buttonH;

    auto actions = detail.removeFromBottom (juce::jmin (blockH, detail.getHeight()));
    loadButton_.setBounds (actions.removeFromTop (loadH));
    actions.removeFromTop (gap);

    if (showSave)
    {
        saveButton_.setBounds (actions.removeFromTop (buttonH));
        actions.removeFromTop (gap);
    }
    else
    {
        saveButton_.setBounds ({});
    }

    saveCopyButton_.setBounds (actions.removeFromTop (buttonH));
    actions.removeFromTop (gap);
    renameButton_.setBounds (actions.removeFromTop (buttonH));
    actions.removeFromTop (actionSeparatorGap);
    deleteButton_.setBounds (actions.removeFromTop (buttonH));
    detailSeparatorY_ = deleteButton_.getY() - actionSeparatorGap / 2;

    updateFormLayout();
}

void PresetManagerOverlay::updateFormLayout()
{
    formShield_.setBounds (getLocalBounds());

    if (formMode_ == FormMode::none)
    {
        formShield_.setCardBounds ({});
        return;
    }

    const int cardW = juce::jmin (ui::px (380), panelBounds_.getWidth() - ui::px (80));
    const int cardH = (formMode_ == FormMode::copy || formMode_ == FormMode::create)
                          ? ui::px (250)
                          : (formMode_ == FormMode::rename ? ui::px (205) : ui::px (175));

    formCardBounds_ = panelBounds_.withSizeKeepingCentre (cardW, cardH);
    formShield_.setCardBounds (formCardBounds_);

    auto content = formCardBounds_.reduced (ui::px (20));
    formTitle_.setBounds (content.removeFromTop (ui::px (24)));
    content.removeFromTop (ui::px (12));

    if (formMode_ == FormMode::copy || formMode_ == FormMode::create
        || formMode_ == FormMode::rename)
    {
        formNameLabel_.setBounds (content.removeFromTop (ui::px (18)));
        content.removeFromTop (ui::px (4));
        formName_.setBounds (content.removeFromTop (ui::px (26)));
        content.removeFromTop (ui::px (10));
    }

    if (formMode_ == FormMode::copy || formMode_ == FormMode::create)
    {
        formCategoryLabel_.setBounds (content.removeFromTop (ui::px (18)));
        content.removeFromTop (ui::px (4));
        formCategory_.setBounds (content.removeFromTop (ui::px (26)));
        content.removeFromTop (ui::px (10));
    }

    formError_.setBounds (content.removeFromTop (ui::px (22)));

    auto buttons = formCardBounds_.reduced (ui::px (20)).removeFromBottom (ui::px (28));
    formCancel_.setBounds (buttons.removeFromRight (ui::px (100)));
    buttons.removeFromRight (ui::px (8));
    formAccept_.setBounds (buttons.removeFromRight (ui::px (100)));
}

void PresetManagerOverlay::mouseDown (const juce::MouseEvent& e)
{
    if (! panelBounds_.contains (e.getPosition()))
        hide();
}

bool PresetManagerOverlay::keyPressed (const juce::KeyPress& key)
{
    if (formMode_ != FormMode::none)
    {
        if (key.getKeyCode() == juce::KeyPress::escapeKey)
        {
            closeForm();
            return true;
        }

        if (key.getKeyCode() == juce::KeyPress::tabKey)
        {
            focusNextInOverlay (key.getModifiers().isShiftDown());
            return true;
        }

        if (key.getKeyCode() == juce::KeyPress::returnKey)
        {
            acceptForm();
            return true;
        }

        return false;
    }

    if (key.getKeyCode() == juce::KeyPress::tabKey)
    {
        focusNextInOverlay (key.getModifiers().isShiftDown());
        return true;
    }

    if (key.getKeyCode() == juce::KeyPress::escapeKey)
    {
        hide();
        return true;
    }

    if (key.getKeyCode() == juce::KeyPress::returnKey)
    {
        loadSelected();
        return true;
    }

    return false;
}

void PresetManagerOverlay::applyThemeToChildren()
{
    const auto colours = themeColours (theme_);

    titleLabel_.setColour (juce::Label::textColourId, colours.text);
    countLabel_.setColour (juce::Label::textColourId, colours.muted);
    sortLabel_.setColour (juce::Label::textColourId, colours.muted);
    libraryLabel_.setColour (juce::Label::textColourId, colours.muted);
    categoriesLabel_.setColour (juce::Label::textColourId, colours.muted);

    allRow_.setTheme (theme_);
    factoryRow_.setTheme (theme_);
    userRow_.setTheme (theme_);
    favoritesRow_.setTheme (theme_);
    recentsRow_.setTheme (theme_);

    for (auto& row : categoryRows_)
        row.setTheme (theme_);
    detailName_.setColour (juce::Label::textColourId, colours.text);
    detailCurrent_.setColour (juce::Label::textColourId, colours.accent);
    footerEsc_.setColour (juce::Label::textColourId, colours.muted);

    searchEditor_.setTextToShowWhenEmpty ({}, colours.muted);
    searchEditor_.setColour (juce::TextEditor::textColourId, colours.text);
    searchEditor_.setColour (juce::TextEditor::highlightColourId, colours.accent.withAlpha (0.35f));

    listBox_.setColour (juce::ListBox::textColourId, colours.text);

    formShield_.setTheme (theme_);
    formTitle_.setColour (juce::Label::textColourId, colours.text);
    formNameLabel_.setColour (juce::Label::textColourId, colours.text);
    formCategoryLabel_.setColour (juce::Label::textColourId, colours.text);
    formError_.setColour (juce::Label::textColourId, juce::Colour (0xffe06c75));
    formName_.setColour (juce::TextEditor::textColourId, colours.text);
    formName_.setColour (juce::TextEditor::highlightColourId, colours.accent.withAlpha (0.35f));
    formCategory_.setColour (juce::ComboBox::textColourId, colours.text);

    updateDetail();
}

} // namespace aeonix
