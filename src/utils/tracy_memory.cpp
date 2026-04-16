#include <cstdlib>
#include <new>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>

void* operator new(std::size_t count) {
    auto ptr = std::malloc(count);
    if (!ptr) throw std::bad_alloc();
    
    TracyAlloc(ptr, count); 
    return ptr;
}

void operator delete(void* ptr) noexcept {
    if (!ptr) return;
    TracyFree(ptr);
    std::free(ptr);
}

void* operator new[](std::size_t count) {
    auto ptr = std::malloc(count);
    if (!ptr) throw std::bad_alloc();
    TracyAlloc(ptr, count);
    return ptr;
}

void operator delete[](void* ptr) noexcept {
    if (!ptr) return;
    TracyFree(ptr);
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept {
    if (!ptr) return;
    TracyFree(ptr);
    std::free(ptr);
}
#endif