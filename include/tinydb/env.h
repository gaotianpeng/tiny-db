#ifndef STORAGE_TINYDB_INCLUDE_ENV_H_
#define STORAGE_TINYDB_INCLUDE_ENV_H_

#include <cstdarg>
#include <cstdint>

#include "tinydb/export.h"
#include "tinydb/status.h"

namespace tinydb {

class SequentialFile;

class TINYDB_EXPORT Env {
public:
    Env();

    Env(const Env&) = delete;
    Env& operator=(const Env&) = delete;

    virtual ~Env();

    static Env* Default();


    virtual Status NewSequentialFile(const std::string& fname,
                                     SequentialFile** result) = 0;
};

/*
    顺序读，如日志文件的读取、Manifest文件的读取
*/
class TINYDB_EXPORT SequentialFile {
public:
    SequentialFile() = default;

    SequentialFile(const SequentialFile&) = delete;
    SequentialFile& operator=(const SequentialFile&) = delete;

    virtual ~SequentialFile();

    virtual Status Read(size_t n, Slice* result, char* scratch) = 0;

    virtual Status Skip(uint64_t n) = 0;
};

class TINYDB_EXPORT RandomAccessFile {
public:
    RandomAccessFile() = default;

    RandomAccessFile(const RandomAccessFile&) = delete;
    RandomAccessFile& operator=(const RandomAccessFile&) = delete;

    virtual ~RandomAccessFile();

    /*
        用于从文件的指定偏移位置读取最多 n 字节的数据
            offset：从文件的哪个偏移位置开始读取
            n：要读取的最大字节数
            result：读取的数据结果，会被设置为指向实际读取的数据
            scratch：用于存储读取数据的缓冲区，可能被写入 scratch[0..n-1]
        该方法是线程安全的，多个线程可以安全地并发使用它
     */
    virtual Status Read(uint64_t offset, size_t, Slice* result, char* scratch) const = 0;
};

/*
    顺序写，用于日志文件、SSTable文件、Manifest文件的写入
    实现这个类的具体子类需要提供数据缓冲区。调用者可能会一次写入小片段数据。
    直接将这些小片段数据写入磁盘效率很低，因为每次写入都会触发 I/O 操作。
    使用缓冲区可以将多个小片段数据合并成一个较大的块，然后一次性写入磁盘，从而提高写入效率
*/
class TINYDB_EXPORT WritableFile {
public:
    WritableFile() = default;

    WritableFile(const WritableFile&) = delete;
    WritableFile& operator=(const WritableFile&) = delete;

    virtual ~WritableFile();

    virtual Status Append(const Slice& data) = 0;
    virtual Status Close() = 0;
    virtual Status Flush() = 0;
    virtual Status Sync() = 0;
};

class TINYDB_EXPORT Logger {
public:
    Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    virtual ~Logger();

    // 按指定格式向日志文件写入一个条目。
    virtual void Logv(const char* format, std::va_list ap) = 0;
};

// Log the specified data to *info_log if info_log is non-null.
    void Log(Logger* info_log, const char* format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((__format__(__printf__, 2, 3)))
#endif
    ;

// A utility routine: write "data" to the named file.
TINYDB_EXPORT Status WriteStringToFile(Env* env, const Slice& data,
const std::string& fname);

// A utility routine: read contents of named file into *data
TINYDB_EXPORT Status ReadFileToString(Env* env, const std::string& fname,
        std::string* data);


class TINYDB_EXPORT FileLock {
public:
    FileLock() = default;

    FileLock(const FileLock&) = delete;
    FileLock operator=(const FileLock&) = delete;

    virtual ~FileLock();
};

/*
    EnvWrapper是一个Env的实现
    转发调用的 Env 实现：这段注释说明 EnvWrapper 类是一个 Env 的实现，它将所有调用转发给另一个 Env 对象
    部分功能覆盖：这个类对于那些只想覆盖部分 Env 功能的客户端来说非常有用
    例如，用户可能只想改变日志记录或文件操作的某些行为，而不必重新实现所有的 Env 方法
 */
class TINYDB_EXPORT EnvWrapper: public Env {
public:

private:
    Env* target_;
};

} // namespace tinydb

#endif // STORAGE_TINYDB_INCLUDE_ENV_H_