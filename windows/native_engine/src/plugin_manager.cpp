#include "plugin_manager.h"
#include "cs_logger.h"
#include <filesystem>

namespace cs {

namespace fs = std::filesystem;

PluginManager& PluginManager::instance() {
    static PluginManager inst;
    return inst;
}

PluginManager::~PluginManager() {
    unloadPlugins();
}

typedef IFrameProcessor* (*CreatePluginFunc)();

void PluginManager::loadPlugins(const std::string& directory) {
    if (!fs::exists(directory)) {
        CS_LOG_INFO("Plugin directory does not exist: " + directory);
        return;
    }

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.path().extension() == ".dll") {
            std::wstring path = entry.path().wstring();
            HMODULE hMod = LoadLibraryW(path.c_str());

            if (hMod) {
                CreatePluginFunc createFunc = (CreatePluginFunc)GetProcAddress(hMod, "create_plugin");
                if (createFunc) {
                    IFrameProcessor* processor = createFunc();
                    if (processor) {
                        plugins_.push_back(std::unique_ptr<IFrameProcessor>(processor));
                        modules_.push_back(hMod);
                        CS_LOG_INFO("Successfully loaded plugin: " + entry.path().filename().string());
                    } else {
                        FreeLibrary(hMod);
                    }
                } else {
                    FreeLibrary(hMod);
                }
            }
        }
    }
}

void PluginManager::unloadPlugins() {
    plugins_.clear(); // Unique pointers will delete the processors
    for (auto hMod : modules_) {
        FreeLibrary(hMod);
    }
    modules_.clear();
}

} // namespace cs
