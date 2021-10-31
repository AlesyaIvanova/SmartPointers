#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), control_block_(other.control_block_) {
        if (control_block_) {
            control_block_->IncrementWeak();
        }
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& other) : ptr_(other.ptr_), control_block_(other.control_block_) {
        if (control_block_) {
            control_block_->IncrementWeak();
        }
    }

    WeakPtr(WeakPtr&& other) : ptr_(other.Get()), control_block_(other.GetControlBlock()) {
        if (control_block_) {
            control_block_->IncrementWeak();
        }
        other.Reset();
    }

    template <typename U>
    WeakPtr(WeakPtr<U>&& other) : ptr_(other.Get()), control_block_(other.GetControlBlock()) {
        if (control_block_) {
            control_block_->IncrementWeak();
        }
        other.Reset();
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr

    WeakPtr(const SharedPtr<T>& other)
        : ptr_(other.Get()), control_block_(other.GetControlBlock()) {
        if (control_block_) {
            control_block_->IncrementWeak();
        }
    }

    template <typename U>
    WeakPtr(const SharedPtr<U>& other)
        : ptr_(other.Get()), control_block_(other.GetControlBlock()) {
        if (control_block_) {
            control_block_->IncrementWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (control_block_ == other.control_block_) {
            ptr_ = other.ptr_;
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementWeak();
        }
        ptr_ = other.ptr_;
        control_block_ = other.control_block_;
        if (control_block_) {
            control_block_->IncrementWeak();
        }
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(const WeakPtr<U>& other) {
        if (control_block_ == other.control_block_) {
            ptr_ = other.ptr_;
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementWeak();
        }
        ptr_ = other.ptr_;
        control_block_ = other.control_block_;
        if (control_block_) {
            control_block_->IncrementWeak();
        }
        return *this;
    }

    WeakPtr& operator=(const SharedPtr<T>& other) {
        if (control_block_ == other.GetControlBlock()) {
            ptr_ = other.Get();
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementWeak();
        }
        ptr_ = other.Get();
        control_block_ = other.GetControlBlock();
        if (control_block_) {
            control_block_->IncrementWeak();
        }
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(const SharedPtr<U>& other) {
        if (control_block_ == other.GetControlBlock()) {
            ptr_ = other.Get();
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementWeak();
        }
        ptr_ = other.Get();
        control_block_ = other.GetControlBlock();
        if (control_block_) {
            control_block_->IncrementWeak();
        }
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (control_block_ == other.control_block_) {
            ptr_ = other.ptr_;
            if (control_block_) {
                control_block_->DecrementWeak();
            }
            other.ptr_ = nullptr;
            other.control_block_ = nullptr;
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementWeak();
        }
        ptr_ = other.ptr_;
        control_block_ = other.control_block_;
        other.ptr_ = nullptr;
        other.control_block_ = nullptr;
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(WeakPtr<U>&& other) {
        if (control_block_ == other.control_block_) {
            ptr_ = other.ptr_;
            if (control_block_) {
                control_block_->DecrementWeak();
            }
            other.ptr_ = nullptr;
            other.control_block_ = nullptr;
            return *this;
        }
        if (control_block_) {
            control_block_->DecrementWeak();
        }
        ptr_ = other.ptr_;
        control_block_ = other.control_block_;
        other.ptr_ = nullptr;
        other.control_block_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (control_block_) {
            control_block_->DecrementWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (control_block_) {
            control_block_->DecrementWeak();
        }
        ptr_ = nullptr;
        control_block_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        auto ptr = other.ptr_;
        other.ptr_ = ptr_;
        ptr_ = ptr;

        auto cb = other.control_block_;
        other.control_block_ = control_block_;
        control_block_ = cb;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!control_block_) {
            return 0;
        }
        return control_block_->shared_count_;
    }

    bool Expired() const {
        return !UseCount();
    }

    SharedPtr<T> Lock() const {
        if (Expired()) {
            if (control_block_) {
                control_block_->IncrementShared();
            }
            return SharedPtr<T>(nullptr, control_block_);
        }
        if (control_block_) {
            control_block_->IncrementShared();
        }
        return SharedPtr<T>(ptr_, control_block_);
    }

    T* Get() const {
        return ptr_;
    }

    ControlBlock* GetControlBlock() const {
        return control_block_;
    }

private:
    T* ptr_ = nullptr;
    ControlBlock* control_block_ = nullptr;

    template <typename U>
    friend class WeakPtr;

    friend struct EnableSharedFromThisBase;

    template <typename U>
    friend struct EnableSharedFromThis;
};
