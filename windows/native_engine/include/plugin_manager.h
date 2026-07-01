#pragma once
#include <vector>
#include <memory>
#include <string>
#include <windows.h>
#include "i_frame_processor.h"

namespace cs {

class PluginManager {
public:
    static PluginManager& instance();

    void loadPlugins(const std::string& directory);
    void unloadPlugins();

    const std::vector<std::unique_ptr<IFrameProcessor>>& getPlugins() const {
        return plugins_;
    }

private:
    PluginManager() = default;
    ~PluginManager();

    std::vector<HMODULE> modules_;
    std::vector<std::unique_ptr<IFrameProcessor>> plugins_;
};

} // namespace cs
