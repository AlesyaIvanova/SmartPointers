#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

#include <iostream>

struct EnableSharedFromThisBase {
    virtual ~EnableSharedFromThisBase() {
    }
};

template <typename T>
struct EnableSharedFromThis : EnableSharedFromThisBase {
    SharedPtr<T> SharedFromThis() {
        return self;
    }

    SharedPtr<const T> SharedFromThis() const {
        return self;
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return self;
    }

    WeakPtr<const T> WeakFromThis() const noexcept {
        return self;
    }

    ~EnableSharedFromThis() override {
        self.control_block_ = nullptr;
    }

    WeakPtr<T> self;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }

    SharedPtr(std::nullptr_t) {
    }

    explicit SharedPtr(T* ptr) : ptr_(ptr), control_block_(new PointerControlBlock<T>(ptr)) {
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, T>) {
            EnableSharedFromThisHelper(ptr);
        }
    }

    template <typename U>
    explicit SharedPtr(U* ptr) : ptr_(ptr), control_block_(new PointerControlBlock<U>(ptr)) {
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, U>) {
            EnableSharedFromThisHelper(ptr);
        }
    }

    SharedPtr(T* ptr, ControlBlock* control_block) : ptr_(ptr), control_block_(control_block) {
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, T>) {
            EnableSharedFromThisHelper(ptr);
        }
    }

    template <typename U>
    SharedPtr(U* ptr, ControlBlock* control_block) : ptr_(ptr), control_block_(control_block) {
        if constexpr (std::is_base_of_v<EnableSharedFromThisBase, U>) {
            EnableSharedFromThisHelper(ptr);
        }
    }

    template <typename Y>
    void EnableSharedFromThisHelper(EnableSharedFromThis<Y>* base) {
        base->self = *this;
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), control_block_(other.control_block_) {
        if (control_block_) {
            control_block_->IncrementShared();
        }
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other)
        : ptr_(other.Get()), control_block_(other.GetControlBlock()) {
        if (control_block_) {
            control_block_->IncrementShared();
        }
    }

    SharedPtr(SharedPtr&& other) : ptr_(other.ptr_), control_block_(other.control_block_) {
        if (control_block_) {
            control_block_->IncrementShared();
        }
        other.Reset();
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) : ptr_(other.Get()), control_block_(other.GetControlBlock()) {
        if (control_block_) {
            control_block_->IncrementShared();
        }
        other.Reset();
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr)
        : ptr_(ptr), control_block_(other.GetControlBlock()) {
        if (control_block_) {
            control_block_->IncrementShared();
        }
    }

    template <typename Y, typename U>
    SharedPtr(const SharedPtr<Y>& other, U* ptr)
        : ptr_(ptr), control_block_(other.GetControlBlock()) {
        if (control_block_) {
            control_block_->IncrementShared();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    SharedPtr(const WeakPtr<T>& other)
        : ptr_(other.Get()), control_block_(other.GetControlBlock()) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        if (control_block_) {
            control_block_->IncrementShared();
        }
    }

    template <typename U>
    SharedPtr(const WeakPtr<U>& other)
        : ptr_(other.Get()), control_block_(other.GetControlBlock()) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        if (control_block_) {
            control_block_->IncrementShared();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (control_block_ == other.control_block_) {
            ptr_ = other.ptr_;
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementShared();
        }
        ptr_ = other.ptr_;
        control_block_ = other.control_block_;
        if (control_block_) {
            control_block_->IncrementShared();
        }
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        if (control_block_ == other.GetControlBlock()) {
            ptr_ = other.Get();
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementShared();
        }
        ptr_ = other.Get();
        control_block_ = other.GetControlBlock();
        if (control_block_) {
            control_block_->IncrementShared();
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (control_block_ == other.control_block_) {
            ptr_ = other.ptr_;
            if (control_block_) {
                control_block_->DecrementShared();
            }
            other.ptr_ = nullptr;
            other.control_block_ = nullptr;
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementShared();
        }
        ptr_ = other.ptr_;
        control_block_ = other.control_block_;
        other.ptr_ = nullptr;
        other.control_block_ = nullptr;
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        if (control_block_ == other.GetControlBlock()) {
            ptr_ = other.Get();
            other.Reset();
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementShared();
        }
        ptr_ = other.Get();
        control_block_ = other.GetControlBlock();
        control_block_->IncrementShared();
        other.Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (control_block_) {
            control_block_->DecrementShared();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (control_block_) {
            control_block_->DecrementShared();
        }
        ptr_ = nullptr;
        control_block_ = nullptr;
    }

    void Reset(T* ptr) {
        if (ptr_ == ptr) {
            return;
        }
        if (control_block_) {
            control_block_->DecrementShared();
        }
        ptr_ = ptr;
        control_block_ = new PointerControlBlock<T>(ptr);
    }

    template <typename U>
    void Reset(U* ptr) {
        if (ptr_ == ptr) {
            return;
        }
        if (control_block_) {
            control_block_->DecrementShared();
        }
        ptr_ = ptr;
        control_block_ = new PointerControlBlock<U>(ptr);
    }

    void Swap(SharedPtr& other) {
        auto ptr = other.ptr_;
        other.ptr_ = ptr_;
        ptr_ = ptr;

        auto cb = other.control_block_;
        other.control_block_ = control_block_;
        control_block_ = cb;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    size_t UseCount() const {
        if (!control_block_) {
            return 0;
        }
        return control_block_->shared_count_;
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

    ControlBlock* GetControlBlock() const {
        return control_block_;
    }

private:
    T* ptr_ = nullptr;
    ControlBlock* control_block_ = nullptr;

    friend struct EnableSharedFromThisBase;

    template <typename U>
    friend struct EnableSharedFromThis;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new EmplaceControlBlock<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block->GetRawPtr(), block);
}
