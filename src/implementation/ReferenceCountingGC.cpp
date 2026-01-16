
#include "config.hpp"
#include <unordered_map>

namespace gc {

class ReferenceCountingGC : public MemoryManager {
private:

    template <typename T>
    class MallocAllocator {
    public:
        MallocAllocator() noexcept = default;

        template <typename U>
        MallocAllocator(const MallocAllocator<U>&) noexcept {}

        T* allocate(size_t n) {

            void* ptr = malloc(n * sizeof(T));
            if (!ptr) {
                throw std::bad_alloc();
            }
            return static_cast<T*>(ptr);
        }

        void deallocate(T* ptr, size_t) noexcept {
            free(ptr);
        }

        template <typename U>
        bool operator==(const MallocAllocator<U>&) const noexcept { return true; }

        template <typename U>
        bool operator!=(const MallocAllocator<U>&) const noexcept { return false; }
    };

    std::unordered_map<
        void*,
        size_t,
        std::hash<void*>,
        std::equal_to<void*>,
        MallocAllocator<std::pair<void* const, size_t>>
    > ref_count;

    void* allocate(std::size_t size) override {

    }

    void deallocate(void* ptr) {
        if (ptr == nullptr) return;

    }
};

}   