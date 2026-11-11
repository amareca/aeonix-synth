#pragma once

// ============================================================================
//  BuildFlavour — diferencia PRO y DEV en tiempo de compilacion.
//
//  PRO (por defecto) no incluye Snapshot, Layout, Abrir capturas ni F11/F12 y
//  usa los datos del producto. DEV anade las herramientas y trabaja sobre
//  carpetas separadas para no mezclar presets ni configuracion.
// ============================================================================

#if defined(AEONIX_DEV_TOOLS) && AEONIX_DEV_TOOLS
#define AEONIX_HAS_DEV_TOOLS 1
#else
#define AEONIX_HAS_DEV_TOOLS 0
#endif

namespace aeonix::build
{

inline constexpr bool devTools = AEONIX_HAS_DEV_TOOLS != 0;

// Carpeta bajo Documentos y %APPDATA%.
inline constexpr const char* dataFolder = devTools ? "AeonixDev" : "Aeonix";

inline constexpr const char* configFile = "config.json";
inline constexpr const char* presetsFolder = "Presets";

} // namespace aeonix::build
