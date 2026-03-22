#pragma once
#include <limits>
#include "config.hpp"

namespace gc {

template <typename T>
class Allocator {
public:
    using value_type = T;

    Allocator() = default;

    template <typename U>
    Allocator(const Allocator<U>&) {}

    T* allocate(std::size_t n) {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_alloc();

        void* ptr = MemoryManager::getInstance()->allocate(n * sizeof(T));
        if (!ptr) throw std::bad_alloc();
        
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t) noexcept {
        MemoryManager::getInstance()->deallocate(p);
    }
};

// Операторы сравнения (обязательны для STL)
template <typename T, typename U>
bool operator==(const Allocator<T>&, const Allocator<U>&) { return true; }

template <typename T, typename U>
bool operator!=(const Allocator<T>&, const Allocator<U>&) { return false; }

} // namespace gc