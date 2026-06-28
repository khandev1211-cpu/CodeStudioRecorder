#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "cs_types.h"

// Note: Requires sqlite3.h
struct sqlite3;

namespace cs {

struct DbRecording {
    std::string id;
    std::string title;
    std::string file_path;
    int64_t duration_ms;
    int64_t file_size;
    int64_t created_at;
};

class DbManager {
public:
    static DbManager& instance();

    bool initialize(const std::string& db_path);

    // Recording History
    bool saveRecording(const DbRecording& rec);
    std::vector<DbRecording> getHistory();
    bool deleteRecording(const std::string& id);

    // Settings (Replacing SettingsManager simple text file)
    bool setSetting(const std::string& key, const std::string& value);
    std::string getSetting(const std::string& key, const std::string& default_val = "");

private:
    DbManager() = default;
    sqlite3* db_ = nullptr;
    std::mutex mutex_;
};

} // namespace cs
