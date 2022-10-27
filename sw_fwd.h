#pragma once

#include <exception>

// Forward declarations and control blocks

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class ESFTBase;

template <typename T>
class EnableSharedFromThis;

class ControlBlock {
public:
    virtual int GetCount() const = 0;
    virtual int GetWeakCount() const = 0;
    virtual void operator++() = 0;
    virtual void operator--() = 0;
    virtual void WeakDecrement() = 0;
    virtual void WeakIncrement() = 0;
    virtual ~ControlBlock() = default;
    virtual void ObjectDelete() = 0;
};

template <typename T>
class PointerControlBlock : public ControlBlock {
public:
    PointerControlBlock() : counter_(0), weak_counter_(0), ptr_(nullptr){};
    explicit PointerControlBlock(T* ptr) : counter_(1), weak_counter_(0), ptr_(ptr){};

    void operator++() override {
        ++counter_;
    }

    void operator--() override {
        --counter_;
    }

    void WeakDecrement() override {
        --weak_counter_;
    }

    void WeakIncrement() override {
        ++weak_counter_;
    }

    int GetCount() const override {
        return counter_;
    }

    int GetWeakCount() const override {
        return weak_counter_;
    }

    ~PointerControlBlock() override = default;

    void ObjectDelete() override {
        delete ptr_;
    }

private:
    int counter_;
    int weak_counter_;
    T* ptr_;
};

template <typename T>
class ObjectControlBlock : public ControlBlock {
public:
    template <typename... Args>
    ObjectControlBlock(Args&&... args) : counter_(1), weak_counter_(0) {
        new (&buf_) T(std::forward<Args>(args)...);
    }

    void operator++() override {
        ++counter_;
    }
    void operator--() override {
        --counter_;
    }

    void WeakDecrement() override {
        --weak_counter_;
    }

    void WeakIncrement() override {
        ++weak_counter_;
    }

    T* Get() {
        return reinterpret_cast<T*>(&buf_);
    }

    int GetCount() const override {
        return counter_;
    }
    int GetWeakCount() const override {
        return weak_counter_;
    }

    ~ObjectControlBlock() override = default;

    void ObjectDelete() override {
        Get()->~T();
    }

private:
    int counter_;
    int weak_counter_;
    alignas(T) char buf_[sizeof(T)];
};
