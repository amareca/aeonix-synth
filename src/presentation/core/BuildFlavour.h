#pragma once

// ============================================================================
//  BuildFlavour - tells PRO and DEV apart at compile time.
//
//  PRO (the default) does not include Snapshot, Layout, Abrir capturas or
//  F11/F12 and uses the product data. DEV adds the tools and works on
//  separate folders so presets and configuration are never mixed.
// ============================================================================

#if defined(AEONIX_DEV_TOOLS) && AEONIX_DEV_TOOLS
#define AEONIX_HAS_DEV_TOOLS 1
#else
#define AEONIX_HAS_DEV_TOOLS 0
#endif

namespace aeonix::build
{

inline constexpr bool devTools = AEONIX_HAS_DEV_TOOLS != 0;

// Folder under Documents and %APPDATA%.
inline constexpr const char* dataFolder = devTools ? "AeonixDev" : "Aeonix";

inline constexpr const char* configFile = "config.json";
inline constexpr const char* presetsFolder = "Presets";

} // namespace aeonix::build
