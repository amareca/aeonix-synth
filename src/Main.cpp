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
//  AEONIX — punto de entrada de la aplicacion standalone (JUCE).
//
//  JUCEApplication es la clase que arranca el bucle de mensajes, la ventana
//  y el ciclo de vida de la app. Aqui solo creamos la ventana principal.
//
//  La ventana usa un chrome propio (sin barra nativa) para que la apariencia
//  sea la misma en Windows, macOS y Linux. La barra la dibuja
//  aeonix::TitleBar dentro de MainComponent.
// ==============================================================================

class AeonixApplication final : public juce::JUCEApplication
{
public:
    AeonixApplication() = default;

    const juce::String getApplicationName() override
    {
        return aeonix::build::devTools ? "AEONIX-dev" : "AEONIX";
    }
    const juce::String getApplicationVersion() override { return "0.1.0"; }

    // Una sola instancia por flavour: PRO y DEV pueden convivir porque usan
    // nombres de instancia y carpetas de datos distintas.
    bool moreThanOneInstanceAllowed() override { return false; }

    // Se llama al arrancar la app: aqui creamos la ventana principal.
    void initialise (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);

        // Idioma: clave explicita o deteccion del sistema (ingles de reserva).
        // Debe fijarse antes de construir la interfaz: los textos se asignan
        // en los constructores de los componentes.
        auto& config = aeonix::ConfigManager::getInstance();
        const auto savedLanguage = config.getString ("ui.language");

        aeonix::setLanguage (savedLanguage.isNotEmpty()
                                 ? aeonix::languageFromKey (savedLanguage)
                                 : aeonix::detectSystemLanguage());
        aeonix::installJuceLocalisation (aeonix::getLanguage());

        // Tamano de texto: eleccion guardada o deteccion por altura de pantalla.
        // Debe fijarse antes de construir la interfaz.
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

    // Se llama al cerrar la app.
    void shutdown() override
    {
        mainWindow = nullptr;   // destruye la ventana y su contenido
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String&) override {}

private:
    // Ventana de nivel superior con chrome propio.
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& name)
            : juce::DocumentWindow (name,
                                    juce::Colour (0xff1b1f27),
                                    juce::DocumentWindow::allButtons)
        {
            // Sin barra nativa: la dibuja TitleBar dentro de MainComponent.
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

                // Coloca la ventana restaurada bajo el cursor, como hace el SO.
                const auto bounds = getBounds();
                setBounds (screenPos.x - juce::roundToInt (proportion * static_cast<float> (bounds.getWidth())),
                           screenPos.y - 14,
                           bounds.getWidth(), bounds.getHeight());
            };

            setContentOwned (content, true);

            setResizable (true, false);   // bordes: nativos en Win/mac, JUCE en Linux

            const auto textSize = aeonix::ui::getTextSize();
            setResizeLimits (aeonix::minWindowWidthForSize (textSize),
                             aeonix::minWindowHeightForSize (textSize),
                             2402, 1616);

            auto& config = aeonix::ConfigManager::getInstance();

            // Migracion de altura: las configs anteriores no reservaban la
            // barra propia, asi que sumamos su alto una sola vez. El peer sin
            // marco anade 1 px de borde por lado: +2 de ancho y +2 de alto.
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

            // Icono de la ventana / barra de tareas (Holo Delta).
            if (auto* peer = getPeer())
                peer->setIcon (aeonix::createAeonixAppIcon (128));
        }

        ~MainWindow() override
        {
            // Guarda tambien si el cierre llega por una ruta del sistema
            // (apagado, cierre de sesion, etc.) y no por el boton propio.
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

            // Si esta maximizada guardamos el tamano de restauracion, no el
            // de la pantalla, para que la proxima sesion abra igual.
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

// Macro que genera el main() de la aplicacion (oculta el main() de Windows).
START_JUCE_APPLICATION (AeonixApplication)
