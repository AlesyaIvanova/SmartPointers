#pragma once

#include "compressed_pair.h"
#include <memory>
#include <type_traits>
#include <iostream>
#include <utility>

#include <cstddef>  // std::nullptr_t

template <typename T>
struct DefaultDelete {
    void operator()(T* ptr) {
        delete ptr;
    }
};

template <typename T>
struct DefaultDelete<T[]> {
    void operator()(T* ptr) const noexcept {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : ptr_(ptr) {
    }

    template <typename D>
    UniquePtr(T* ptr, D&& deleter) noexcept : ptr_(ptr), deleter_(std::forward<D>(deleter)) {
    }

    template <typename U>
    UniquePtr(UniquePtr<U, DefaultDelete<U>>&& other) noexcept : ptr_(other.Release()) {
    }

    template <typename U, typename D>
    UniquePtr(UniquePtr<U, D>&& other) noexcept
        : ptr_(other.Release()), deleter_(std::forward<D>(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Reset(other.Release());
        deleter_ = std::forward<Deleter>(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        deleter_(ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* old_ptr = ptr_;
        ptr_ = ptr;
        deleter_(old_ptr);
    }

    void Swap(UniquePtr& other) {
        T* ptr = ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = ptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    Deleter& GetDeleter() {
        return deleter_;
    }

    const Deleter& GetDeleter() const {
        return deleter_;
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

private:
    T* ptr_;
    [[no_unique_address]] Deleter deleter_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : ptr_(ptr) {
    }

    template <typename D>
    UniquePtr(T* ptr, D&& deleter) noexcept : ptr_(ptr), deleter_(std::forward<D>(deleter)) {
    }

    template <typename U>
    UniquePtr(UniquePtr<U, DefaultDelete<U>>&& other) noexcept : ptr_(other.Release()) {
    }

    template <typename U, typename D>
    UniquePtr(UniquePtr<U, D>&& other) noexcept
        : ptr_(other.Release()), deleter_(std::forward<D>(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Reset(other.Release());
        deleter_ = std::forward<Deleter>(other.GetDeleter());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        deleter_(ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* old_ptr = ptr_;
        ptr_ = ptr;
        deleter_(old_ptr);
    }

    void Swap(UniquePtr& other) {
        T* ptr = ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = ptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    Deleter& GetDeleter() {
        return deleter_;
    }

    const Deleter& GetDeleter() const {
        return deleter_;
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    T& operator[](std::size_t i) const {
        return ptr_[i];
    }

private:
    T* ptr_;
    [[no_unique_address]] Deleter deleter_;
};