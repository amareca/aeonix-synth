#include <juce_audio_utils/juce_audio_utils.h>
#include "MainComponent.h"
#include "infrastructure/ConfigManager.h"
#include "presentation/chrome/AeonixLogo.h"
#include "presentation/chrome/TitleBar.h"
#include "presentation/core/BuildFlavour.h"
#include "presentation/core/I18n.h"
#include "presentation/core/TextSize.h"
#include "presentation/theme/Typography.h"

// ==============================================================================
//  AEONIX - entry point of the standalone application (JUCE).
//
//  JUCEApplication is the class that starts the message loop, the window
//  and the app lifecycle. Here we only create the main window.
//
//  The window uses a custom chrome (no native bar) so the appearance is
//  the same on Windows, macOS and Linux. The bar is drawn by
//  aeonix::TitleBar inside MainComponent.
// ==============================================================================

class AeonixApplication final : public juce::JUCEApplication
{
public:
    AeonixApplication() = default;

    const juce::String getApplicationName() override
    {
        return aeonix::build::devTools ? "AEONIX-dev" : "AEONIX";
    }
    const juce::String getApplicationVersion() override { return "0.2.0"; }

    // One instance per flavour: PRO and DEV can coexist because they use
    // different instance names and data folders.
    bool moreThanOneInstanceAllowed() override { return false; }

    // Called when the app starts: here we create the main window.
    void initialise (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);

        // Language: explicit key or system detection (English as fallback).
        // Must be set before building the UI: the texts are assigned
        // in the component constructors.
        auto& config = aeonix::ConfigManager::getInstance();
        const auto savedLanguage = config.getString ("ui.language");

        aeonix::setLanguage (savedLanguage.isNotEmpty()
                                 ? aeonix::languageFromKey (savedLanguage)
                                 : aeonix::detectSystemLanguage());
        aeonix::installJuceLocalisation (aeonix::getLanguage());

        // Text size: saved choice or detection by screen height.
        // Must be set before building the UI.
        const auto savedTextSize = config.getString ("ui.textSize");

        if (savedTextSize.isNotEmpty())
        {
            aeonix::ui::setTextSize (aeonix::textSizeFromKey (savedTextSize));
        }
        else
        {
            int screenHeight = 1080;

            if (auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
                screenHeight = juce::roundToInt (display->userArea.getHeight() * display->scale);

            aeonix::ui::setTextSize (aeonix::defaultTextSizeForHeight (screenHeight));
        }

        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    // Called when the app closes.
    void shutdown() override
    {
        mainWindow = nullptr;   // destroys the window and its content
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String&) override {}

private:
    // Top-level window with custom chrome.
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& name)
            : juce::DocumentWindow (name,
                                    juce::Colour (0xff1b1f27),
                                    juce::DocumentWindow::allButtons)
        {
            // No native bar: it is drawn by TitleBar inside MainComponent.
            setUsingNativeTitleBar (false);
            setTitleBarHeight (0);
            setTitleBarButtonsRequired (0, false);

            auto* content = new MainComponent();

            content->onWindowMinimise = [this]
            {
                if (auto* peer = getPeer())
                    peer->setMinimised (true);
            };

            content->onWindowToggleMaximise = [this] { setMaximised (! maximised_); };

            content->onWindowClose = [this] { closeButtonPressed(); };

            content->onWindowRestoreForDrag = [this] (float proportion, juce::Point<int> screenPos)
            {
                if (! maximised_)
                    return;

                setMaximised (false);

                // Places the restored window under the cursor, as the OS does.
                const auto bounds = getBounds();
                setBounds (screenPos.x - juce::roundToInt (proportion * static_cast<float> (bounds.getWidth())),
                           screenPos.y - 14,
                           bounds.getWidth(), bounds.getHeight());
            };

            setContentOwned (content, true);

            setResizable (true, false);   // edges: native on Win/mac, JUCE on Linux

            const auto textSize = aeonix::ui::getTextSize();
            setResizeLimits (aeonix::minWindowWidthForSize (textSize),
                             aeonix::minWindowHeightForSize (textSize),
                             2402, 1616);

            auto& config = aeonix::ConfigManager::getInstance();

            // Height migration: older configs did not reserve the custom
            // bar, so we add its height only once. The peer without a
            // frame adds 1 px of border per side: +2 width and +2 height.
            const bool hasCustomChrome = config.getInt ("window.chrome", 0) != 0;

            int width  = config.getInt ("window.w", aeonix::defaultWindowWidthForSize (textSize));
            int height = config.getInt ("window.h", aeonix::defaultWindowHeightForSize (textSize));

            if (! hasCustomChrome)
            {
                width  += 2;
                height += aeonix::ui::px (aeonix::TitleBar::preferredHeight) + 2;
                config.setInt ("window.chrome", 1);
                config.save();
            }

            width  = juce::jlimit (aeonix::minWindowWidthForSize (textSize), 2402, width);
            height = juce::jlimit (aeonix::minWindowHeightForSize (textSize), 1616, height);

            if (config.getInt ("window.valid", 0) != 0)
                setBoundsConstrained ({ config.getInt ("window.x", 100),
                                        config.getInt ("window.y", 100),
                                        width, height });
            else
                centreWithSize (width, height);

            setVisible (true);

            if (config.getInt ("window.maximised", 0) != 0)
                setMaximised (true);

            // Window / taskbar icon (Holo Delta).
            if (auto* peer = getPeer())
                peer->setIcon (aeonix::createAeonixAppIcon (128));
        }

        ~MainWindow() override
        {
            // Also saves if the close comes through a system path
            // (shutdown, log off, etc.) and not through the custom button.
            saveWindowState();
        }

        void closeButtonPressed() override
        {
            saveWindowState();
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

        void setMaximised (bool shouldBeMaximised)
        {
            if (maximised_ == shouldBeMaximised)
                return;

            if (shouldBeMaximised)
            {
                restoreBounds_ = getBounds();
                maximised_ = true;

                const auto& displays = juce::Desktop::getInstance().getDisplays();
                auto* display = displays.getDisplayForRect (getScreenBounds());

                if (display == nullptr)
                    display = displays.getPrimaryDisplay();

                if (display != nullptr)
                    setBounds (display->userArea);
            }
            else
            {
                maximised_ = false;
                setBounds (restoreBounds_);
            }

            if (auto* content = dynamic_cast<MainComponent*> (getContentComponent()))
                content->setWindowMaximised (maximised_);
        }

    private:
        void saveWindowState()
        {
            auto& config = aeonix::ConfigManager::getInstance();

            // If maximized we save the restore size, not the
            // screen one, so the next session opens the same.
            const auto bounds = maximised_ ? restoreBounds_ : getBounds();

            config.setInt ("window.x", bounds.getX());
            config.setInt ("window.y", bounds.getY());
            config.setInt ("window.w", bounds.getWidth());
            config.setInt ("window.h", bounds.getHeight());
            config.setInt ("window.maximised", maximised_ ? 1 : 0);
            config.setInt ("window.chrome", 1);
            config.setInt ("window.valid", 1);
            config.save();
        }

        juce::Rectangle<int> restoreBounds_;
        bool maximised_ = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

// Macro that generates the application's main() (hides the Windows main()).
START_JUCE_APPLICATION (AeonixApplication)
