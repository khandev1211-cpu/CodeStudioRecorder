#pragma once
#include "i_frame_processor.h"
#include <string>

namespace cs {

/**
 * Interface for CodeStudio Plugins.
 * Plugins must export a function:
 * extern "C" __declspec(dllexport) cs::IFrameProcessor* create_plugin();
 */
struct PluginInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
};

// Optional: Plugins can also export info
// extern "C" __declspec(dllexport) void get_plugin_info(cs::PluginInfo* info);

} // namespace cs
