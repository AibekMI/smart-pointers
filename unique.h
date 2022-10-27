#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <utility>
#include <type_traits>
#include <stdlib.h>

template <typename T>
struct DefaultDeleter {
    DefaultDeleter() = default;

    template <typename U>
    constexpr DefaultDeleter(DefaultDeleter<U>&&) {
        static_assert(std::is_base_of_v<T, U>);
    };

    template <typename U>
    constexpr DefaultDeleter& operator=(DefaultDeleter<U>&&) {
        static_assert(std::is_base_of_v<T, U>);
        return *this;
    };

    void operator()(T* ptr) const {
        delete ptr;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    DefaultDeleter() = default;

    template <typename U>
    constexpr DefaultDeleter(DefaultDeleter<U[]>&&) {
        static_assert(std::is_base_of_v<T, U>);
    };

    template <typename U>
    constexpr DefaultDeleter& operator=(DefaultDeleter<U[]>&&) {
        static_assert(std::is_base_of_v<T, U>);
        return *this;
    };

    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

template <>
struct DefaultDeleter<void> {
    void operator()(void* ptr) const {
        free(ptr);
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()){};
    UniquePtr(T* ptr, Deleter deleter) : data_(ptr, std::move(deleter)){};

    UniquePtr(UniquePtr&& other) noexcept : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    template <typename TT, typename DD>
    UniquePtr(UniquePtr<TT, DD>&& other) noexcept : data_(other.Release(), Deleter()) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Reset(other.Release());

        GetDeleter() = std::move(other.GetDeleter());
        other.GetDeleter() = std::remove_reference_t<decltype(other.GetDeleter())>();
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        if (Get()) {
            GetDeleter()(Release());
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (Get()) {
            GetDeleter()(Release());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto res = Get();
        data_.GetFirst() = nullptr;
        return res;
    }

    void Reset(T* ptr = nullptr) {
        auto old_ptr = Get();
        data_.GetFirst() = ptr;

        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }

    Deleter& GetDeleter() {
        return data_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }

    explicit operator bool() const {
        return (Get() != nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const {
        return *Get();
    }

    T* operator->() const {
        return Get();
    }

private:
    CompressedPair<T*, Deleter> data_;
};

//// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()){};
    UniquePtr(T* ptr, Deleter deleter) : data_(ptr, std::move(deleter)){};

    UniquePtr(UniquePtr&& other) noexcept : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    template <typename TT, typename DD>
    UniquePtr(UniquePtr<TT, DD>&& other) noexcept : data_(other.Release(), Deleter()) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Reset(other.Release());

        GetDeleter() = std::move(other.GetDeleter());
        other.GetDeleter() = std::remove_reference_t<decltype(other.GetDeleter())>();
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        if (Get()) {
            GetDeleter()(Release());
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (Get()) {
            GetDeleter()(Release());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto res = Get();
        data_.GetFirst() = nullptr;
        return res;
    }

    void Reset(T* ptr = nullptr) {
        auto old_ptr = Get();
        data_.GetFirst() = ptr;

        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }

    Deleter& GetDeleter() {
        return data_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }

    explicit operator bool() const {
        return (Get() != nullptr);
    }

    T& operator[](size_t index) {
        return *(Get() + index);
    }

private:
    CompressedPair<T*, Deleter> data_;
};

//// Specialization for void type
template <typename Deleter>
class UniquePtr<void, Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(void* ptr = nullptr) : data_(ptr, Deleter()){};
    UniquePtr(void* ptr, Deleter deleter) : data_(ptr, std::move(deleter)){};

    UniquePtr(UniquePtr&& other) noexcept : data_(other.Release(), std::move(other.GetDeleter())) {
    }

    template <typename TT, typename DD>
    UniquePtr(UniquePtr<TT, DD>&& other) noexcept : data_(other.Release(), Deleter()) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Reset(other.Release());

        GetDeleter() = std::move(other.GetDeleter());
        other.GetDeleter() = std::remove_reference_t<decltype(other.GetDeleter())>();
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        if (Get()) {
            GetDeleter()(Release());
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (Get()) {
            GetDeleter()(Release());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void* Release() {
        auto res = Get();
        data_.GetFirst() = nullptr;
        return res;
    }

    void Reset(void* ptr = nullptr) {
        auto old_ptr = Get();
        data_.GetFirst() = ptr;

        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(data_, other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    void* Get() const {
        return data_.GetFirst();
    }

    Deleter& GetDeleter() {
        return data_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }

    explicit operator bool() const {
        return (Get() != nullptr);
    }

    void* operator->() const {
        return Get();
    }

private:
    CompressedPair<void*, Deleter> data_;
};
