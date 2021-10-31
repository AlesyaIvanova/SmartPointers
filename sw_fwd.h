#pragma once

#include <exception>
#include <type_traits>
#include <iostream>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

struct EnableSharedFromThisBase;

template <typename T>
struct EnableSharedFromThis;

struct ControlBlock {
    void IncrementShared() {
        shared_count_++;
    }

    virtual void DecrementShared() {
        shared_count_--;
        if (!shared_count_) {
            OnZeroShared();
        }
        if (!weak_count_ && !shared_count_) {
            OnZeroWeak();
        }
    }

    void IncrementWeak() {
        weak_count_++;
    }

    virtual void DecrementWeak() {
        weak_count_--;
        if (!weak_count_ && !shared_count_) {
            OnZeroWeak();
        }
    }

    virtual ~ControlBlock() = default;

    virtual void OnZeroShared() = 0;
    virtual void OnZeroWeak() = 0;

    int shared_count_ = 1;
    int weak_count_ = 0;
};

template <typename T>
struct PointerControlBlock : ControlBlock {
    void DecrementShared() override {
        shared_count_--;
        if (!shared_count_) {
            OnZeroShared();
        }
        if (!weak_count_ && !shared_count_) {
            OnZeroWeak();
        }
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, T>) {
            if (weak_count_ == 1 && !shared_count_) {
                OnZeroWeak();
            }
        }
    }

    void DecrementWeak() override {
        weak_count_--;
        if (!weak_count_ && !shared_count_) {
            OnZeroWeak();
        }
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, T>) {
            if (weak_count_ == 1 && !shared_count_) {
                OnZeroWeak();
            }
        }
    }

    explicit PointerControlBlock(T* ptr) : ptr_(ptr) {
    }

    void OnZeroShared() override {
        auto obj = ptr_;
        ptr_ = nullptr;
        delete obj;
    };

    void OnZeroWeak() override {
        delete this;
    }

    ~PointerControlBlock() override {
    }

    T* ptr_ = nullptr;
};

template <typename T>
struct EmplaceControlBlock : public ControlBlock {
    void DecrementShared() override {
        shared_count_--;
        if (!shared_count_) {
            OnZeroShared();
        }
        if (!weak_count_ && !shared_count_) {
            OnZeroWeak();
        }
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, T>) {
            if (weak_count_ == 1 && !shared_count_) {
                OnZeroWeak();
            }
        }
    }

    void DecrementWeak() override {
        weak_count_--;
        if (!weak_count_ && !shared_count_) {
            OnZeroWeak();
        }
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, T>) {
            if (weak_count_ == 1 && !shared_count_) {
                OnZeroWeak();
            }
        }
    }

    template <typename... Args>
    EmplaceControlBlock(Args&&... args) {
        new (&storage) T(std::forward<Args>(args)...);
    }

    T* GetRawPtr() {
        return reinterpret_cast<T*>(&storage);
    }

    void OnZeroShared() override {
        GetRawPtr()->~T();
    };

    void OnZeroWeak() override {
        delete this;
    }

    std::aligned_storage_t<sizeof(T), alignof(T)> storage;
};