#ifndef STORAGE_TINYDB_DB_LOG_WRITER_H_
#define STORAGE_TINYDB_DB_LOG_WRITER_H_

#include <cstdint>

#include "tinydb/slice.h"
#include "tinydb/status.h"
#include "db/log_format.h"

namespace tinydb {

class WritableFile;

namespace log {

// 用于写 预写日志（Write-Ahead Log，WAL）
class Writer {
public:
    explicit Writer(WritableFile* dest);
    Writer(WritableFile* dest, uint64_t dest_length);

    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;

    ~Writer();

    Status AddRecord(const Slice& slice);

private:
    Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);

    WritableFile* dest_;
    int block_offset_;
    uint32_t type_crc_[kMaxRecordType + 1];
};

} // namespace log
} // namespace tinydb

#endif  // STORAGE_TINYDB_DB_LOG_WRITER_H_