#include <iostream>
#include <cstdlib>
#include <new>
#include <string>

#include "config.hpp" 
#include "logger.hpp"

#ifdef GC_DEBUG_ALLOC

void* operator new(std::size_t size) noexcept(false) {
    //std::cerr << "[Global New] Allocated: " << size << " bytes.\n";

    if (void* p = std::malloc(size)) {
        Logger::getInstance().log((long long)size);
        return p;
    }
    throw std::bad_alloc{};
}

void operator delete(void* p) noexcept {
    if (p == nullptr) return;
    //std::cerr << "[Global Delete] Deallocating memory.\n";
    std::free(p);
}

void* operator new[](std::size_t size) noexcept(false) {
    return operator new(size);
}

void operator delete[](void* p) noexcept {
    operator delete(p);
}

#endif // GC_DEBUG_ALLOC