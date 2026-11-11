cmake_minimum_required(VERSION 3.24)

project(Aeonix
    VERSION 0.1.0
    DESCRIPTION "AEONIX - sintetizador sustractivo cinematografico (standalone)"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AEONIX_BUILD_TESTS "Compila y registra los unit tests del nucleo" ON)
option(AEONIX_DEV_TOOLS "Compila las herramientas de desarrollo (F11/F12, snapshots y badge DEV)" OFF)

# ==============================================================================
#  NUCLEO PURO (dominio) — C++ sin JUCE, sin I/O, sin GUI.
# ==============================================================================
add_library(aeonix_core STATIC
    src/domain/Parameter.h
    src/domain/ParameterIds.h
    src/domain/ParameterFormat.h
    src/domain/MathUtils.h
    src/domain/Patch.cpp
    src/domain/Patch.h
    src/domain/Oscillator.h
    src/domain/Filter.h
    src/domain/Envelope.h
    src/domain/Lfo.h
    src/domain/SpScQueue.h
    src/domain/Delay.h
    src/domain/Reverb.h
    src/domain/Preset.cpp
    src/domain/Preset.h
    src/domain/Voice.cpp
    src/domain/Voice.h
    src/domain/SynthEngine.cpp
    src/domain/SynthEngine.h
)

target_include_directories(aeonix_core PUBLIC src)
target_compile_features(aeonix_core PUBLIC cxx_std_20)

# ==============================================================================
#  JUCE (submodulo fijado en external/JUCE, tag 8.0.15)
# ==============================================================================
add_subdirectory(external/JUCE)

# ==============================================================================
#  Aplicacion standalone: ventana + audio + MIDI
# ==============================================================================
if(AEONIX_DEV_TOOLS)
    set(AEONIX_PRODUCT_NAME "AEONIX-dev")
else()
    set(AEONIX_PRODUCT_NAME "AEONIX")
endif()

juce_add_gui_app(Aeonix
    PRODUCT_NAME "${AEONIX_PRODUCT_NAME}"
    COMPANY_NAME "Aeonix"
    VERSION     "0.1.0"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
)

target_sources(Aeonix PRIVATE
    src/Main.cpp
    src/MainComponent.cpp
    src/MainComponent.h
    src/infrastructure/ConfigManager.h
    src/infrastructure/PresetLibrary.cpp
    src/infrastructure/PresetLibrary.h
    src/infrastructure/PresetRepository.h
    src/presentation/SynthPanel.cpp
    src/presentation/SynthPanel.h
    src/presentation/chrome/AeonixLogo.h
    src/presentation/chrome/AeonixLookAndFeel.h
    src/presentation/chrome/SettingsOverlay.h
    src/presentation/chrome/TitleBar.h
    src/presentation/chrome/Toast.h
    src/presentation/core/Observable.h
    src/presentation/core/BuildFlavour.h
    src/presentation/core/Utf8.h
    src/presentation/core/Language.h
    src/presentation/core/I18n.h
    src/presentation/core/I18n.cpp
    src/presentation/core/StringCatalog.inc
    src/presentation/core/ViewModels.h
    src/presentation/keyboard/AeonixKeyboard.h
    src/presentation/keyboard/KeyboardBed.cpp
    src/presentation/keyboard/KeyboardBed.h
    src/presentation/patch/ParameterMetadata.h
    src/presentation/patch/PatchBinder.cpp
    src/presentation/patch/PatchBinder.h
    src/presentation/patch/SynthViewModels.h
    src/presentation/presets/PresetBar.cpp
    src/presentation/presets/PresetBar.h
    src/presentation/presets/PresetCatalog.cpp
    src/presentation/presets/PresetCatalog.h
    src/presentation/presets/PresetController.cpp
    src/presentation/presets/PresetController.h
    src/presentation/presets/PresetManagerOverlay.cpp
    src/presentation/presets/PresetManagerOverlay.h
    src/presentation/sections/EffectViews.cpp
    src/presentation/sections/EffectViews.h
    src/presentation/sections/MasterView.cpp
    src/presentation/sections/MasterView.h
    src/presentation/sections/ModulationViews.cpp
    src/presentation/sections/ModulationViews.h
    src/presentation/sections/SectionLayout.h
    src/presentation/sections/SectionPanel.h
    src/presentation/sections/SynthesisViews.cpp
    src/presentation/sections/SynthesisViews.h
    src/presentation/sections/Waveforms.h
    src/presentation/theme/Theme.h
    src/presentation/widgets/ComboControl.h
    src/presentation/widgets/IconButton.h
    src/presentation/widgets/KnobView.h
    src/presentation/widgets/PerformanceWheels.cpp
    src/presentation/widgets/PerformanceWheels.h
    src/presentation/widgets/ThemedDialog.cpp
    src/presentation/widgets/ThemedDialog.h
)

target_compile_definitions(Aeonix PRIVATE
    AEONIX_DEV_TOOLS=$<BOOL:${AEONIX_DEV_TOOLS}>
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Aeonix,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Aeonix,JUCE_VERSION>"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

# Los literales de la interfaz estan en UTF-8 (tildes, signos invertidos, etc.).
target_compile_options(Aeonix PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_link_libraries(Aeonix PRIVATE
    aeonix_core
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
)

# ==============================================================================
#  Unit tests (Catch2 v3 + CTest)
# ==============================================================================
if(AEONIX_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(aeonix_tests
        tests/test_Parameter.cpp
        tests/test_ParameterFormat.cpp
        tests/test_Patch.cpp
        tests/test_MathUtils.cpp
        tests/test_Oscillator.cpp
        tests/test_Envelope.cpp
        tests/test_Filter.cpp
        tests/test_Lfo.cpp
        tests/test_SpScQueue.cpp
        tests/test_Delay.cpp
        tests/test_Reverb.cpp
        tests/test_Preset.cpp
        tests/test_PresetLibrary.cpp
        tests/test_SynthEngine.cpp
        tests/test_Stability.cpp
        tests/test_Aliasing.cpp
        tests/test_Limiter.cpp
        tests/test_TextSize.cpp
        tests/test_I18n.cpp
        src/presentation/core/I18n.cpp
        src/infrastructure/PresetLibrary.cpp
    )

    target_compile_options(aeonix_tests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

    target_link_libraries(aeonix_tests PRIVATE
        aeonix_core
        juce::juce_core
        Catch2::Catch2WithMain
    )

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(aeonix_tests)
endif()
