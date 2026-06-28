# 07 — Database

## CodeStudio Recorder — Database Layer

---

## Table of Contents

1. [Overview](#overview)
2. [Technology Choice](#technology-choice)
3. [Schema Design](#schema-design)
4. [Tables Reference](#tables-reference)
5. [Settings Storage](#settings-storage)
6. [Recording History](#recording-history)
7. [Export History](#export-history)
8. [User Profiles](#user-profiles)
9. [Migrations](#migrations)
10. [Configuration Management](#configuration-management)

---

## Overview

The Database layer handles all persistent application state: user settings, recording history, export history, plugin data, and application configuration. It is designed to be lightweight, reliable, and schema-versioned for safe evolution.

---

## Technology Choice

**SQLite** is the database engine for CodeStudio Recorder.

| Criterion | SQLite | Rationale |
|---|---|---|
| Deployment | Zero-config, embedded | No server process needed |
| File size | <1MB for most users | Minimal disk footprint |
| Reliability | ACID-compliant | Safe against crashes |
| Performance | Sufficient for recording metadata | Not a high-throughput DB |
| Integration | C API, widely supported | Easy FFI from C++ |

The database file is stored at:
```
%APPDATA%\CodeStudioRecorder\data\codestudio.db
```

---

## Schema Design

### Design Principles

- **No data loss:** Recordings are never deleted from history unless user explicitly requests
- **Schema versioning:** Every schema change is tracked in `schema_migrations` table
- **Normalized design:** Avoid duplicating data across tables
- **Soft deletes:** Most records support `deleted_at` rather than hard deletes
- **UTF-8 everywhere:** All text stored as UTF-8

---

## Tables Reference

### `recordings`

Core recording history table.

```sql
CREATE TABLE recordings (
    id              TEXT    PRIMARY KEY,  -- UUID v4
    title           TEXT    NOT NULL DEFAULT '',
    file_path       TEXT    NOT NULL,
    file_size       INTEGER NOT NULL DEFAULT 0,     -- bytes
    duration_ms     INTEGER NOT NULL DEFAULT 0,     -- milliseconds
    
    width           INTEGER NOT NULL,
    height          INTEGER NOT NULL,
    fps             INTEGER NOT NULL,
    codec           TEXT    NOT NULL,
    
    audio_enabled   INTEGER NOT NULL DEFAULT 1,
    mic_enabled     INTEGER NOT NULL DEFAULT 1,
    system_audio_enabled INTEGER NOT NULL DEFAULT 1,
    
    capture_mode    TEXT    NOT NULL,    -- 'fullscreen'|'window'|'region'
    capture_target  TEXT,               -- window title or monitor ID
    
    status          TEXT    NOT NULL DEFAULT 'completed',
    -- 'recording'|'completed'|'partial'|'failed'
    
    thumbnail_path  TEXT,
    
    created_at      TEXT    NOT NULL,   -- ISO 8601
    updated_at      TEXT    NOT NULL,
    deleted_at      TEXT,               -- soft delete
    
    tags            TEXT    DEFAULT '[]',  -- JSON array of strings
    notes           TEXT    DEFAULT '',
    
    metadata        TEXT    DEFAULT '{}'   -- JSON blob for extensibility
);

CREATE INDEX idx_recordings_created_at ON recordings(created_at DESC);
CREATE INDEX idx_recordings_status     ON recordings(status);
CREATE INDEX idx_recordings_deleted_at ON recordings(deleted_at);
```

---

### `exports`

Tracks every export job performed on a recording.

```sql
CREATE TABLE exports (
    id              TEXT    PRIMARY KEY,
    recording_id    TEXT    NOT NULL REFERENCES recordings(id),
    
    output_path     TEXT    NOT NULL,
    output_size     INTEGER NOT NULL DEFAULT 0,
    
    preset_name     TEXT    NOT NULL,    -- e.g. 'YouTube 1080p', 'Instagram Reel'
    codec           TEXT    NOT NULL,
    container       TEXT    NOT NULL,
    
    width           INTEGER NOT NULL,
    height          INTEGER NOT NULL,
    fps             INTEGER NOT NULL,
    bitrate         INTEGER NOT NULL,
    
    audio_codec     TEXT,
    audio_bitrate   INTEGER,
    
    duration_ms     INTEGER NOT NULL DEFAULT 0,
    processing_time_ms INTEGER NOT NULL DEFAULT 0,
    
    status          TEXT    NOT NULL DEFAULT 'pending',
    -- 'pending'|'processing'|'completed'|'failed'
    error_message   TEXT,
    
    created_at      TEXT    NOT NULL,
    completed_at    TEXT
);

CREATE INDEX idx_exports_recording_id ON exports(recording_id);
CREATE INDEX idx_exports_status       ON exports(status);
```

---

### `settings`

Key-value settings store. All application settings are stored here.

```sql
CREATE TABLE settings (
    key         TEXT    PRIMARY KEY,
    value       TEXT    NOT NULL,
    value_type  TEXT    NOT NULL DEFAULT 'string',
    -- 'string'|'integer'|'float'|'boolean'|'json'
    
    category    TEXT    NOT NULL DEFAULT 'general',
    description TEXT    DEFAULT '',
    updated_at  TEXT    NOT NULL
);
```

**Default settings:**

```sql
INSERT INTO settings (key, value, value_type, category) VALUES
  ('recording.default_fps',         '60',         'integer', 'recording'),
  ('recording.default_codec',       '"h264"',     'string',  'recording'),
  ('recording.default_bitrate',     '10000000',   'integer', 'recording'),
  ('recording.capture_mode',        '"window"',   'string',  'recording'),
  ('recording.output_directory',    '""',         'string',  'recording'),
  ('audio.mic_enabled',             'true',       'boolean', 'audio'),
  ('audio.system_enabled',          'true',       'boolean', 'audio'),
  ('audio.mic_gain',                '1.0',        'float',   'audio'),
  ('audio.system_gain',             '1.0',        'float',   'audio'),
  ('ui.theme',                      '"dark"',     'string',  'ui'),
  ('ui.language',                   '"en"',       'string',  'ui'),
  ('hotkeys.start_stop',            '"F9"',       'string',  'hotkeys'),
  ('hotkeys.pause_resume',          '"F10"',      'string',  'hotkeys');
```

---

### `profiles`

User-defined recording profiles (saved configurations).

```sql
CREATE TABLE profiles (
    id          TEXT    PRIMARY KEY,
    name        TEXT    NOT NULL,
    description TEXT    DEFAULT '',
    is_default  INTEGER NOT NULL DEFAULT 0,
    
    -- Recording config (JSON blob)
    config      TEXT    NOT NULL DEFAULT '{}',
    
    created_at  TEXT    NOT NULL,
    updated_at  TEXT    NOT NULL
);
```

**Example profile config JSON:**
```json
{
  "fps": 60,
  "codec": "h264",
  "bitrate": 12000000,
  "captureMode": "window",
  "audio": {
    "micEnabled": true,
    "systemEnabled": false,
    "micGain": 1.2
  },
  "export": {
    "preset": "YouTube 1080p",
    "autoExport": false
  }
}
```

---

### `tags`

Tag registry for organizing recordings.

```sql
CREATE TABLE tags (
    id          TEXT    PRIMARY KEY,
    name        TEXT    NOT NULL UNIQUE,
    color       TEXT    NOT NULL DEFAULT '#4A90D9',
    created_at  TEXT    NOT NULL
);
```

---

### `schema_migrations`

Tracks applied schema migrations.

```sql
CREATE TABLE schema_migrations (
    version     INTEGER PRIMARY KEY,
    description TEXT    NOT NULL,
    applied_at  TEXT    NOT NULL
);
```

---

### `plugin_data`

Key-value store for plugins to persist their own data.

```sql
CREATE TABLE plugin_data (
    plugin_id   TEXT    NOT NULL,
    key         TEXT    NOT NULL,
    value       TEXT    NOT NULL,
    updated_at  TEXT    NOT NULL,
    
    PRIMARY KEY (plugin_id, key)
);
```

---

## Settings Storage

### SettingsManager

```cpp
class SettingsManager {
public:
    static SettingsManager& instance();
    
    template<typename T>
    T    get(const std::string& key, T defaultValue) const;
    
    template<typename T>
    void set(const std::string& key, const T& value);
    
    void reset(const std::string& key);
    void resetAll();
    
    // Watch for changes (Flutter UI subscription)
    using ChangeCallback = std::function<void(std::string, std::string)>;
    void subscribe(const std::string& keyPattern, ChangeCallback cb);
    void unsubscribe(SubscriptionHandle handle);

private:
    sqlite3*                             db_;
    std::unordered_map<std::string, std::string> cache_;
    mutable std::shared_mutex            mutex_;
};
```

All settings reads are served from the in-memory cache. Writes are committed to SQLite immediately and the cache is updated.

---

## Recording History

### RecordingRepository

```cpp
class RecordingRepository {
public:
    std::string   insert(const Recording& rec);
    Recording     findById(const std::string& id);
    bool          update(const Recording& rec);
    bool          softDelete(const std::string& id);
    bool          hardDelete(const std::string& id);
    
    std::vector<Recording> findAll(
        int limit  = 50,
        int offset = 0,
        const std::string& sortBy = "created_at DESC",
        const RecordingFilter& filter = {}
    );
    
    int           count(const RecordingFilter& filter = {});
    uint64_t      totalSize(); // bytes of all recording files
};

struct RecordingFilter {
    std::optional<std::string> status;
    std::optional<std::string> captureMode;
    std::optional<std::string> tagName;
    std::optional<std::string> dateFrom;
    std::optional<std::string> dateTo;
    std::optional<std::string> search; // searches title + notes
};
```

---

## Export History

### ExportRepository

```cpp
class ExportRepository {
public:
    std::string  insert(const Export& exp);
    Export       findById(const std::string& id);
    bool         updateStatus(const std::string& id, ExportStatus status,
                              const std::string& errorMsg = "");
    
    std::vector<Export> findByRecordingId(const std::string& recordingId);
    std::vector<Export> findPending();
};
```

---

## User Profiles

### ProfileRepository

```cpp
class ProfileRepository {
public:
    std::string  create(const Profile& profile);
    Profile      findById(const std::string& id);
    bool         update(const Profile& profile);
    bool         remove(const std::string& id);
    bool         setDefault(const std::string& id);
    
    std::vector<Profile> findAll();
    Profile              getDefault();
};
```

---

## Migrations

All schema changes use a versioned migration system. Migrations run automatically on app startup if newer migrations are present.

### MigrationRunner

```cpp
class MigrationRunner {
public:
    void run(sqlite3* db);

private:
    std::vector<Migration> migrations_ = {
        {1, "Initial schema",            migration_v1},
        {2, "Add tags support",          migration_v2},
        {3, "Add plugin_data table",     migration_v3},
        {4, "Add exports.error_message", migration_v4},
    };
    
    int  getCurrentVersion(sqlite3* db);
    void applyMigration(sqlite3* db, const Migration& m);
    void recordMigration(sqlite3* db, const Migration& m);
};
```

### Migration Function Signature

```cpp
using MigrationFn = std::function<void(sqlite3*)>;

static void migration_v1(sqlite3* db) {
    sqlite3_exec(db,
        "CREATE TABLE recordings (...); "
        "CREATE TABLE settings (...); "
        "INSERT INTO settings ...",
        nullptr, nullptr, nullptr);
}
```

All migrations run inside a transaction. If a migration fails, the transaction is rolled back and the app reports a database error.

---

## Configuration Management

Application configuration (distinct from user settings) is stored as an INI file rather than SQLite for maximum portability:

```
%APPDATA%\CodeStudioRecorder\config.ini
```

```ini
[Application]
Version=1.0.0
FirstRun=false
DatabasePath=%APPDATA%\CodeStudioRecorder\data\codestudio.db
LogPath=%APPDATA%\CodeStudioRecorder\logs\

[Updates]
AutoCheckEnabled=true
LastChecked=2025-01-01T00:00:00Z

[Telemetry]
Enabled=false
```

---

*Last updated: 2025 | Module 07 of 19*
