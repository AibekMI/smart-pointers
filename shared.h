#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <type_traits>

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : stored_ptr_(nullptr), control_block_(nullptr){};
    SharedPtr(std::nullptr_t) : stored_ptr_(nullptr), control_block_(nullptr){};
    explicit SharedPtr(T* ptr) : stored_ptr_(ptr), control_block_(new PointerControlBlock<T>(ptr)) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(ptr);
        }
    };

    SharedPtr(const SharedPtr& other)
        : stored_ptr_(other.Get()), control_block_(other.control_block_) {
        if (operator bool()) {
            control_block_->operator++();
        }
    }

    SharedPtr(SharedPtr&& other) : stored_ptr_(other.Get()), control_block_(other.control_block_) {
        other.stored_ptr_ = nullptr;
        other.control_block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr)
        : stored_ptr_(ptr), control_block_(other.control_block_) {  // implicit conversion
        if (operator bool()) {
            control_block_->operator++();
        }
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other, T* ptr)
        : stored_ptr_(ptr), control_block_(other.control_block_) {
        other.stored_ptr_ = nullptr;
        other.control_block_ = nullptr;
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other)
        : stored_ptr_(other.Get()), control_block_(other.control_block_) {
        if (operator bool()) {
            control_block_->operator++();
        }
    };

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other)
        : stored_ptr_(other.Get()), control_block_(other.control_block_) {
        other.stored_ptr_ = nullptr;
        other.control_block_ = nullptr;
    };

    template <typename Y>
    SharedPtr(Y* ptr) : stored_ptr_(ptr), control_block_(new PointerControlBlock<Y>(ptr)) {
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            InitWeakThis(stored_ptr_);
        }
    };

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other)
        : stored_ptr_(other.stored_ptr_), control_block_(other.control_block_) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        if (operator bool()) {
            control_block_->operator++();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    // Can be implemented more optimally, if forwent new variable instantiation; on the other hand,
    // such code is precise and solid.

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        SharedPtr<T>(other).Swap(*this);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (this == &other) {
            return *this;
        }
        SharedPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (operator bool()) {
            if (UseCount() == 1 && WeakCount() == 0) {
                control_block_->ObjectDelete();
                delete control_block_;
                control_block_ = nullptr;
            } else {
                control_block_->operator--();
                if (UseCount() == 0) {
                    control_block_->ObjectDelete();
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        SharedPtr<T>().Swap(*this);
    }
    void Reset(T* ptr) {
        SharedPtr<T>(ptr).Swap(*this);
    }
    template <typename Y>
    void Reset(Y* ptr) {
        SharedPtr<T>(ptr).Swap(*this);
    }

    void Swap(SharedPtr& other) {
        std::swap(stored_ptr_, other.stored_ptr_);
        std::swap(control_block_, other.control_block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return stored_ptr_;
    }

    T& operator*() const {
        return *Get();
    }

    T* operator->() const {
        return Get();
    }

    size_t UseCount() const {
        if (!operator bool()) {
            return 0;
        }
        return control_block_->GetCount();
    }

    explicit operator bool() const {
        return (Get() != nullptr);
    }

private:
    size_t WeakCount() const {
        if (!operator bool()) {
            return 0;
        }
        return control_block_->GetWeakCount();
    }

    T* stored_ptr_;
    ControlBlock* control_block_;
    template <typename Z>
    void InitWeakThis(EnableSharedFromThis<Z>* e) {
        e->weak_this_ = WeakPtr<Z>(*this);
    }

    template <typename Y>
    friend class SharedPtr;

    template <typename TT, typename... Args>
    friend SharedPtr<TT> MakeShared(Args&&... args);

    template <typename Y>
    friend class WeakPtr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return (left.Get() == right.Get());
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> res;
    auto object_control_block = new ObjectControlBlock<T>(std::forward<Args>(args)...);
    res.control_block_ = object_control_block;
    res.stored_ptr_ = object_control_block->Get();
    if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
        res.InitWeakThis(res.stored_ptr_);
    }
    return res;
}

class ESFTBase {};

template <typename T>
class EnableSharedFromThis : public ESFTBase {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_this_);
    }
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<T>(weak_this_);
    }
    WeakPtr<T> WeakFromThis() noexcept {
        return WeakPtr<T>(weak_this_);
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return WeakPtr<T>(weak_this_);
    }

private:
    mutable WeakPtr<T> weak_this_;

    template <typename Y>
    friend class SharedPtr;
};
