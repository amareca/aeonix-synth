#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

#include "presentation/chrome/AeonixLogo.h"
#include "presentation/core/BuildFlavour.h"
#include "presentation/core/I18n.h"
#include "presentation/core/Utf8.h"
#include "presentation/theme/Theme.h"
#include "presentation/theme/Typography.h"
#include "presentation/widgets/IconButton.h"

namespace aeonix
{

// ============================================================================
//  TitleBar — chrome propio de la ventana.
//
//  Sustituye a la barra nativa del sistema (que desaparece con
//  setUsingNativeTitleBar(false)) para que la app se vea igual en Windows,
//  macOS y Linux. Contiene:
//
//    [Holo Delta + AEONIX]  ···  [Ajustes] [MIDI] | [Min] [Max] [Cerrar]
//
//  - La zona central arrastra la ventana y el doble clic maximiza/restaura.
//  - Los iconos de aplicacion van agrupados antes de un divisor para no
//    mezclarlos con los controles de ventana (y que el cierre nunca quede
//    pegado a una accion).
// ============================================================================
class TitleBar final : public juce::Component
{
public:
    static constexpr int preferredHeight = 32;
    static constexpr int lineHeight      = 2;

    TitleBar()
    {
        addAndMakeVisible (logo_);
        addAndMakeVisible (settingsButton_);
        addAndMakeVisible (midiButton_);
        addAndMakeVisible (minimiseButton_);
        addAndMakeVisible (maximiseButton_);
        addAndMakeVisible (closeButton_);

        settingsButton_.onClick  = [this] { if (onSettings)  onSettings(); };
        midiButton_.onClick      = [this] { if (onMidi)      onMidi(); };
        minimiseButton_.onClick  = [this] { if (onMinimise)  onMinimise(); };
        maximiseButton_.onClick  = [this] { if (onToggleMaximise) onToggleMaximise(); };
        closeButton_.onClick     = [this] { if (onClose)     onClose(); };

        setTitle (tr (TextId::titleBarTitle));
    }

    void setTheme (ThemeId theme)
    {
        theme_ = theme;
        settingsButton_.setTheme (theme);
        midiButton_.setTheme (theme);
        minimiseButton_.setTheme (theme);
        maximiseButton_.setTheme (theme);
        closeButton_.setTheme (theme);
        repaint();
    }

    void setMaximised (bool maximised)
    {
        if (maximised_ == maximised)
            return;

        maximised_ = maximised;
        maximiseButton_.setIcon (maximised ? IconType::restore : IconType::maximise);
        maximiseButton_.setTooltip (tr (maximised ? TextId::tooltipRestore
                                                  : TextId::tooltipMaximise));
        repaint();
    }

    // ---- acciones de la barra ----
    std::function<void()> onSettings;
    std::function<void()> onMidi;
    std::function<void()> onMinimise;
    std::function<void()> onToggleMaximise;
    std::function<void()> onClose;

    // (proporcionX, posicionEnPantalla): la ventana esta maximizada y hay que
    // restaurarla antes de empezar a arrastrar.
    std::function<void (float, juce::Point<int>)> onRestoreForDrag;

