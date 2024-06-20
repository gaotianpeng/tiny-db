#ifndef STORAGE_TINYDB_DB_LOG_FORMAT_H_
#define STORAGE_TINYDB_DB_LOG_FORMAT_H_

namespace tinydb {
namespace log {

/*
 * WAL日志文件，以块(大小为kBlockSize)为基本单位
 * 一条日志记录可能全部写到一个块上，也可能跨几个块
 */
enum RecordType {
    kZeroType = 0,
    kFullType = 1,
    kFirstType = 2,
    kMiddleType = 3,
    kLastTyep = 4
};

static const int kMaxRecordType = kLastTyep;
static const int kBlockSize = 32768;

// Header is checksum (4 bytes), length (2 bytes), type (1 byte).
static const int kHeaderSize = 4 + 2 + 1;

} // namespace log
} // namespace tinydb


#endif  // STORAGE_TINYDB_DB_LOG_FORMAT_H_