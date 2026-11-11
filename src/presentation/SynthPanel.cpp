#include "SynthPanel.h"

#include "presentation/theme/Typography.h"

namespace aeonix
{

SynthPanel::SynthPanel (std::function<void (const std::string&, float)> setParameter,
                        std::function<void (int, float)> noteOn,
                        std::function<void (int)> noteOff,
                        PresetRepository& presetRepository)
    : setParameter_ (std::move (setParameter)),
      noteOn_ (std::move (noteOn)),
      noteOff_ (std::move (noteOff)),
      presetRepository_ (presetRepository),
      viewModels_ (setParameter_)
{
    // ---- paneles de seccion (deben estar detras de los controles) ----
    addAndMakeVisible (oscView_);
    addAndMakeVisible (filterView_);
    addAndMakeVisible (filterEnvView_);
    addAndMakeVisible (ampEnvView_);
    addAndMakeVisible (lfoView_);
    addAndMakeVisible (unisonView_);
    addAndMakeVisible (delayView_);
    addAndMakeVisible (reverbView_);
    addAndMakeVisible (masterView_);

    masterView_.onOctaveChanged = [this] (int octave)
    {
        if (onOctaveChanged)
            onOctaveChanged (octave);
    };

    masterView_.onPolyphonyChanged = [this] (int voices)
    {
        if (onPolyphonyChanged)
            onPolyphonyChanged (voices);
    };

    // ---- presets ----
    addAndMakeVisible (presetBar_);
    presetBar_.onOpenFolder = [this] { if (onOpenPresetsFolder) onOpenPresetsFolder(); };

    // Ventana de tooltips (aparecen tras 700 ms sobre cada control).
    tooltipWindow_ = std::make_unique<juce::TooltipWindow> (this, 700);

    // ---- selector de dispositivo MIDI ----
    // El dispositivo MIDI se elige en Ajustes > MIDI (SettingsOverlay).

    // ---- ruedas de performance: pitch bend + mod wheel ----
    addAndMakeVisible (performanceWheels_);
    performanceWheels_.onPitchBend = [this] (float value)
    {
        if (onPitchBend)
            onPitchBend (value);
    };
    performanceWheels_.onModWheel = [this] (float value)
    {
        if (onModWheel)
            onModWheel (value);
    };

    // ---- teclado virtual ----
    addAndMakeVisible (keyboardBed_);

    keyboardState_.addListener (this);

    presetController_ = std::make_unique<PresetController> (
        presetBar_, presetCatalog_, patchBinder_, *this,
        [this] (const juce::String& name)
        {
            if (onPresetLoaded)
                onPresetLoaded (name);
        },
        [this] { return presetBar_.isModified(); },
        [this] (const juce::String& message)
        {
            if (onPresetToast)
                onPresetToast (message);
        });
    presetController_->refresh();
    presetController_->onRequestSaveCopy = [this]
    {
        if (onRequestSaveCopy)
            onRequestSaveCopy();
    };

    setTheme (ThemeId::holoAccents);
}

SynthPanel::~SynthPanel()
{
    keyboardState_.removeListener (this);
}

void SynthPanel::resized()
{
    auto area = getLocalBounds().reduced (12);

    // ---- barra superior: presets ----
    auto presetArea = area.removeFromTop (PresetBar::preferredHeight());
    presetBar_.setBounds (presetArea.getX(), presetArea.getY(),
                          ui::px (PresetBar::preferredWidth), PresetBar::preferredHeight());

    area.removeFromTop (ui::px (10));

    // ---- filas de secciones ----
    const int gap            = ui::px (12);
    const int sectionPadding = ui::px (10);

    auto layoutRow = [&] (juce::Rectangle<int> row,
                          std::initializer_list<std::pair<SectionPanel*, int>> sections)
    {
        int cells = 0;
        for (const auto& entry : sections)
            cells += entry.second;

        const int sectionCount = static_cast<int> (sections.size());
        const int usable = row.getWidth()
                         - (sectionCount - 1) * gap
                         - sectionCount * 2 * sectionPadding;
        const int cellWidth = juce::jlimit (ui::px (56), ui::px (80), usable / juce::jmax (1, cells));

        int totalWidth = (sectionCount - 1) * gap;
        for (const auto& entry : sections)
            totalWidth += entry.second * cellWidth + 2 * sectionPadding;

        int x = row.getX() + (row.getWidth() - totalWidth) / 2;

        for (const auto& entry : sections)
        {
            const int width = entry.second * cellWidth + 2 * sectionPadding;
            entry.first->setBounds (x, row.getY(), width, row.getHeight());
            x += width + gap;
        }
    };

    // Fila 1: generacion y forma (OSC, filtro, envolventes).
    auto row1 = area.removeFromTop (ui::sectionRow1Height());
    layoutRow (row1, {
        { &oscView_, 3 },        // combo de onda (2 celdas) + nivel
        { &filterView_, 3 },
        { &filterEnvView_, 4 },
        { &ampEnvView_, 4 }
    });

    area.removeFromTop (gap);

    // Fila 2: modulacion y salida (LFO, unison, delay, reverb, master).
    auto row2 = area.removeFromTop (ui::sectionRow2Height());
    layoutRow (row2, {
        { &lfoView_, 4 },        // combo de onda (2 celdas) + rate + depth
        { &unisonView_, 3 },
        { &delayView_, 3 },
        { &reverbView_, 2 },
        { &masterView_, 3 }
    });

    area.removeFromTop (ui::px (16));

    // ---- teclado + ruedas de performance a la izquierda ----
    // Ocupan todo el alto restante. El KeyboardBed anade el fieltro arriba y
    // el marco de 6 px alrededor del keybed.
    auto performanceRow = area;
    performanceWheels_.setBounds (performanceRow.removeFromLeft (ui::px (PerformanceWheels::preferredWidth)));
    performanceRow.removeFromLeft (8);

    auto keybed = performanceRow;
    keybed.removeFromTop (8);       // tira de fieltro
    keybed.reduce (6, 0);           // marco lateral
    keybed.removeFromBottom (6);    // marco inferior

    const float keyWidth = juce::jlimit (16.0f, 40.0f,
                                         juce::jmax (120.0f, (float) keybed.getWidth()) / 29.5f);

    keyboardBed_.getKeyboard().setKeyWidth (keyWidth);
    keyboardBed_.setBounds (KeyboardBed::bedBoundsFor (keybed));
}

void SynthPanel::resetPerformanceControls()
{
    performanceWheels_.reset();
}

void SynthPanel::setPitchBendDisplay (float value)
{
    performanceWheels_.setPitchBendDisplay (value);
}

void SynthPanel::setModWheelDisplay (float value)
{
    performanceWheels_.setModWheelDisplay (value);
}

void SynthPanel::setOctave (int octave)
{
    masterView_.setOctave (octave);
}

void SynthPanel::setPolyphony (int voices)
{
    masterView_.setPolyphony (voices);
}

bool SynthPanel::isPresetModified() const
{
    return presetBar_.isModified();
}

void SynthPanel::setTheme (ThemeId theme)
{
    SectionPanel* sections[] = {
        &oscView_, &filterView_, &filterEnvView_, &ampEnvView_,
        &lfoView_, &unisonView_, &delayView_, &reverbView_, &masterView_
    };

    for (auto* section : sections)
        section->setTheme (theme);

    keyboardBed_.setTheme (theme);
    performanceWheels_.setTheme (theme);
    presetBar_.setTheme (theme);

    if (presetController_ != nullptr)
        presetController_->setTheme (theme);

    repaint();
}

void SynthPanel::selectPresetByName (const juce::String& name, bool countAsRecent)
{
    if (presetController_ != nullptr)
        presetController_->selectByName (name, countAsRecent);
}

void SynthPanel::requestLoadPreset (const juce::String& name)
{
    if (presetController_ != nullptr)
        presetController_->requestLoadByName (name);
}

void SynthPanel::savePreset()
{
    if (presetController_ != nullptr)
        presetController_->saveCurrentPreset();
}

void SynthPanel::setPresetFavorite (const juce::String& name, bool favorite)
{
    presetCatalog_.setFavorite (name, favorite);
}

bool SynthPanel::saveCurrentPreset()
{
    return presetController_ != nullptr && presetController_->saveCurrentPreset();
}

bool SynthPanel::savePresetCopy (const juce::String& name, PresetCategory category)
{
    return presetController_ != nullptr && presetController_->savePresetCopy (name, category);
}

bool SynthPanel::renamePreset (const juce::String& oldName, const juce::String& newName)
{
    return presetController_ != nullptr && presetController_->renamePreset (oldName, newName);
}

bool SynthPanel::deletePreset (const juce::String& name)
{
    return presetController_ != nullptr && presetController_->deletePreset (name);
}

void SynthPanel::setPresetModified (bool modified)
{
    presetBar_.setModified (modified);
}

// ============================================================================
//  Teclado virtual
// ============================================================================

void SynthPanel::highlightNoteOn (int note, float velocity)
{
    applyingFeedback_ = true;
    keyboardState_.noteOn (1, note, velocity);
    applyingFeedback_ = false;
}

void SynthPanel::highlightNoteOff (int note)
{
    applyingFeedback_ = true;
    keyboardState_.noteOff (1, note, 0.0f);
    applyingFeedback_ = false;
}

void SynthPanel::clearAllNotes()
{
    applyingFeedback_ = true;
    keyboardState_.allNotesOff (1);
    applyingFeedback_ = false;
}

void SynthPanel::handleNoteOn (juce::MidiKeyboardState*, int, int note, float velocity)
{
    if (applyingFeedback_)
        return;

    noteOn_ (note, velocity);
}

void SynthPanel::handleNoteOff (juce::MidiKeyboardState*, int, int note, float)
{
    if (applyingFeedback_)
        return;

    noteOff_ (note);
}

// ============================================================================
//  Informe de layout (F11) — texto legible con bounds, solapes y textos.
// ============================================================================

juce::String SynthPanel::createLayoutReport() const
{
    juce::String report;
    int controls = 0;
    int overlaps = 0;
    int outside  = 0;
    int clipped  = 0;

    const auto font11    = ui::font (11.0f);
    const auto titleFont = ui::font (11.0f, juce::Font::bold);

    auto measure = [] (const juce::Font& font, const juce::String& text)
    {
        return juce::GlyphArrangement::getStringWidth (font, text);
    };

    auto append = [&report] (const juce::String& name,
                             const juce::Rectangle<int>& b,
                             const juce::String& text,
                             const juce::String& status)
    {
        report << "    " << name.paddedRight (' ', 16)
               << " x=" << juce::String (b.getX()).paddedLeft (' ', 4)
               << " y=" << juce::String (b.getY()).paddedLeft (' ', 4)
               << " w=" << juce::String (b.getWidth()).paddedLeft (' ', 4)
               << " h=" << juce::String (b.getHeight()).paddedLeft (' ', 4)
               << "  \"" << text << "\" " << status << "\n";
    };

    auto auditLeaf = [&] (juce::Component& leaf, juce::Point<int> offset,
                          const juce::Rectangle<int>& allowed,
                          std::vector<std::pair<juce::String, juce::Rectangle<int>>>& rects)
    {
        const auto b = leaf.getBounds().translated (offset.x, offset.y);
        const auto name = leaf.getName().isNotEmpty() ? leaf.getName()
                                                      : juce::String ("?");
        const bool inside = allowed.contains (b);
        if (! inside)
            ++outside;

        juce::String text;
        bool fits = true;

        if (auto* slider = dynamic_cast<juce::Slider*> (&leaf))
        {
            text = slider->getTextFromValue (slider->getValue());
            fits = measure (font11, text) + 2.0f
                <= static_cast<float> (slider->getTextBoxWidth());
        }
        else if (auto* combo = dynamic_cast<juce::ComboBox*> (&leaf))
        {
            text = combo->getText();
            const auto font = getLookAndFeel().getComboBoxFont (*combo);
            fits = measure (font, text) + 6.0f
                <= static_cast<float> (combo->getWidth() - 26);
        }
        else if (auto* label = dynamic_cast<juce::Label*> (&leaf))
        {
            text = label->getText();
            fits = measure (label->getFont(), text) + 2.0f
                <= static_cast<float> (label->getWidth());
        }
        else if (auto* button = dynamic_cast<juce::Button*> (&leaf))
        {
            text = button->getButtonText();

            if (button->getProperties().contains ("aeonix.iconOnly"))
            {
                // Boton solo-icono: el nombre es accesible, no texto visible.
                text.clear();
                fits = true;
            }
            else
            {
                juce::Font font (ui::font (13.0f));
                if (auto* textButton = dynamic_cast<juce::TextButton*> (button))
                    font = getLookAndFeel().getTextButtonFont (*textButton,
                                                               textButton->getHeight());

                fits = measure (font, text) + 8.0f
                    <= static_cast<float> (button->getWidth());
            }
        }
        else if (auto* editor = dynamic_cast<juce::TextEditor*> (&leaf))
        {
            text = editor->getText();
            fits = measure (editor->getFont(), text) + 8.0f
                <= static_cast<float> (editor->getWidth());
        }

        if (! fits)
            ++clipped;

        ++controls;

        append (name, b, text,
                (inside ? juce::String() : juce::String ("FUERA "))
                    + (fits ? "OK" : "CLIP"));

        for (const auto& [otherName, otherBounds] : rects)
        {
            if (b.intersects (otherBounds))
            {
                ++overlaps;
                report << "      !! solape: " << name << " <-> " << otherName << "\n";
            }
        }

        rects.emplace_back (name, b);
    };

    auto audit = [&] (const juce::Component& parent, const juce::Rectangle<int>& allowed)
    {
        std::vector<std::pair<juce::String, juce::Rectangle<int>>> rects;

        for (int i = 0; i < parent.getNumChildComponents(); ++i)
        {
            auto* child = parent.getChildComponent (i);

            if (child == nullptr || ! child->isVisible()
                || dynamic_cast<SectionPanel*> (child) != nullptr)
                continue;

            // Los widgets compuestos se auditan como sus hojas internas para
            // conservar el mismo informe que cuando eran controles sueltos.
            if (auto* bar = dynamic_cast<PresetBar*> (child))
            {
                for (int j = 0; j < bar->getNumChildComponents(); ++j)
                {
                    auto* barChild = bar->getChildComponent (j);

                    if (barChild != nullptr && barChild->isVisible())
                        auditLeaf (*barChild, bar->getPosition(), allowed, rects);
                }

                continue;
            }

            if (auto* wheels = dynamic_cast<PerformanceWheels*> (child))
            {
                for (int j = 0; j < wheels->getNumChildComponents(); ++j)
                {
                    auto* leaf = wheels->getChildComponent (j);

                    if (leaf != nullptr && leaf->isVisible())
                        auditLeaf (*leaf, wheels->getPosition(), allowed, rects);
                }

                continue;
            }

            if (auto* bed = dynamic_cast<KeyboardBed*> (child))
            {
                auditLeaf (bed->getKeyboard(), bed->getPosition(), allowed, rects);
                continue;
            }

            if (auto* knob = dynamic_cast<KnobView*> (child))
            {
                // Los ParameterKnob antiguos anadian slider y luego label;
                // los knobs discretos (voices/octave) al reves. Conservamos
                // ese orden en el informe.
                if (dynamic_cast<ParameterKnob*> (child) != nullptr)
                {
                    auditLeaf (knob->getSlider(), knob->getPosition(), allowed, rects);
                    auditLeaf (knob->getLabel(),  knob->getPosition(), allowed, rects);
                }
                else
                {
                    auditLeaf (knob->getLabel(),  knob->getPosition(), allowed, rects);
                    auditLeaf (knob->getSlider(), knob->getPosition(), allowed, rects);
                }

                continue;
            }

            if (auto* comboControl = dynamic_cast<ComboControl*> (child))
            {
                auditLeaf (comboControl->getLabel(),    comboControl->getPosition(), allowed, rects);
                auditLeaf (comboControl->getComboBox(), comboControl->getPosition(), allowed, rects);
                continue;
            }

            auditLeaf (*child, { 0, 0 }, allowed, rects);
        }
    };

    report << "SynthPanel: " << getWidth() << " x " << getHeight() << "\n";

    const SectionPanel* sections[] = {
        &oscView_, &filterView_, &filterEnvView_, &ampEnvView_,
        &lfoView_, &unisonView_, &delayView_, &reverbView_, &masterView_
    };

    for (const auto* section : sections)
    {
        const auto b = section->getBounds();
        const bool inside = getLocalBounds().contains (b);
        if (! inside)
            ++outside;

        const auto titleWidth = measure (titleFont, section->getTitle());
        const bool titleFits = titleWidth + 18.0f
                            <= static_cast<float> (section->getWidth());
        if (! titleFits)
            ++clipped;

        report << "\n[" << section->getTitle() << "]"
               << " x=" << b.getX() << " y=" << b.getY()
               << " w=" << b.getWidth() << " h=" << b.getHeight()
               << " " << (inside ? "OK" : "FUERA")
               << (titleFits ? "" : " TITLE-CLIP") << "\n";

        audit (*section, section->getContentBounds());
    }

    report << "\n[Barra / teclado]\n";
    audit (*this, getLocalBounds());

    report << "\nResumen: " << controls << " controles, "
           << overlaps << " solapes, " << outside << " fuera, "
           << clipped << " textos cortados\n";

    return report;
}

} // namespace aeonix
