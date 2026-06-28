#include "settings_manager.h"
#include <fstream>
#include <iostream>

namespace cs {

SettingsManager& SettingsManager::instance() {
    static SettingsManager inst;
    return inst;
}

void SettingsManager::setString(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    settings_[key] = value;
}

std::string SettingsManager::getString(const std::string& key, const std::string& default_value) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        return it->second;
    }
    return default_value;
}

void SettingsManager::setInt(const std::string& key, int32_t value) {
    setString(key, std::to_string(value));
}

int32_t SettingsManager::getInt(const std::string& key, int32_t default_value) {
    std::string val = getString(key, "");
    if (val.empty()) return default_value;
    try {
        return std::stoi(val);
    } catch (...) {
        return default_value;
    }
}

void SettingsManager::save() {
    // Simple implementation for MVP
    std::ofstream file("settings.txt");
    for (const auto& pair : settings_) {
        file << pair.first << "=" << pair.second << "\n";
    }
}

void SettingsManager::load() {
    std::ifstream file("settings.txt");
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            settings_[line.substr(0, pos)] = line.substr(pos + 1);
        }
    }
}

} // namespace cs
