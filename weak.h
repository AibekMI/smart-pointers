#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : stored_ptr_(nullptr), control_block_(nullptr){};

    WeakPtr(const WeakPtr& other)
        : stored_ptr_(other.stored_ptr_), control_block_(other.control_block_) {
        if (operator bool()) {
            control_block_->WeakIncrement();
        }
    }

    WeakPtr(WeakPtr&& other)
        : stored_ptr_(other.stored_ptr_), control_block_(other.control_block_) {
        other.stored_ptr_ = nullptr;
        other.control_block_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other)
        : stored_ptr_(other.stored_ptr_), control_block_(other.control_block_) {
        if (operator bool()) {
            control_block_->WeakIncrement();
        }
    }

    template <typename Y>
    WeakPtr(const SharedPtr<Y>& other)
        : stored_ptr_(other.stored_ptr_), control_block_(other.control_block_) {
        if (operator bool()) {
            control_block_->WeakIncrement();
        }
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other)
        : stored_ptr_(other.stored_ptr_), control_block_(other.control_block_) {
        if (operator bool()) {
            control_block_->WeakIncrement();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (this == &other) {
            return *this;
        }
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (operator bool()) {
            if (UseCount() == 0 && control_block_->GetWeakCount() == 1) {
                control_block_->WeakDecrement();
                auto control_block_address = control_block_;
                control_block_ = nullptr;
                delete control_block_address;
            } else {
                control_block_->WeakDecrement();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (operator bool()) {
            if (UseCount() == 0 && control_block_->GetWeakCount() == 1) {
                control_block_->WeakDecrement();
                delete control_block_;
            } else {
                control_block_->WeakDecrement();
            }
        }
        stored_ptr_ = nullptr;
        control_block_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(stored_ptr_, other.stored_ptr_);
        std::swap(control_block_, other.control_block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!operator bool()) {
            return 0;
        }
        return control_block_->GetCount();
    }

    bool Expired() const {
        if (!operator bool()) {
            return true;
        }
        return (control_block_->GetCount() == 0);
    }

    operator bool() const {
        return (control_block_ != nullptr);
    }

    SharedPtr<T> Lock() const {
        return Expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
    }

private:
    T* stored_ptr_;
    ControlBlock* control_block_;

    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;
};
