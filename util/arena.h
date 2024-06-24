//
// Created by gtp on 2024/6/20.
//

#ifndef STORAGE_TINYDB_UTIL_ARENA_H
#define STORAGE_TINYDB_UTIL_ARENA_H

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace tinydb {

class Arena {
public:
    Arena();

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    ~Arena();

    char* Allocate(size_t bytes);
    char* AllocateAligned(size_t bytes);

    size_t MemoryUsage() const {
        return memory_usage_.load(std::memory_order_relaxed);
    }

private:
    // 在正常分配失败时调用的回退方法，通常用于分配新的内存
    char* AllocateFallback(size_t bytes);
    // 分配新的内存块，大小为 block_bytes，并将其添加到内存块列表中
    char* AllocateNewBlock(size_t block_bytes);
    // 当前内存块的分配指针，指向当前内存块的可用内存的起始位置
    char* alloc_ptr_;
    // 当前内存块中剩余的字节数
    size_t alloc_bytes_remaining_;

    std::vector<char*> blocks_;
    // 跟踪已分配的总内存量，使用 std::atomic 保证线程安全
    std::atomic<size_t> memory_usage_;
};



} // namespace tinydb


#endif //STORAGE_TINYDB_UTIL_ARENA_H