    void paint (juce::Graphics& g) override
    {
        const auto colours = themeColours (theme_);

        g.fillAll (colours.backgroundAlt.withAlpha (colours.translucentPanels ? 0.38f : 0.94f));

        g.setColour (colours.panelBorder.withAlpha (0.55f));
        g.fillRect (0, getHeight() - lineHeight - 1, getWidth(), 1);

        auto line = juce::Rectangle<int> (0, getHeight() - lineHeight, getWidth(), lineHeight);

        if (colours.gradientLines)
        {
            juce::ColourGradient gradient (holoColour (0.0f), 0.0f, 0.0f,
                                           holoColour (1.0f), static_cast<float> (getWidth()), 0.0f,
                                           false);
            g.setGradientFill (gradient);
            g.fillRect (line);
        }
        else
        {
            g.setColour (theme_ == ThemeId::prototype ? colours.accent : colours.panelBorder);
            g.fillRect (line);
        }

        if (dividerX_ > 0)
        {
            g.setColour (colours.panelBorder.withAlpha (0.8f));
            g.fillRect (dividerX_, 8, 1, juce::jmax (0, getHeight() - lineHeight - 16));
        }

        if (build::devTools)
        {
            const auto area = getLocalBounds().withTrimmedBottom (lineHeight);
            const auto badge = juce::Rectangle<int> (ui::px (172), 0, ui::px (38), ui::px (16))
                                   .withY ((area.getHeight() - ui::px (16)) / 2);

            g.setColour (colours.accent.withAlpha (0.16f));
            g.fillRoundedRectangle (badge.toFloat(), 4.0f);
            g.setColour (colours.accent.withAlpha (0.85f));
            g.drawRoundedRectangle (badge.toFloat(), 4.0f, 1.0f);
            g.setColour (colours.accent);
            g.setFont (ui::font (9.0f, juce::Font::bold));
            g.drawFittedText ("DEV", badge, juce::Justification::centred, 1);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().withTrimmedBottom (lineHeight);
        auto row = area.reduced (8, 3);

        constexpr int windowButtonWidth = 32;
        constexpr int appButtonWidth    = 30;
        constexpr int gap               = 2;

        closeButton_.setBounds (row.removeFromRight (windowButtonWidth));
        row.removeFromRight (gap);
        maximiseButton_.setBounds (row.removeFromRight (windowButtonWidth));
        row.removeFromRight (gap);
        minimiseButton_.setBounds (row.removeFromRight (windowButtonWidth));

        row.removeFromRight (8);
        dividerX_ = row.getRight() + 4;
        row.removeFromRight (8);

        midiButton_.setBounds (row.removeFromRight (appButtonWidth));
        row.removeFromRight (gap);
        settingsButton_.setBounds (row.removeFromRight (appButtonWidth));

        logo_.setBounds (14, 0, 150, area.getHeight());
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
            return;

        pendingRestore_ = maximised_;
        pendingProportion_ = static_cast<float> (e.x) / static_cast<float> (juce::jmax (1, getWidth()));

        // Ojo: e.getScreenPosition() deja de ser fiable en cuanto movemos la
        // ventana durante el arrastre. Usamos siempre el raton actual.
        dragStartScreen_ = currentMouseScreen();
        dragStartWindow_ = getTopLevelComponent()->getBounds();
        dragging_ = true;
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        juce::ignoreUnused (e);

        if (! dragging_)
            return;

        auto* window = getTopLevelComponent();
        const auto mouseScreen = currentMouseScreen();

        if (pendingRestore_)
        {
            pendingRestore_ = false;

            if (onRestoreForDrag)
                onRestoreForDrag (pendingProportion_, mouseScreen);

            // La ventana acaba de cambiar de tamano y posicion: reanclamos el
            // arrastre al punto actual para que no salte.
            dragStartScreen_ = mouseScreen;
            dragStartWindow_ = window->getBounds();
            return;
        }

        window->setBounds (dragStartWindow_ + (mouseScreen - dragStartScreen_));
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        dragging_ = false;
        pendingRestore_ = false;
    }

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        if (onToggleMaximise)
            onToggleMaximise();
    }

private:
    [[nodiscard]] static juce::Point<int> currentMouseScreen()
    {
        return juce::Desktop::getInstance().getMainMouseSource().getScreenPosition().roundToInt();
    }

    AeonixLogo logo_;

    IconButton settingsButton_ { IconType::settings, tr (TextId::tooltipSettings) };
    IconButton midiButton_     { IconType::midi,     tr (TextId::tooltipMidiDevices) };
    IconButton minimiseButton_ { IconType::minimise, tr (TextId::tooltipMinimise) };
    IconButton maximiseButton_ { IconType::maximise, tr (TextId::tooltipMaximise) };
    IconButton closeButton_    { IconType::close,    tr (TextId::tooltipClose) };

    juce::Point<int> dragStartScreen_;
    juce::Rectangle<int> dragStartWindow_;
    ThemeId theme_ = ThemeId::holoAccents;
    bool maximised_ = false;
    bool pendingRestore_ = false;
    bool dragging_ = false;
    float pendingProportion_ = 0.5f;
    int dividerX_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleBar)
};

} // namespace aeonix
