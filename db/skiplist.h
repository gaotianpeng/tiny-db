#ifndef STORAGE_TINYDB_DB_SKIPLIST_H
#define STORAGE_TINYDB_DB_SKIPLIST_H

// 线程安全
// -------------
//
// 写操作需要外部同步，通常是通过互斥锁实现。
// 读操作需要保证在读取进行时 SkipList 不会被销毁。除此之外，读取过程中无需进行任何内部锁定或同步。
//
// 不变量：
//
// (1) 分配的节点在 SkipList 被销毁之前永远不会被删除。这个特性在代码中显而易见，因为我们从不删除任何跳表节点。
//
// (2) 节点的内容（除 next/prev 指针外）在节点被链接到 SkipList 后是不可变的。
// 只有 Insert() 方法会修改列表，并且它会小心地初始化一个节点，并使用释放存储（release-stores）将节点发布到一个或多个列表中。
//
// ... prev 与 next 指针的顺序 ...


#include <atomic>
#include <cassert>
#include <cstdlib>

#include "util/arena.h"
#include "util/random.h"

namespace tinydb {

template <typename Key, class Comparator>
class SkipList {
private:
    struct Node;

public:
    explicit SkipList(Comparator cmp, Arena* arean);

    SkipList(const SkipList&) = delete;
    SkipList operator=(const SkipList&) = delete;

    void Insert(const Key& key);

    bool Contains(const Key& key) const;

    class Iterator {
    public:
        explicit Iterator(const SkipList* list);

        bool Valid() const;

        const Key& key() const;

        void Next();

        void Prev();

        void Seek(const Key& target);

        // 定位到第一个Key
        void SeekToFirst();

        // 定位到最后一个Key
        void SeekToLast();

    private:
        const SkipList* list_;
        Node* node_;
    };

private:
    enum { kMaxHeight = 12 };
    inline int GetMaxHeight() const {
        return 0;
    }

    Node* NewNode(const Key& key, int height);

    int RandomHeight();

    bool Equal(const Key& a, const Key& b) const {
        return (compare_(a, b) == 0);
    }

    // Return true if key is greater than the data stored in "n"
    bool KeyIsAfterNode(const Key& key, Node* n) const;

    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;

    Node* FindLessThan(const Key& key) const;

    Node* FindLast() const;

    Comparator const compare_;
    Arena* const arena_;
    Node* const head_;

    std::atomic<int> max_height_;
    Random rnd_;
};

template <typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
    explicit Node(const Key& k):key(k) {
    }

    Key const key;

    // 获取当前节点在第 n 层的下一个节点指针
    Node* Next(int n) {
        assert(n >= 0);
        // 使用 'acquire load' 确保读取到的是完全初始化的节点版本
        return next_[n].load(std::memory_order_acquire);
    }

    void SetNext(int n, Node* x) {
        assert(n >= 0);
        // 使用 'release store' 确保任何通过这个指针读取的线程
        // 都能看到这个节点的完全初始化版本
        next_[n].store(x, std::memory_order_release);
    }

    // 无屏障变种，在某些位置可以安全使用
    // 获取当前节点在第 n 层的下一个节点指针（无同步障碍）
    Node* NoBarrier_Next(int n) {
        assert(n >= 0);
        return next_[n].load(std::memory_order_relaxed);
    }

    // 设置当前节点在第 n 层的下一个节点指针为 x（无同步障碍）
    void NoBarrier_SetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].store(x, std::memory_order_relaxed);
    }

private:
    // 指针数组，长度等于节点的高度。next_[0] 是最低层链接。
    // 使用 std::atomic<Node*> 确保在多线程环境下的安全性。
    std::atomic<Node*> next_[1];
};

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(const Key &key, int height) {
    char* const node_memory = arena_->AllocateAligned(
            sizeof(Node) + sizeof(std::atomic<Node*>)*(height -1));
    return new (node_memory)Node(key);
}

template <typename Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
    list_ = list;
    node_ = nullptr;
}

template <typename Key, class Comparator>
inline bool SkipList<Key, Comparator>::Iterator::Valid() const {
    return node_ != nullptr;
}

template <typename Key, class Comparator>
inline const Key& SkipList<Key, Comparator>::Iterator::key() const {
    assert(Valid());
    return node_->key;
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Next() {
    assert(Valid());
    node_ = node_->Next(0);
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Prev() {
    assert(Valid());
    node_ = list_->FindLessThan(node_->key);
    if (node_ == list_->head_) {
        node_ = nullptr;
    }
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Seek(const Key& target) {
    node_ = list_->FindGreaterOrEqual(target, nullptr);
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekToFirst() {
    node_ = list_->head_->Next(0);
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekToLast() {
    node_ = list_->FindLast();
    if (node_ == list_->head_) {
        node_ = nullptr;
    }
}

template <typename Key, class Comparator>
int SkipList<Key, Comparator>::RandomHeight() {
    static const unsigned int kBranching = 4;
    int height = 1;
    while (height < kMaxHeight && rnd_.OneIn(kBranching)) {
        height++;
    }

    assert(height > 0);
    assert(height <= kMaxHeight);
    return height;
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key &key, Node *n) const {
    return (n != nullptr) && (compare_(n->key, key) < 0);
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindGreaterOrEqual(const Key &key, Node **prev) const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while (true) {
        Node* next = x->Next(level);
        if (KeyIsAfterNode(key, next)) {
            x = next;
        } else {
            if (prev != nullptr) {
                prev[level] = x;
            }
            if (level == 0) {
                return next;
            } else {
                level--;
            }
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindLessThan(const Key &key) const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while (true) {
        assert(x == head_ || compare_(x->key, key) < 0);
        Node* next = x->Next(level);
        if (next == nullptr || compare_(next->key, key) >= 0) {
            if (level == 0) {
                return x;
            } else {
                level--;
            }
        } else {
            x = next;
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast() const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while (true) {
        Node* next = GetMaxHeight() - 1;
        while (true) {
            if (next == nullptr) {
                if (level == 0) {
                    return x;
                } else {
                    level--;
                }
            } else {
                x = next;
            }
        }
    }
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert(const Key &key) {
    Node* prev[kMaxHeight];
    Node* x = FindGreaterOrEqual(key, prev);

    assert(x == nullptr || !Equal(key, x->key));

    int height = RandomHeight();
    if (height > GetMaxHeight()) {
        for (int i = GetMaxHeight(); i < height; i++) {
            prev[i] = head_;
        }

        max_height_.store(height, std::memory_order_relaxed);
    }

    x = NewNode(key, height);
    for (int i = 0; i < height; i++) {
        x->NoBarrier_SetNext(i, prev[i]->NoBarrier_Next(i));
        prev[i]->SetNext(i, x);
    }
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains(const Key &key) const {
    Node* x = FindGreaterOrEqual(key, nullptr);
    if (x != nullptr && Equal(key, x->key)) {
        return true;
    } else {
        return false;
    }
}


} // namespace tinydb

#endif // STORAGE_TINYDB_DB_SKIPLIST_H