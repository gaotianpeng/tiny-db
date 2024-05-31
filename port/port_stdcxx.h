
#ifndef STORAGE_TINYDB_PORT_PORT_STDCXX_H_
#define STORAGE_TINYDB_PORT_PORT_STDCXX_H_

#if defined(TINYDB_HAS_PORT_CONFIG_H)

#if TINYDB_HAS_PORT_CONFIG_H
#include "port/port_config.h"
#endif  // TINYDB_HAS_PORT_CONFIG_H

#elif defined(__has_include)

#if __has_include("port/port_config.h")
#include "port/port_config.h"
#endif  // __has_include("port/port_config.h")

#endif  // defined(TINYDB_HAS_PORT_CONFIG_H)

#if HAVE_CRC32C
#include <crc32c/crc32c.h>
#endif  // HAVE_CRC32C
#if HAVE_SNAPPY
#include <snappy.h>
#endif  // HAVE_SNAPPY
#if HAVE_ZSTD
#define ZSTD_STATIC_LINKING_ONLY  // For ZSTD_compressionParameters.
#include <zstd.h>
#endif  // HAVE_ZSTD

#include <cassert>
#include <condition_variable>  // NOLINT
#include <cstddef>
#include <cstdint>
#include <mutex>  // NOLINT
#include <string>

#include "port/thread_annotations.h"

namespace tinydb {
namespace port {

class CondVar;

// Thinly wraps std::mutex.
class LOCKABLE Mutex {
public:
    Mutex() = default;
    ~Mutex() = default;

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    /*
     * 获取锁。如果锁已经被其他线程持有，则调用Lock()时本线程进入等待状态，直到其他线程释放锁
     * 如果本线程已经持有锁，则再次调用Lock()会导致死锁
    */
    void Lock() EXCLUSIVE_LOCK_FUNCTION() { mu_.lock(); }
    // 释放锁。注意，需要在同一个线程内调用Lock()获得锁之后才可以调用Unlock()释放锁
    void Unlock() UNLOCK_FUNCTION() { mu_.unlock(); }
    void AssertHeld() ASSERT_EXCLUSIVE_LOCK() {}

private:
    friend class CondVar;
    std::mutex mu_;
};

// Thinly wraps std::condition_variable.
class CondVar {
public:
    explicit CondVar(Mutex* mu) : mu_(mu) { assert(mu != nullptr); }
    ~CondVar() = default;

    CondVar(const CondVar&) = delete;
    CondVar& operator=(const CondVar&) = delete;

    // 阻塞等待
    void Wait() {
        std::unique_lock<std::mutex> lock(mu_->mu_, std::adopt_lock);
        cv_.wait(lock);
        lock.release();
    }
    void Signal() { cv_.notify_one(); }
    void SignalAll() { cv_.notify_all(); }

private:
    std::condition_variable cv_;
    Mutex* const mu_;
};

/*
    将参数input指针指向的数据（参数length表明待压缩数据的长度）进行Snappy压缩并且将压//缩内容放到参数output中
    如果不支持Snappy压缩则返回false
 */
inline bool Snappy_Compress(const char* input, size_t length,
                            std::string* output) {
#if HAVE_SNAPPY
    output->resize(snappy::MaxCompressedLength(length));
    size_t outlen;
    snappy::RawCompress(input, length, &(*output)[0], &outlen);
    output->resize(outlen);
    return true;
#else
    // Silence compiler warnings about unused arguments.
    (void)input;
    (void)length;
    (void)output;
#endif  // HAVE_SNAPPY

  return false;
}

/*
    计算参数input指针指向的数据(参数length表明待解压缩数据的长度)进行Snappy解//压缩后的数据长度，
    并将其存储到参数result中。如果无法按照Snappy解压缩，则返回false
 */
inline bool Snappy_GetUncompressedLength(const char* input, size_t length,
                                         size_t* result) {
#if HAVE_SNAPPY
    return snappy::GetUncompressedLength(input, length, result);
#else
    // Silence compiler warnings about unused arguments.
    (void)input;
    (void)length;
    (void)result;
    return false;
#endif  // HAVE_SNAPPY
}

/*
    将参数input_data指针指向的数据(参数length表明待解压缩数据的长度)进行//Snappy解压缩，
    并且将解压缩内容放到参数output中。如果无法进行Snappy解压缩，则返回false
 */
inline bool Snappy_Uncompress(const char* input, size_t length, char* output) {
#if HAVE_SNAPPY
    return snappy::RawUncompress(input, length, output);
#else
    // Silence compiler warnings about unused arguments.
    (void)input;
    (void)length;
    (void)output;
    return false;
#endif  // HAVE_SNAPPY
}

inline bool Zstd_Compress(int level, const char* input, size_t length,
                          std::string* output) {
#if HAVE_ZSTD
    // Get the MaxCompressedLength.
    size_t outlen = ZSTD_compressBound(length);
    if (ZSTD_isError(outlen)) {
        return false;
    }
    output->resize(outlen);
    ZSTD_CCtx* ctx = ZSTD_createCCtx();
    ZSTD_compressionParameters parameters =
      ZSTD_getCParams(level, std::max(length, size_t{1}), /*dictSize=*/0);
    ZSTD_CCtx_setCParams(ctx, parameters);
    outlen = ZSTD_compress2(ctx, &(*output)[0], output->size(), input, length);
    ZSTD_freeCCtx(ctx);
    if (ZSTD_isError(outlen)) {
        return false;
    }
    output->resize(outlen);
    return true;
#else
    // Silence compiler warnings about unused arguments.
    (void)level;
    (void)input;
    (void)length;
    (void)output;
    return false;
#endif  // HAVE_ZSTD
}

inline bool Zstd_GetUncompressedLength(const char* input, size_t length,
                                       size_t* result) {
#if HAVE_ZSTD
    size_t size = ZSTD_getFrameContentSize(input, length);
    if (size == 0) return false;
    *result = size;
    return true;
#else
    // Silence compiler warnings about unused arguments.
    (void)input;
    (void)length;
    (void)result;
    return false;
#endif  // HAVE_ZSTD
}

inline bool Zstd_Uncompress(const char* input, size_t length, char* output) {
#if HAVE_ZSTD
    size_t outlen;
    if (!Zstd_GetUncompressedLength(input, length, &outlen)) {
    return false;
    }
    ZSTD_DCtx* ctx = ZSTD_createDCtx();
    outlen = ZSTD_decompressDCtx(ctx, output, outlen, input, length);
    ZSTD_freeDCtx(ctx);
    if (ZSTD_isError(outlen)) {
    return false;
    }
    return true;
#else
    // Silence compiler warnings about unused arguments.
    (void)input;
    (void)length;
    (void)output;
    return false;
#endif  // HAVE_ZSTD
}

// 生成当前堆内存使用情况的快照
inline bool GetHeapProfile(void (*func)(void*, const char*, int), void* arg) {
  // Silence compiler warnings about unused arguments.
    (void)func;
    (void)arg;
    return false;
}

inline uint32_t AcceleratedCRC32C(uint32_t crc, const char* buf, size_t size) {
#if HAVE_CRC32C
    return ::crc32c::Extend(crc, reinterpret_cast<const uint8_t*>(buf), size);
#else
    // Silence compiler warnings about unused arguments.
    (void)crc;
    (void)buf;
    (void)size;
    return 0;
#endif  // HAVE_CRC32C
}

}  // namespace port
}  // namespace leveldb

#endif  // STORAGE_TINYDB_PORT_PORT_STDCXX_H_
