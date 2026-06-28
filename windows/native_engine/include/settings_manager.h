#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

namespace cs {

class SettingsManager {
public:
    static SettingsManager& instance();

    void setString(const std::string& key, const std::string& value);
    std::string getString(const std::string& key, const std::string& default_value = "");

    void setInt(const std::string& key, int32_t value);
    int32_t getInt(const std::string& key, int32_t default_value = 0);

    void save();
    void load();

private:
    SettingsManager() = default;
    std::unordered_map<std::string, std::string> settings_;
    std::mutex mutex_;
};

} // namespace cs
