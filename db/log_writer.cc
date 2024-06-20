#include "db/log_writer.h"

#include <cstdint>
#include "tinydb/env.h"

namespace tinydb {
namespace log {

Writer::Writer(WritableFile *dest) {
}

Writer::Writer(WritableFile *dest, uint64_t dest_length) {
}

Writer::~Writer() = default;

Status Writer::AddRecord(const tinydb::Slice &slice) {
    const char* ptr = slice.data();
    size_t left = slice.size();

    /*
     *  如果有必要，将记录进行分片并发出。注意，如果slice是空的，仍会迭代一次以发出单个零长度记录。
     */
    Status s;
    bool begin = true;
    do {
        // leftover当前block还可以写数据的字节数
        const int leftover = kBlockSize - block_offset_;
        assert(leftover >= 0);
        if (leftover < kHeaderSize) {
            if (leftover > 0) {
                static_assert(kHeaderSize == 7, "");
                dest_->Append(Slice("\x00\x00\x00\x00\x00\x00", leftover));
            }
            block_offset_ = 0;
        }

        assert(kBlockSize - block_offset_ - kHeaderSize >= 0);
        // 当前block剩余可写日志数据空间
        const size_t avail = kBlockSize - block_offset_ - kHeaderSize;

        const size_t fragment_length = (left < avail) ? left: avail;
        RecordType type;
        const bool end = (left == fragment_length);
        if (begin && end) {
            type = kFullType;
        } else if (begin) {
            type = kFirstType;
        } else if (end) {
            type = kLastTyep;
        } else {
            type = kMiddleType;
        }

        s = EmitPhysicalRecord(type, ptr, fragment_length);
        ptr += fragment_length;
        left -= fragment_length;
        begin = false;
    } while (s.ok() && left > 0);

    return s;
}

Status Writer::EmitPhysicalRecord(RecordType type, const char* ptr, size_t length) {
    return Status::OK();
}



} // namespace log
} // namespace tinydb