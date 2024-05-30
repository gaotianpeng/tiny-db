#ifndef STORAGE_TINYDB_INCLUDE_OPTIONS_H
#define STORAGE_TINYDB_INCLUDE_OPTIONS_H


#include <cstddef>
#include "tinydb/export.h"


namespace tinydb {

enum CompressionType {
    kNoCompression = 0x0,
    kSnappyCompression = 0x1,
    kZstdCompression = 0x2,
};

struct TINYDB_EXPORT Options {
    Options();

    bool create_if_missing = false;
    bool error_if_exists = false;
    int max_open_files = 1000;
    size_t block_size = 4 * 1024;
    int block_restart_interval = 16;
    size_t max_file_size = 2 * 1024 * 1024;
    CompressionType compression = kSnappyCompression;
};

struct TINYDB_EXPORT ReadOption {
    bool verify_checksums = false;
    bool fill_cache = true;

};

struct TINYDB_EXPORT WriteOptions {
    WriteOptions() = default;
    bool sync = false;
};


} // namespace tinydb

#endif // STORAGE_TINYDB_INCLUDE_OPTIONS_H