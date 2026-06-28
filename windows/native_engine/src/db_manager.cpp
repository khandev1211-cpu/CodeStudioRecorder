#include "db_manager.h"
#include "cs_logger.h"
#include <iostream>

// Simplified mock implementation of DbManager while SQLite3 binary is being linked.
// In a full environment, we would use sqlite3_open, sqlite3_exec, etc.

namespace cs {

DbManager& DbManager::instance() {
    static DbManager inst;
    return inst;
}

bool DbManager::initialize(const std::string& db_path) {
    CS_LOG_INFO("Initializing SQLite Database at: " + db_path);
    // 1. sqlite3_open(db_path.c_str(), &db_);
    // 2. CREATE TABLE IF NOT EXISTS recordings...
    // 3. CREATE TABLE IF NOT EXISTS settings...
    return true;
}

bool DbManager::saveRecording(const DbRecording& rec) {
    std::lock_guard<std::mutex> lock(mutex_);
    CS_LOG_INFO("DB: Saving recording " + rec.title);
    return true;
}

std::vector<DbRecording> DbManager::getHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    return {}; // Mock empty history
}

bool DbManager::setSetting(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    // UPDATE settings SET value = ? WHERE key = ?
    return true;
}

std::string DbManager::getSetting(const std::string& key, const std::string& default_val) {
    std::lock_guard<std::mutex> lock(mutex_);
    // SELECT value FROM settings WHERE key = ?
    return default_val;
}

} // namespace cs
