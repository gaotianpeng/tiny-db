//
// Created by gtp on 2024/6/20.
//

#ifndef STORAGE_TINYDB__UTIL_RANDOM_H
#define STORAGE_TINYDB__UTIL_RANDOM_H

#include <cstdint>

namespace tinydb {

class Random {
private:
    uint32_t  seed_;

public:
    explicit Random(uint32_t s) : seed_(s & 0x7fffffffu) {
        if (seed_ == 0 || seed_ == 2147483647L) {
            seed_ = 1;
        }
    }
    // 生成下一个随机数
    uint32_t Next() {
        static const uint32_t M = 2147483647L;  // 常数M = 2^31-1
        static const uint64_t A = 16807;        // 常数A，生成随机数的乘数

        // 计算 seed_ = (seed_ * A) % M，其中 M = 2^31-1
        // seed_ 不能为0或M，否则后续计算的值将全部为0或M
        uint64_t product = seed_ * A;

        // 使用 ((x << 31) % M) == x 的特性计算 (product % M)
        seed_ = static_cast<uint32_t>((product >> 31) + (product & M));

        // 第一次减法可能会溢出1位，因此需要再次减去M
        // 使用 > 允许更快的基于符号位的测试
        if (seed_ > M) {
            seed_ -= M;
        }
        return seed_;
    }
    // 返回 [0..n-1] 范围内均匀分布的值
    // 要求: n > 0
    uint32_t Uniform(int n) { return Next() % n; }

    // 随机返回 true 的概率约为 "1/n"，否则返回 false
    // 要求: n > 0
    bool OneIn(int n) { return (Next() % n) == 0; }

    // Skewed: 从 [0, max_log] 范围内均匀选取 "base"，然后返回 "base" 个随机位
    // 其效果是以指数偏差选择 [0, 2^max_log-1] 范围内的一个数，偏向较小的数
    uint32_t Skewed(int max_log) { return Uniform(1 << Uniform(max_log + 1)); }
};

} // namespace tinydb

#endif //STORAGE_TINYDB__UTIL_RANDOM_H
