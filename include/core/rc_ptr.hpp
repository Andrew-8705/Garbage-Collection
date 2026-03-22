#pragma once
#include "config.hpp"

namespace gc {

template <typename T>
class RcPtr {
public:
    RcPtr() : ptr(nullptr) {}
    
    explicit RcPtr(T* p) : ptr(p) {
        if (ptr) MemoryManager::getInstance()->addRef(ptr);
    }

    ~RcPtr() {
        if (ptr) MemoryManager::getInstance()->release(ptr);
    }

    RcPtr(const RcPtr& other) : ptr(other.ptr) {
        if (ptr) MemoryManager::getInstance()->addRef(ptr);
    }

    RcPtr& operator=(const RcPtr& other) {
        if (this != &other) {
            if (ptr) MemoryManager::getInstance()->release(ptr);
            
            ptr = other.ptr;
            if (ptr) MemoryManager::getInstance()->addRef(ptr);
        }
        return *this;
    }

    RcPtr(RcPtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    RcPtr& operator=(RcPtr&& other) noexcept {
        if (this != &other) {
            if (ptr) MemoryManager::getInstance()->release(ptr);
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    T* get() const { return ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    explicit operator bool() const { return ptr != nullptr; }

    bool operator==(const RcPtr& other) const { return ptr == other.ptr; }
    bool operator!=(const RcPtr& other) const { return ptr != other.ptr; }

private:
    T* ptr;
};

template <typename T, typename... Args>
RcPtr<T> make_ptr(Args&&... args) {
    T* raw = gc::make<T>(std::forward<Args>(args)...);
    return RcPtr<T>(raw);
}

} // namespace gc